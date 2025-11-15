/** @file   TimeScaleGroup.cpp
 *  @brief  複数オブジェクトをまとめて時間スケール制御するためのコンポーネント
 *  @date   2025/11/16
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/TimeScaleGroup.h"

//-----------------------------------------------------------------------------
// TimeScaleGroup class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
TimeScaleGroup::TimeScaleGroup(GameObject* _owner, bool _active)
	: Component(_owner, _active), scaleGroups()
{}

/// @brief 初期化処理
void TimeScaleGroup::Initialize()
{}

/// @brief 終了処理
void TimeScaleGroup::Dispose()
{
	this->ClearGroups();
}

/** @brief グループにメンバーを追加する（必要であればグループを作る）
 *  @param _groupName グループ名
 *  @param _timeScaleComponent 追加するコンポーネント
 *  @return メンバーの追加に成功したら true
 */
bool TimeScaleGroup::AddGroup(const std::string& _groupName, TimeScaleComponent* _timeScaleComponent)
{
	if (!this->scaleGroups.contains(_groupName))
	{
		// グループが存在しない場合のみ新規作成する
		this->scaleGroups.emplace(_groupName, ScaleGroupInfo{ _groupName, 1.0f, {} });
	}

	// グループ情報を取得する
	ScaleGroupInfo& groupInfo = this->scaleGroups.at(_groupName);

	// 重複追加を防止する
	if (std::find(groupInfo.members.begin(), groupInfo.members.end(), _timeScaleComponent) != groupInfo.members.end())
	{
		return false;
	}

	// メンバーに追加する
	groupInfo.members.push_back(_timeScaleComponent);

	return true;
}

/** @brief グループを新規作成する
 *  @param _groupName グループ名
 *  @return 追加に成功したら true。すでに存在する場合は false。
 */
bool TimeScaleGroup::AddGroup(const std::string& _groupName)
{
	// 同じ名前のグループが存在しないか確認する
	if (this->scaleGroups.contains(_groupName)) { return false; }

	// 新しいグループ情報を作成して追加する
	this->scaleGroups.emplace(_groupName, ScaleGroupInfo{ _groupName, 1.0f, {} });

	return true;
}

/** @brief 時間スケールを設定する
 *  @param const std::string& _name グループ名
 */
void TimeScaleGroup::SetTimeScale(const std::string& _name, float _scale)
{
    auto it = scaleGroups.find(_name);
	if (it == scaleGroups.end()) { return; }

    it->second.timeScale = _scale;

	for (auto* m : it->second.members)
	{
		m->SetTimeScale(_scale);
	}
}

/**@brief 時間スケールを取得する
 * @param const std::string& _groupName グループ名
 * @return float グループの時間スケール値
 */
float TimeScaleGroup::GetTimeScale(const std::string& _groupName) const
{
	// 同じ名前のグループが存在しない場合は何もしない
	if (this->scaleGroups.contains(_groupName))
	{
		// グループ情報を取得する
		const ScaleGroupInfo& groupInfo = this->scaleGroups.at(_groupName);
		return groupInfo.timeScale;
	}
	else
	{
		// デフォルトの時間スケールを返す	
		return 1.0f;
	}
}

/** @brief 指定のグループを削除する
 *	@param const std::string& _groupName グループ名
 */
void TimeScaleGroup::ResetGroup(const std::string& _groupName)
{
	if (this->scaleGroups.contains(_groupName))
	{
		this->scaleGroups.erase(_groupName);
	}
}
