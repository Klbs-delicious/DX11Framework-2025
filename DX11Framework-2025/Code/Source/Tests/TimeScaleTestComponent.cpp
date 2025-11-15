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

#include "Include/Framework/Entities/GameObject.h"

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
	timeScaleSystem(SystemLocator::Get<TimeScaleSystem>()),
	timeScaleComponents()
{}

/// @brief 初期化処理
void TimeScaleTestComponent::Initialize()
{
	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("Slow_Global", static_cast<int>(DirectInputDevice::KeyboardKey::Space));
	this->inputSystem.RegisterKeyBinding("Slow_GameObject", static_cast<int>(DirectInputDevice::KeyboardKey::V));
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
	if (this->inputSystem.IsActionPressed("Slow_Global"))
	{
		this->timeScaleSystem.SetGlobalScale(0.1f); 
	}
	else { this->timeScaleSystem.SetGlobalScale(1.0f); }

	// Vキーでこのオブジェクトのタイムスケールを0.1に変更する
	if (this->inputSystem.IsActionTriggered("Slow_GameObject"))
	{
		for (auto& timeScale : this->timeScaleComponents)
		{
			timeScale->SetTimeScale(0.1f);
			this->Owner()->TimeScale()->SetTimeScale(0.1f);
		}
	}

	// Vキーを離したら元に戻す
	if (this->inputSystem.isActionReleased("Slow_GameObject"))
	{
		for (auto& timeScale : this->timeScaleComponents)
		{
			timeScale->SetTimeScale(1.0f);
			this->Owner()->TimeScale()->SetTimeScale(1.0f);
		}
	}
}

/**@brief TimeScaleComponentを追加する
 * @param _component
 */
void TimeScaleTestComponent::AddTimeScaleComponent(TimeScaleComponent* _component)
{
	this->timeScaleComponents.push_back(_component);
}