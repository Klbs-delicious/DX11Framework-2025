/** @file   TimeScaleComponent.h
 *  @brief  オブジェクトごとの時間スケールを保持・管理するコンポーネント
 *  @date   2025/11/15
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

#include<string>

struct ScaleGroupInfo;

 /** @class TimeScaleComponent
  *  @brief オブジェクトごとの時間スケールを保持・管理するコンポーネント
  *  @details - Componentを継承し、各オブジェクトの個別時間係数を管理する
  */
class TimeScaleComponent : public Component
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	TimeScaleComponent(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~TimeScaleComponent() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/// @brief 時間スケールを設定する
	void SetTimeScale(float _scale);

	/// @brief 時間スケールを取得する
	[[nodiscard]] float GetTimeScale() const;

	/// @brief 蓄積された時間スケールを取得する
	[[nodiscard]] float GetAccumulatedScale() const;

	/**@brief デルタタイムに時間スケールを適用する
	 * @param _baseDelta
	 * @return
	 */
	[[nodiscard]] float ApplyTimeScale(float _baseDelta) const;

	/**@brief 時間スケールレイヤーを設定する
	 * @param _layer 
	 */
	void SetTimeScaleLayer(TimeScaleLayer _layer) { this->timeScaleLayer = _layer; }

	/**@brief 時間スケールレイヤーを取得する
	 * @return レイヤー情報
	 */
	[[nodiscard]] TimeScaleLayer GetTimeScaleLayer() const { return this->timeScaleLayer; }

	/**@brief グループ名を設定する
	 * @return グループ名
	 */
	[[nodiscard]] const std::string& GetGroupName() const { return this->groupName; }

	/// @brief グループ名を設定する
	void SetignoreGroup(bool _ignore) { this->ignoreGroup = _ignore; }
	void SetIgnoreLayer(bool _ignore) { this->ignoreLayer = _ignore; }
	void SetIgnoreGlobal(bool _ignore) { this->ignoreGlobal = _ignore; }

	/// @brief グループの時間スケールを無視するかどうかを取得する
	[[nodiscard]] bool IsIgnoreGroup() const { return this->ignoreGroup; }
	[[nodiscard]] bool IsIgnoreLayer() const { return this->ignoreLayer; }
	[[nodiscard]] bool IsIgnoreGlobal() const { return this->ignoreGlobal; }

	/**@brief 所属するグループ情報を設定・取得する
	 * @param _groupInfo グループ情報へのポインタ
	 */
	void SetGroupInfo(ScaleGroupInfo* _groupInfo) { this->groupInfo = _groupInfo; }

	/**@brief 所属するグループ情報を取得する
	 * @return グループ情報へのポインタ
	 */
	[[nodiscard]] const ScaleGroupInfo* GetGroupInfo() const;

private:
	TimeScaleSystem& timeScaleSystem;	///< 時間スケール管理システムの参照

	float timeScale;					///< オブジェクト固有の時間倍率
	TimeScaleLayer timeScaleLayer;		///< オブジェクトの時間スケールレイヤー
	std::string groupName;              ///< 所属するグループ名
	ScaleGroupInfo* groupInfo = nullptr;///< 所属するグループ情報へのポインタ

	bool ignoreGroup;					///< グループの時間スケールを無視するかどうか
	bool ignoreLayer;					///< レイヤーの時間スケールを無視するかどうか
	bool ignoreGlobal;					///< グローバルの時間スケールを無視するかどうか
};