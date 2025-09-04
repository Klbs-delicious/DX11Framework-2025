/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/RenderSystem.h"
#include "Framework/Utils/DebugHooks.h"
#include "Framework/Core/FPS.h"

#include "Framework/Scenes/SceneFactory.h"
#include "Scenes/SceneManager.h"
#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"


#include<iostream>
#include <Windows.h> // timeBeginPeriod 用
#pragma comment(lib, "Winmm.lib")
//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{
    timeBeginPeriod(1);               // スリープ精度を1ms単位に強化
    DebugHooks::Install();
    FPS fps(70);

    WindowSystem::Initialize(640, 480);
    RenderSystem::Initialize();

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

    // FPSログ用変数
    float fpsAccumulator = 0.0f;
    int fpsFrameCount = 0;
    fps.ResetTime();

    // メインループ
    MSG msg{};
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
                case VK_SPACE:  sceneManager->RequestSceneChange(SceneType::Test);  break;
                case VK_RETURN: sceneManager->RequestSceneChange(SceneType::Title); break;
                }
            }
        }

        sceneManager->Update(FPS);
        RenderSystem::BeginRender();
        sceneManager->Draw();
        RenderSystem::EndRender();
    }

    RenderSystem::Finalize();
    WindowSystem::Finalize();
    timeEndPeriod(1); // 精度設定を解除

    return 0;
}
