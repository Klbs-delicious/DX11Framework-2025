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

#include "Framework/Scenes/SceneFactory.h"
#include "Scenes/TestScene.h"

//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{    
	// Debug、Releseで適切なハンドラをセット
	DebugHooks::Install();  

    WindowSystem::Initialize(640, 480);
	RenderSystem::Initialize();

	// SceneFactoryテスト用
	// シーンの登録
	SceneFactory factory;
	factory.Register(SceneType::Test, [=] {
		return std::make_unique<TestScene>();
		});

	// シーンの生成
	auto scene = factory.Create(SceneType::Test);
	scene->Initialize();
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		scene->Update(0.016f);
		// ゲームループなど
		RenderSystem::BeginRender();
		scene->Draw();
		RenderSystem::EndRender();
	}

	RenderSystem::Finalize();
    WindowSystem::Finalize();
    return 0;
}