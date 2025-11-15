/** @file   TimeScaleComponent.cpp
 *  @brief  オブジェクトごとの時間スケールを保持・管理するコンポーネントの実装
 *  @date   2025/11/15
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/TimeScaleComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"

//-----------------------------------------------------------------------------
// TimeScaleComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
TimeScaleComponent::TimeScaleComponent(GameObject* _owner, bool _active)
    : Component(_owner, _active),
    timeScaleSystem(SystemLocator::Get<TimeScaleSystem>()), 
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

/// @brief 蓄積された時間スケールを取得する
float TimeScaleComponent::GetAccumulatedScale() const
{
    float scale = this->timeScale;

    GameObject* p = this->Owner()->Parent();
    while (p)
    {
		// 親のTimeScaleComponentを取得してスケールを掛ける
        auto t = p->TimeScale();
        if (t) { scale *= t->GetTimeScale(); }

        p = p->Parent();
    }

	// グローバルスケールを掛ける
    scale *= this->timeScaleSystem.GlobalScale();

    return scale;
}

/**@brief デルタタイムに時間スケールを適用する
 * @param _baseDelta
 * @return
 */
float TimeScaleComponent::ApplyTimeScale(float _baseDelta) const
{
    return _baseDelta * this->GetAccumulatedScale();
}