/** @file   TimeScaleSystem.h
 *  @brief  時間スケール管理システム
 *  @date   2026/02/15
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Utils/NonCopyable.h"

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

//-----------------------------------------------------------------------------
// Enums / Structs
//-----------------------------------------------------------------------------

/// @brief 時間スケールのレイヤー種別
enum class TimeScaleLayer : uint8_t
{
	Default = 0,	///< 通常のゲーム進行
	UI,				///< UI 表示など
	Effect,			///< 演出・エフェクト
	Max				///< レイヤー最大数
};

/// @brief TimeScale イベント種別
enum class TimeScaleEventId : uint8_t
{
	JustDodge = 0,	///< 世界スロー
	HitStop,		///< 超短時間停止


	TestDodge,		///< テスト用イベント
	Max
};

/// @brief 同一イベント再成立時の扱い
enum class TimeScaleStackPolicy : uint8_t
{
	Extend = 0,	///< 残り時間を延長する（基本）
	Overwrite	///< 残り時間を上書きする
};

/** @struct TimeScaleEventDef
 *  @brief TimeScale イベントの定義
 */
struct TimeScaleEventDef
{
	TimeScaleEventId id = TimeScaleEventId::JustDodge;					///< イベントID
	std::string targetGroupName;										///< 適用対象グループ名
	float scale = 1.0f;													///< 適用倍率
	float durationRawSec = 0.0f;										///< 継続時間（raw秒）
	int priority = 0;													///< 優先度（同一グループ競合で使用）
	TimeScaleStackPolicy stackPolicy = TimeScaleStackPolicy::Extend;	///< 再成立時挙動
};

//-----------------------------------------------------------------------------
// TimeScaleSystem class
//-----------------------------------------------------------------------------

/** @class  TimeScaleSystem
 *  @brief  グローバル／レイヤー／グループの時間スケールを一元管理する
 *  @details
 *          - Global / Layer は従来通り保持
 *          - Group は TimeScaleSystem が倍率表を保持（TimeScaleGroup コンポーネント廃止）
 *          - イベントは ID でリクエストし、定義から対象グループ倍率を書き換える
 *          - 継続時間は rawDelta で管理し、時間切れで確実復帰する
 */
class TimeScaleSystem : private NonCopyable
{
public:
	/// @brief コンストラクタ
	TimeScaleSystem();

	/// @brief デストラクタ
	~TimeScaleSystem() = default;

	/** @brief 毎フレーム更新（イベント残り時間を rawDelta で減算）
	 *  @param _rawDeltaSec rawDelta（秒）
	 */
	void Update(float _rawDeltaSec);

	/** @brief グローバル TimeScale を設定する
	 *  @param _scale 設定するスケール値
	 */
	void SetGlobalScale(float _scale);

	/// @brief グローバル TimeScale を取得する
	[[nodiscard]] float GlobalScale() const;

	/** @brief レイヤーの TimeScale を設定する
	 *  @param _layer 対象レイヤー
	 *  @param _scale 設定するスケール値
	 */
	void SetLayerScale(TimeScaleLayer _layer, float _scale);

	/** @brief レイヤーの TimeScale を取得する
	 *  @param _layer 対象レイヤー
	 *  @return 指定レイヤーのスケール値
	 */
	[[nodiscard]] float LayerScale(TimeScaleLayer _layer) const;

	/** @brief 合成済み TimeScale（Global × Layer）を取得する
	 *  @param _layer 対象レイヤー
	 *  @return 合成されたスケール値
	 */
	[[nodiscard]] float CombinedScale(TimeScaleLayer _layer) const;

	/** @brief グループ倍率（基準値）を設定する
	 *  @param _groupName グループ名
	 *  @param _scale 基準倍率
	 */
	void SetGroupBaseScale(const std::string& _groupName, float _scale);

	/** @brief グループ倍率（最終適用値）を取得する
	 *  @param _groupName グループ名
	 *  @return 最終適用倍率（未登録は 1.0f）
	 */
	[[nodiscard]] float GetGroupScale(const std::string& _groupName) const;

	/** @brief TimeScale イベントをリクエストする（IDのみ）
	 *  @param _eventId イベントID
	 */
	void RequestEvent(TimeScaleEventId _eventId);

	/** @brief イベント定義を差し替える（必要な場合のみ使用）
	 *  @param _def 定義
	 */
	void SetEventDef(const TimeScaleEventDef& _def);

private:
	struct EventKey
	{
		TimeScaleEventId id;
		std::string groupName;

		bool operator==(const EventKey& _rhs) const
		{
			return (id == _rhs.id) && (groupName == _rhs.groupName);
		}
	};

	struct EventKeyHash
	{
		size_t operator()(const EventKey& _key) const
		{
			const size_t h1 = std::hash<uint8_t>()(static_cast<uint8_t>(_key.id));
			const size_t h2 = std::hash<std::string>()(_key.groupName);
			return (h1 * 1315423911u) ^ h2;
		}
	};

	struct ActiveEvent
	{
		TimeScaleEventDef def;			///< 定義（コピー保持）
		float remainingRawSec = 0.0f;	///< 残り時間（raw秒）
	};
	void ApplyEventToGroup(const TimeScaleEventDef& _def);
	void RebuildAppliedGroupScales();

	[[nodiscard]] const TimeScaleEventDef* FindEventDef(TimeScaleEventId _eventId) const;

private:
	float globalScale;															///< 全体に適用される時間スケール
	std::array<float, static_cast<size_t>(TimeScaleLayer::Max)> layerScales;	///< レイヤーごとの時間スケール

	std::unordered_map<std::string, float> groupBaseScales;		///< グループ倍率（基準値）
	std::unordered_map<std::string, float> groupAppliedScales;	///< グループ倍率（最終適用値）

	std::array<TimeScaleEventDef, static_cast<size_t>(TimeScaleEventId::Max)> eventDefs;	///< ID→定義
	std::unordered_map<EventKey, ActiveEvent, EventKeyHash> activeEvents;					///< 実行中イベント（ID+Groupで一意）
};