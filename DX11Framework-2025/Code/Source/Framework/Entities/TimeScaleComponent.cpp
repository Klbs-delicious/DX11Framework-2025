/** @file   TimeScaleComponent.cpp
 *  @brief  オブジェクトごとの時間スケールを保持・管理するコンポーネントの実装
 *  @date   2025/11/15
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/TimeScaleComponent.h"

//-----------------------------------------------------------------------------
// TimeScaleComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
TimeScaleComponent::TimeScaleComponent(GameObject* _owner, bool _active)
	: Component(_owner, _active),
	timeScale(1.0f)
{}

/// @brief 初期化処理
void TimeScaleComponent::Initialize()
{
	timeScale = 1.0f;
}

/// @brief 終了処理
void TimeScaleComponent::Dispose()
{}

/** @brief 時間スケールを設定する
 *  @param _scale 設定するスケール値
 */
void TimeScaleComponent::SetTimeScale(float _scale)
{
	this->timeScale = _scale;
}

/// @brief 時間スケールを取得する
float TimeScaleComponent::GetTimeScale() const
{
	return this->timeScale;
}
