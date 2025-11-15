/** @file   TimeScaleComponent.h
 *  @brief  オブジェクトごとの時間スケールを保持・管理するコンポーネント
 *  @date   2025/11/15
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

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

private:
	TimeScaleSystem& timeScaleSystem;	///< 時間スケール管理システムの参照

	float timeScale;					///< オブジェクト固有の時間倍率
};
