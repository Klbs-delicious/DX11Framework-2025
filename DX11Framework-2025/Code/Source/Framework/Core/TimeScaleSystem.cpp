/** @file   TimeScaleSystem.cpp
 *  @brief  TimeScaleSystem の実装
 *  @date   2026/02/15
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/TimeScaleSystem.h"

#include <algorithm>

//-----------------------------------------------------------------------------
// TimeScaleSystem class
//-----------------------------------------------------------------------------

/// @brief コンストラクタ
TimeScaleSystem::TimeScaleSystem() :
	globalScale(1.0f),
	layerScales{},
	groupBaseScales{},
	groupAppliedScales{},
	eventDefs{},
	activeEvents{}
{
	for (auto& s : this->layerScales)
	{
		s = 1.0f;
	}

	//------------------------------------------------------------------------------
	// イベント定義
	//------------------------------------------------------------------------------
	{
		TimeScaleEventDef def{};
		def.id = TimeScaleEventId::JustDodge;
		def.targetGroupName = "World";
		def.scale = 0.25f;
		def.durationRawSec = 0.50f;
		def.priority = 10;
		def.stackPolicy = TimeScaleStackPolicy::Extend;
		this->eventDefs[static_cast<size_t>(TimeScaleEventId::JustDodge)] = def;
	}
	{
		TimeScaleEventDef def{};
		def.id = TimeScaleEventId::HitStop;
		def.targetGroupName = "World";
		def.scale = 0.0f;
		def.durationRawSec = 0.06f;
		def.priority = 100;
		def.stackPolicy = TimeScaleStackPolicy::Extend;
		this->eventDefs[static_cast<size_t>(TimeScaleEventId::HitStop)] = def;
	}
	{
		TimeScaleEventDef def{};
		def.id = TimeScaleEventId::TestDodge;
		def.targetGroupName = "TestGroup";
		def.scale = 0.3f;
		def.durationRawSec = 0.50f;
		def.priority = 100;
		def.stackPolicy = TimeScaleStackPolicy::Extend;
		this->eventDefs[static_cast<size_t>(TimeScaleEventId::TestDodge)] = def;
	}

	// 初期の適用値は基準値に一致させる（空なら何もしない）
	this->RebuildAppliedGroupScales();
}

/** @brief 毎フレーム更新（イベント残り時間を rawDelta で減算）
 *  @param _rawDeltaSec rawDelta（秒）
 */
void TimeScaleSystem::Update(float _rawDeltaSec)
{
	if (_rawDeltaSec <= 0.0f)
	{
		return;
	}

	bool anyChanged = false;

	for (auto it = this->activeEvents.begin(); it != this->activeEvents.end();)
	{
		ActiveEvent& e = it->second;
		e.remainingRawSec -= _rawDeltaSec;

		if (e.remainingRawSec <= 0.0f)
		{
			it = this->activeEvents.erase(it);
			anyChanged = true;
			continue;
		}

		++it;
	}

	if (anyChanged)
	{
		this->RebuildAppliedGroupScales();
	}
}

/** @brief グローバル TimeScale を設定する
 *  @param _scale 設定するスケール値
 */
void TimeScaleSystem::SetGlobalScale(float _scale)
{
	this->globalScale = _scale;
}

/// @brief グローバル TimeScale を取得する
float TimeScaleSystem::GlobalScale() const
{
	return this->globalScale;
}

/** @brief レイヤーの TimeScale を設定する
 *  @param _layer 対象レイヤー
 *  @param _scale 設定するスケール値
 */
void TimeScaleSystem::SetLayerScale(TimeScaleLayer _layer, float _scale)
{
	const size_t index = static_cast<size_t>(_layer);
	this->layerScales[index] = _scale;
}

/** @brief レイヤーの TimeScale を取得する
 *  @param _layer 対象レイヤー
 *  @return 指定レイヤーのスケール値
 */
float TimeScaleSystem::LayerScale(TimeScaleLayer _layer) const
{
	const size_t index = static_cast<size_t>(_layer);
	return this->layerScales[index];
}

/** @brief 合成済み TimeScale（Global × Layer）を取得する
 *  @param _layer 対象レイヤー
 *  @return 合成されたスケール値
 */
float TimeScaleSystem::CombinedScale(TimeScaleLayer _layer) const
{
	const size_t index = static_cast<size_t>(_layer);
	return this->globalScale * this->layerScales[index];
}

