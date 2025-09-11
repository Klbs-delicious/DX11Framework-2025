/**	@file	Application.cpp
*	@date	2025/09/10
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/Application.h"
#include "Framework/Utils/DebugHooks.h"
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/RenderSystem.h"
#include"Framework/Core/FPS.h"

#include "Scenes/SceneManager.h"
#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"

#include<iostream>

//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------
Application::AppConfig Application::appConfig = {};

//-----------------------------------------------------------------------------
// RenderSystem Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
 *	@param	const AppConfig _config	アプリケーション実行時の設定
 */
Application::Application(const AppConfig _config)
{
    // スリープ精度を1ms単位に強化
    timeBeginPeriod(1);

    Application::appConfig = _config;
}

/// @brief	デストラクタ
Application::~Application()
{
    // 精度設定を解除
    timeEndPeriod(1);
}

/// @brief 初期化処理
bool Application::Initialize()
{
    DebugHooks::Install();

    if (!WindowSystem::Initialize(Application::appConfig.screenWidth, Application::appConfig.screenHeight)) { return false; }
    if(!RenderSystem::Initialize()) { return false; }

    // 初期化成功
    return true;
}

/// @brief 実行処理
void Application::Run()
{
    if (Application::Initialize())
    {
        Application::MainLoop();
    }
    Application::ShutDown();
}

/// @brief	メインループ処理
void Application::MainLoop()
{
    MSG msg{};
    FPS fps(70);

    // ------------------------------ テスト ------------------------------
    // シーン構成の初期化
    auto factory = std::make_unique<SceneFactory>();
    factory->Register(SceneType::Test, [] { return std::make_unique<TestScene>(); });
    factory->Register(SceneType::Title, [] { return std::make_unique<TitleScene>(); });

    auto sceneManager = std::make_unique<SceneManager>(std::move(factory));
    sceneManager->SetTransitionCallback([raw = sceneManager.get()](SceneType next) {
        std::cout << "シーン遷移時の演出を行いました。\n";
        raw->NotifyTransitionReady(next);
        });

    sceneManager->RequestSceneChange(SceneType::Test);
    // ------------------------------ テスト ------------------------------

    while (msg.message != WM_QUIT)
    {
        fps.Tick();
        fps.Measure();

        // 瞬間FPS
        float FPS = 1.0f / fps.DeltaSec();
        std::cout << "Measured FPS: " << 1.0f / fps.DeltaSec() << std::endl;

        // 以下、普段の更新・描画
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN) {
                switch (msg.wParam) {
                    // ------------------------------ テスト ------------------------------
                case VK_SPACE:  sceneManager->RequestSceneChange(SceneType::Test);  break;
                case VK_RETURN: sceneManager->RequestSceneChange(SceneType::Title); break;
                    // ------------------------------ テスト ------------------------------
                }
            }
        }

        sceneManager->Update(FPS);
        RenderSystem::BeginRender();
        sceneManager->Draw();
        RenderSystem::EndRender();
    }
}

/// @brief	終了処理
void Application::ShutDown()
{
    RenderSystem::Finalize();
    WindowSystem::Finalize();
}