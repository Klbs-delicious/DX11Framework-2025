﻿/** @file   GameLoop.cpp
*   @date   2025/09/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/GameLoop.h"
#include"Framework/Core/SystemLocator.h"
#include"Framework/Core/DirectInputDevice.h"
#include"Framework/Core/ResourceHub.h"

#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"

#include<iostream>

//-----------------------------------------------------------------------------
// GameLoop Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
GameLoop::GameLoop() :isRunning(true), gameState(GameState::Play) {}
/// @brief	デストラクタ
GameLoop::~GameLoop(){ this->Dispose(); }

/**	@brief		初期化処理を行う
 */
void GameLoop::Initialize()
{
    // 入力管理を行うクラスの生成と登録
    this->inputSystem = std::make_unique<InputSystem>();
    SystemLocator::Register<InputSystem>(this->inputSystem.get());

    // ゲームオブジェクトの管理を行うクラスの生成と登録
    this->gameObjectManager = std::make_unique<GameObjectManager>();
    SystemLocator::Register<GameObjectManager>(this->gameObjectManager.get());

    // シーン構成の初期化
    auto factory = std::make_unique<SceneFactory>();
    factory->Register(SceneType::Test, [](GameObjectManager& manager) {
        return std::make_unique<TestScene>(manager);
        });
    factory->Register(SceneType::Title, [](GameObjectManager& manager) {
        return std::make_unique<TitleScene>(manager);
        });

    // シーン管理の作成
    this->sceneManager = std::make_unique<SceneManager>(std::move(factory));
    this->sceneManager->SetTransitionCallback([raw = this->sceneManager.get()](SceneType _next) {
        std::cout << "シーン遷移時の演出を行いました。\n";
        raw->NotifyTransitionReady(_next);
        });

    // シーン管理を登録
    SystemLocator::Register<SceneManager>(this->sceneManager.get());

    // 入力デバイスの登録
    auto& window = SystemLocator::Get<WindowSystem>();
    auto directInput = std::make_unique<DirectInputDevice>();
    if (!directInput->Initialize(window.GetHInstance(), window.GetWindow())) { return; }
    this->inputSystem->RegisterDevice(std::move(directInput));

    // キーバインドの登録
    this->inputSystem->RegisterKeyBinding("SceneChangeTest", static_cast<int>(DirectInputDevice::KeyboardKey::D));
    this->inputSystem->RegisterKeyBinding("SceneChangeTitle", static_cast<int>(DirectInputDevice::KeyboardKey::A));
    this->inputSystem->RegisterKeyBinding("GameExit", static_cast<int>(DirectInputDevice::KeyboardKey::K));

    // 画像管理クラスをリソース管理クラスに登録
    this->spriteManager = std::make_unique<SpriteManager>();
    ResourceHub::Register(this->spriteManager.get());

	// シェーダー管理クラスをリソース管理クラスに登録
	this->shaderManager = std::make_unique<ShaderManager>();
    ResourceHub::Register(this->shaderManager.get());

    // シーンの変更
    this->sceneManager->RequestSceneChange(SceneType::Test);
}

/**	@brief		更新処理を行う
 *	@param		float _deltaTime	デルタタイム
 */
void GameLoop::Update(float _deltaTime)
{
    if (!this->isRunning) { return; }

    this->inputSystem->Update();
    this->sceneManager->Update(_deltaTime);
}

/**	@brief		描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 */
void GameLoop::Draw()
{
    if (!this->isRunning) { return; }

    this->sceneManager->Draw();
}

/**	@brief		終了処理を行う
 */
void GameLoop::Dispose()
{
    SystemLocator::Unregister<ShaderManager>();
    ResourceHub::Unregister<SpriteManager>();
    SystemLocator::Unregister<SceneManager>();
    SystemLocator::Unregister<GameObjectManager>();

    this->shaderManager.reset();
    this->spriteManager.reset();
    this->sceneManager.reset();
    this->gameObjectManager.reset();
    this->inputSystem.reset();
}