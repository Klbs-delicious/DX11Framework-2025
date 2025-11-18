/** @file   GameLoop.cpp
*   @date   2025/09/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include<iostream>

#include"Include/Framework/Core/GameLoop.h"
#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Core/DirectInputDevice.h"
#include"Include/Framework/Core/ResourceHub.h"

#include "Include/Scenes/TestScene.h"
#include "Include/Scenes/TitleScene.h"

//-----------------------------------------------------------------------------
// GameLoop Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
GameLoop::GameLoop() :isRunning(true), gameState(GameState::Play), timeSystem(60) {}
/// @brief	デストラクタ
GameLoop::~GameLoop() { this->Dispose(); }

/// @brief		初期化処理を行う
void GameLoop::Initialize()
{    
    //--------------------------------------------------------------------------    
    // ResourceHubに登録・管理する
    //--------------------------------------------------------------------------    

    // 画像管理クラスをリソース管理クラスに登録
    this->spriteManager = std::make_unique<SpriteManager>();
    ResourceHub::Register(this->spriteManager.get());

    // シェーダー管理クラスをリソース管理クラスに登録
    this->shaderManager = std::make_unique<ShaderManager>();
    ResourceHub::Register(this->shaderManager.get());

    // マテリアル管理クラスをリソース管理クラスに登録
    this->materialManager = std::make_unique<MaterialManager>();
    ResourceHub::Register(this->materialManager.get());

    // メッシュ管理クラスをリソース管理クラスに登録
    this->meshManager = std::make_unique<MeshManager>();
    ResourceHub::Register(this->meshManager.get());

    this->services = {
        &ResourceHub::Get<SpriteManager>(),
        &ResourceHub::Get<MaterialManager>(),
        &ResourceHub::Get<MeshManager>(),
        &ResourceHub::Get<ShaderManager>(),
    };

    //--------------------------------------------------------------------------    
    // SystemLocatorに登録・管理する
    //--------------------------------------------------------------------------  
    
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

    // 物理システムの管理
	this->physicsSystem = std::make_unique<Framework::Physics::PhysicsSystem>();
    if (!this->physicsSystem->Initialize()) 
    {
        std::cerr << "PhysicsSystemの初期化に失敗しました。\n";
        return;
	}
    SystemLocator::Register<Framework::Physics::PhysicsSystem>(this->physicsSystem.get());

    // ゲームオブジェクトの管理を行うクラスの生成と登録
    this->gameObjectManager = std::make_unique<GameObjectManager>(&services);
    SystemLocator::Register<GameObjectManager>(this->gameObjectManager.get());

    // 時間スケールの管理
    this->timeScaleSystem = std::make_unique<TimeScaleSystem>();
    SystemLocator::Register<TimeScaleSystem>(this->timeScaleSystem.get());

    // 入力管理
    this->inputSystem = std::make_unique<InputSystem>();
    SystemLocator::Register<InputSystem>(this->inputSystem.get());

    //--------------------------------------------------------------------------    
    // 各システムの設定
    //--------------------------------------------------------------------------    

    // 入力デバイスの登録
    auto& window = SystemLocator::Get<WindowSystem>();
    auto directInput = std::make_unique<DirectInputDevice>();
    if (!directInput->Initialize(window.GetHInstance(), window.GetWindow())) { return; }
    this->inputSystem->RegisterDevice(std::move(directInput));

    // キーバインドの登録
    this->inputSystem->RegisterKeyBinding("SceneChangeTest", static_cast<int>(DirectInputDevice::KeyboardKey::D));
    this->inputSystem->RegisterKeyBinding("SceneChangeTitle", static_cast<int>(DirectInputDevice::KeyboardKey::A));
    this->inputSystem->RegisterKeyBinding("GameExit", static_cast<int>(DirectInputDevice::KeyboardKey::K));

    // シーンの変更
    this->sceneManager->RequestSceneChange(SceneType::Test);
}

/// @brief		更新処理を行う
void GameLoop::Update()
{
    if (!this->isRunning) { return; }

    // デルタタイムの計算
    this->timeSystem.TickRawDelta();
    float delta = this->timeSystem.RawDelta();
    float fixedDelta = this->timeSystem.FixedDelta();

#ifdef _DEBUG
    // 瞬間FPS
    std::cout << "可変FPS: " << 1.0f / delta << std::endl;
    std::cout << "固定FPS: " << 1.0f / fixedDelta << std::endl;
#endif

    //-------------------------------------------------------------
    // 可変ステップ更新
    //-------------------------------------------------------------
    this->inputSystem->Update();
    this->sceneManager->Update(delta);

	//-------------------------------------------------------------
	// 固定ステップ更新
	//-------------------------------------------------------------
    while (this->timeSystem.ShouldRunFixedStep())
    {
        this->physicsSystem->Step(fixedDelta);
        this->timeSystem.ConsumeFixedStep();
    }
}

/// @brief		描画処理を行う
void GameLoop::Draw()
{
    if (!this->isRunning) { return; }

    this->sceneManager->Draw();
}

/// @brief		終了処理を行う
void GameLoop::Dispose()
{
    SystemLocator::Unregister<InputSystem>();
    this->inputSystem.reset();

    SystemLocator::Unregister<TimeScaleSystem>();
    this->timeScaleSystem.reset();

    SystemLocator::Unregister<SceneManager>();
    this->sceneManager.reset();

    SystemLocator::Unregister<GameObjectManager>();
    this->gameObjectManager.reset();

    SystemLocator::Unregister<Framework::Physics::PhysicsSystem>();
    this->physicsSystem.reset();

    ResourceHub::Unregister<MeshManager>();
    this->meshManager.reset();
    
    ResourceHub::Unregister<MaterialManager>();
    this->materialManager.reset();
    
    ResourceHub::Unregister<ShaderManager>();
    this->shaderManager.reset();

    ResourceHub::Unregister<SpriteManager>();
    this->spriteManager.reset();
}