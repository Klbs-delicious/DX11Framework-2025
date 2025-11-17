/**	@file	Application.cpp
*	@date	2025/09/10
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Core/Application.h"
#include "Include/Framework/Core/SystemLocator.h"

#include "Include/Framework/Utils/DebugHooks.h"

#include<iostream>

//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------
Application::AppConfig          Application::appConfig = {};
std::unique_ptr<WindowSystem>   Application::windowSystem;
std::unique_ptr<D3D11System>    Application::d3d11System;
std::unique_ptr<RenderSystem>   Application::renderSystem;
std::unique_ptr<GameLoop>       Application::gameLoop;

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

/** @brief      初期化処理
*   @details    - 使用するシステムをSystemLocatorに登録する（今後SystemLocatorからシステムを取得して使用できるようにする）
*               - 生成順序に気を付ける
*/  
bool Application::Initialize()
{
    DebugHooks::Install();

    // Window の生成と登録
    Application::windowSystem = std::make_unique<WindowSystem>();
    if (!Application::windowSystem->Initialize(Application::appConfig.screenWidth, Application::appConfig.screenHeight)) { return false; }
    SystemLocator::Register<WindowSystem>(Application::windowSystem.get());

    // D3D11System の生成と登録
    Application::d3d11System = std::make_unique<D3D11System>(Application::windowSystem.get());
    if (!Application::d3d11System->Initialize()) { return false; }

    SystemLocator::Register<D3D11System>(Application::d3d11System.get());

    // RenderSystem の生成と登録
    Application::renderSystem = std::make_unique<RenderSystem>(Application::d3d11System.get(), Application::windowSystem.get());
    if (!Application::renderSystem->Initialize()) { return false; }
    SystemLocator::Register<RenderSystem>(Application::renderSystem.get());

    // ゲーム進行
    Application::gameLoop = std::make_unique<GameLoop>();

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
    Application::gameLoop->Initialize();

    while (msg.message != WM_QUIT && Application::gameLoop->IsRunning())
    {

        // 以下、普段の更新・描画
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_KEYDOWN) {
                //switch (msg.wParam) {
                //    // ------------------------------ テスト ------------------------------
                ////case VK_SPACE:  sceneManager->RequestSceneChange(SceneType::Test);  break;
                ////case VK_RETURN: sceneManager->RequestSceneChange(SceneType::Title); break;
                ////case VK_RETURN: Application::gameLoop->RequestExit(); break;
                //    // ------------------------------ テスト ------------------------------
                //}
            }
        }

        // テスト的にメンバに持たずに直接取得する-----------------------------------------------------
        auto& input = SystemLocator::Get<InputSystem>();
        if (input.IsActionTriggered("GameExit")) { Application::gameLoop->RequestExit(); }
        // -------------------------------------------------------------------------------------------

        Application::gameLoop->Update();
        Application::renderSystem->BeginRender();
        Application::gameLoop->Draw();
        Application::renderSystem->EndRender();
    }
}

/** @brief      終了処理
*   @details    - システムをSystemLocatorに登録した順から逆に解除する
*               - システムの破棄順に注意する
*/
void Application::ShutDown()
{
    Application::gameLoop.reset();

    Application::renderSystem.reset();
    SystemLocator::Unregister<RenderSystem>();

    Application::d3d11System.reset();
    SystemLocator::Unregister<D3D11System>();

    Application::windowSystem.reset();
    SystemLocator::Unregister<WindowSystem>();
}