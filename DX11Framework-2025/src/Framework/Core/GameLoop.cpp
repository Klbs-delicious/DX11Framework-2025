/** @file   GameLoop.cpp
*   @date   2025/09/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/GameLoop.h"
#include"Framework/Core/SystemLocator.h"
#include"Framework/Core/DirectInputDevice.h"

#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"

#include<iostream>

//-----------------------------------------------------------------------------
// GameLoop Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
GameLoop::GameLoop() :isRunning(true), gameState(GameState::Play) {}
/// @brief	デストラクタ
GameLoop::~GameLoop(){}

/**	@brief		初期化処理を行う
 */
void GameLoop::Initialize()
{
    // シーン構成の初期化
    auto factory = std::make_unique<SceneFactory>();
    factory->Register(SceneType::Test, [] { return std::make_unique<TestScene>(); });
    factory->Register(SceneType::Title, [] { return std::make_unique<TitleScene>(); });

    // シーン管理の作成
    this->sceneManager = std::make_unique<SceneManager>(std::move(factory));
    this->sceneManager->SetTransitionCallback([raw = this->sceneManager.get()](SceneType _next) {
        std::cout << "シーン遷移時の演出を行いました。\n";
        raw->NotifyTransitionReady(_next);
        });

    // シーン管理を登録
    SystemLocator::Register<SceneManager>(this->sceneManager.get());

    // 入力管理を行うクラスの生成と登録
    this->inputSystem = std::make_unique<InputSystem>();
    SystemLocator::Register<InputSystem>(this->inputSystem.get());

    // 入力デバイスの登録
    auto& window = SystemLocator::Get<WindowSystem>();
    auto directInput = std::make_unique<DirectInputDevice>();
    if (!directInput->Initialize(window.GetHInstance(), window.GetWindow())) { return; }
    this->inputSystem->RegisterDevice(std::move(directInput));

    // キーバインドの登録
    this->inputSystem->RegisterKeyBinding("Space", static_cast<int>(DirectInputDevice::KeyboardKey::Space));
    this->inputSystem->RegisterKeyBinding("DownArrow", static_cast<int>(DirectInputDevice::KeyboardKey::DownArrow));
    this->inputSystem->RegisterKeyBinding("SceneChangeTest", static_cast<int>(DirectInputDevice::KeyboardKey::D));
    this->inputSystem->RegisterKeyBinding("SceneChangeTitle", static_cast<int>(DirectInputDevice::KeyboardKey::A));
    this->inputSystem->RegisterKeyBinding("GameExit", static_cast<int>(DirectInputDevice::KeyboardKey::K));

    // シーンの変更
    this->sceneManager->RequestSceneChange(SceneType::Test);
}

/**	@brief		更新処理を行う
 *	@param		float _deltaTime	デルタタイム
 */
void GameLoop::Update(float _deltaTime)
{
    this->inputSystem->Update();
    this->sceneManager->Update(_deltaTime);
}

/**	@brief		描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 */
void GameLoop::Draw()
{
    this->sceneManager->Draw();
}

/**	@brief		終了処理を行う
 */
void GameLoop::Dispose()
{
    SystemLocator::Unregister<SceneManager>();

    this->inputSystem.reset();
    this->sceneManager.reset();
}