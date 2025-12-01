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
	timeScaleGroup(nullptr)
{}

/// @brief 初期化処理
void TimeScaleTestComponent::Initialize()
{
	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("Slow_Global", static_cast<int>(DirectInputDevice::KeyboardKey::Space));
	this->inputSystem.RegisterKeyBinding("Slow_GameObject_1", static_cast<int>(DirectInputDevice::KeyboardKey::J));
	this->inputSystem.RegisterKeyBinding("Slow_GameObject_2", static_cast<int>(DirectInputDevice::KeyboardKey::K));
	this->inputSystem.RegisterKeyBinding("Slow_GameObject_3", static_cast<int>(DirectInputDevice::KeyboardKey::L));
}

/// @brief 終了処理
void TimeScaleTestComponent::Dispose()
{}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void TimeScaleTestComponent::Update(float _deltaTime)
{
	if (!this->timeScaleGroup) { return; }

	// スペースキーでグローバルタイムスケールを0.1に変更する
	if (this->inputSystem.IsActionPressed("Slow_Global"))
	{
		this->timeScaleSystem.SetGlobalScale(0.5f);
	}
	else { this->timeScaleSystem.SetGlobalScale(1.0f); }

	// 1～3キーで各ゲームオブジェクトのタイムスケールを0.1に変更する
	for (int i = 1; i <= 3; i++)
	{
		std::string actionName = "Slow_GameObject_" + std::to_string(i);
		std::string groupName = "EnemyGroup_" + std::to_string(i);

		if (this->inputSystem.IsActionTriggered(actionName))
		{
			this->timeScaleGroup->SetGroupScale(groupName, 0.5f);
		}
		if (this->inputSystem.isActionReleased(actionName))
		{
			this->timeScaleGroup->SetGroupScale(groupName, 1.0f);
		}
	}
}