/** @brief グループ倍率（基準値）を設定する
 *  @param _groupName グループ名
 *  @param _scale 基準倍率
 */
void TimeScaleSystem::SetGroupBaseScale(const std::string& _groupName, float _scale)
{
	this->groupBaseScales[_groupName] = _scale;
	this->RebuildAppliedGroupScales();
}

/** @brief グループ倍率（最終適用値）を取得する
 *  @param _groupName グループ名
 *  @return 最終適用倍率（未登録は 1.0f）
 */
float TimeScaleSystem::GetGroupScale(const std::string& _groupName) const
{
	auto it = this->groupAppliedScales.find(_groupName);
	if (it != this->groupAppliedScales.end())
	{
		return it->second;
	}

	auto itBase = this->groupBaseScales.find(_groupName);
	if (itBase != this->groupBaseScales.end())
	{
		return itBase->second;
	}

	return 1.0f;
}

/** @brief TimeScale イベントをリクエストする（IDのみ）
 *  @param _eventId イベントID
 */
void TimeScaleSystem::RequestEvent(TimeScaleEventId _eventId)
{
	const TimeScaleEventDef* def = this->FindEventDef(_eventId);
	if (def == nullptr)
	{
		return;
	}

	EventKey key{};
	key.id = def->id;
	key.groupName = def->targetGroupName;

	auto it = this->activeEvents.find(key);
	if (it != this->activeEvents.end())
	{
		ActiveEvent& active = it->second;

		if (def->stackPolicy == TimeScaleStackPolicy::Extend)
		{
			active.remainingRawSec = std::max(active.remainingRawSec, 0.0f) + def->durationRawSec;
		}
		else
		{
			active.remainingRawSec = def->durationRawSec;
		}

		active.def = *def;
		this->RebuildAppliedGroupScales();
		return;
	}

	ActiveEvent active{};
	active.def = *def;
	active.remainingRawSec = def->durationRawSec;
	this->activeEvents.emplace(key, active);

	this->RebuildAppliedGroupScales();
}

/** @brief イベント定義を差し替える（必要な場合のみ使用）
 *  @param _def 定義
 */
void TimeScaleSystem::SetEventDef(const TimeScaleEventDef& _def)
{
	const size_t index = static_cast<size_t>(_def.id);
	if (index >= this->eventDefs.size())
	{
		return;
	}

	this->eventDefs[index] = _def;
}

/** @brief イベント定義を取得する
 *  @param _eventId イベントID
 *  @return 定義（見つからなければ nullptr）
 */
const TimeScaleEventDef* TimeScaleSystem::FindEventDef(TimeScaleEventId _eventId) const
{
	const size_t index = static_cast<size_t>(_eventId);
	if (index >= this->eventDefs.size())
	{
		return nullptr;
	}

	const TimeScaleEventDef& def = this->eventDefs[index];

	// target が空のものは無効扱いにする
	if (def.targetGroupName.empty())
	{
		return nullptr;
	}

	return &def;
}

void TimeScaleSystem::ApplyEventToGroup(const TimeScaleEventDef& _def)
{
	// RebuildAppliedGroupScales() で groupAppliedScales は基準値に戻してある前提
	// ここではイベント値をそのまま上書きするだけ
	this->groupAppliedScales[_def.targetGroupName] = _def.scale;
}

void TimeScaleSystem::RebuildAppliedGroupScales()
{
	// まず基準値で埋める（登録済みグループのみ）
	this->groupAppliedScales = this->groupBaseScales;

	// activeEvents を group ごとに「優先度最大」を採用する
	// 同一 group で priority が同じ場合は現状「後勝ち」
	std::unordered_map<std::string, const TimeScaleEventDef*> bestByGroup;

	for (const auto& pair : this->activeEvents)
	{
		const ActiveEvent& e = pair.second;
		const std::string& groupName = e.def.targetGroupName;

		auto itBest = bestByGroup.find(groupName);
		if (itBest == bestByGroup.end())
		{
			bestByGroup[groupName] = &e.def;
			continue;
		}

		const TimeScaleEventDef* currentBest = itBest->second;
		if (e.def.priority > currentBest->priority)
		{
			bestByGroup[groupName] = &e.def;
		}
		else if (e.def.priority == currentBest->priority)
		{
			bestByGroup[groupName] = &e.def;
		}
	}

	for (const auto& pair : bestByGroup)
	{
		const TimeScaleEventDef* def = pair.second;
		if (def != nullptr)
		{
			this->ApplyEventToGroup(*def);
		}
	}
}