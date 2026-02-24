/** @file   DebugSceneChanger.cpp
 *  @brief  デバッグ用シーン切り替えコンポーネント
 *  @date   2026/01/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/DebugSceneChanger.h"

#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"

#include<iostream>

//-----------------------------------------------------------------------------
// DebugSceneChanger class
//-----------------------------------------------------------------------------

DebugSceneChanger::DebugSceneChanger(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	sceneManager(SystemLocator::Get<SceneManager>()),
	inputSystem(SystemLocator::Get<InputSystem>()),
	currentSceneType(SceneType::Title)
{}

void DebugSceneChanger::Initialize()
{
	// 現在のシーンタイプを取得
	this->currentSceneType = this->sceneManager.GetCurrentSceneType();

	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("SceneChangeFlg", static_cast<int>(DirectInputDevice::KeyboardKey::LShift));
	this->inputSystem.RegisterKeyBinding("NextScene", static_cast<int>(DirectInputDevice::KeyboardKey::K));
	this->inputSystem.RegisterKeyBinding("PreviousScene", static_cast<int>(DirectInputDevice::KeyboardKey::L));
	this->inputSystem.RegisterKeyBinding("ReloadScene", static_cast<int>(DirectInputDevice::KeyboardKey::R));
}	

void DebugSceneChanger::Dispose()
{
}

void DebugSceneChanger::Update(float _deltaTime)
{
	// 現在のシーンタイプを保存
	auto nextScene = static_cast<int>(this->currentSceneType);

	// LShiftキーが押されている場合のみシーン切り替えを許可
	if (this->inputSystem.IsActionPressed("SceneChangeFlg"))
	{
		if (this->inputSystem.IsActionTriggered("ReloadScene"))
		{
			std::cout << "[DebugSceneChanger] Reloading scene "
				<< static_cast<int>(this->currentSceneType) << ".\n";

			this->sceneManager.RequestSceneChange(this->currentSceneType);
			return;
		}

		// 右矢印キーで次のシーンへ
		if (this->inputSystem.IsActionTriggered("NextScene"))
		{
			nextScene += 1;
		}
		// 左矢印キーで前のシーンへ
		if (this->inputSystem.IsActionTriggered("PreviousScene"))
		{
			nextScene -= 1;
		}

		// シーンタイプの範囲を循環させる
		if (nextScene >= static_cast<int>(SceneType::Max))
		{
			nextScene = 0;
		}
		else if (nextScene < 0)
		{
			nextScene = static_cast<int>(SceneType::Num);
		}
	}	
	else { return; }

	// シーンが変更されていればシーン遷移をリクエスト
	if (static_cast<int>(this->currentSceneType) != nextScene)
	{
		std::cout<< "[DebugSceneChanger] Changing scene from "
			<< static_cast<int>(this->currentSceneType) << " to " << nextScene << ".\n";

		this->sceneManager.RequestSceneChange(static_cast<SceneType>(nextScene));
	}
}

