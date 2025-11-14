/** @file   TimeScaleTestComponent.cpp
 *  @brief  時間スケールの挙動を検証するテスト用コンポーネントの実装
 *  @date   2025/11/12
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Tests/TimeScaleTestComponent.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"

//-----------------------------------------------------------------------------
// TimeScaleTestComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
TimeScaleTestComponent::TimeScaleTestComponent(GameObject* _owner, bool _active)
	: Component(_owner, _active),
	inputSystem(SystemLocator::Get<InputSystem>()),
	timeScaleSystem(SystemLocator::Get<TimeScaleSystem>())
{}

/// @brief 初期化処理
void TimeScaleTestComponent::Initialize()
{
	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("Slow_Global", static_cast<int>(DirectInputDevice::KeyboardKey::Space));
}

/// @brief 終了処理
void TimeScaleTestComponent::Dispose()
{}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void TimeScaleTestComponent::Update(float _deltaTime)
{
	// スペースキーでグローバルタイムスケールを0.1に変更する
	if (this->inputSystem.IsActionPressed("Slow_Global")) { this->timeScaleSystem.SetGlobalScale(0.1f); }
	else { this->timeScaleSystem.SetGlobalScale(1.0f); }
}
