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
#include "Scenes/SceneManager.h"
#include "Scenes/TestScene.h"
#include "Scenes/TitleScene.h"

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
	std::unique_ptr<SceneFactory >factory= std::make_unique<SceneFactory>();
	factory->Register(SceneType::Test, [=] {
		return std::make_unique<TestScene>();
		});
	factory->Register(SceneType::Title, [=] {
		return std::make_unique<TitleScene>();
		});

	// シーンマネージャーの作成
	std::unique_ptr< SceneManager> sceneManager = std::make_unique<SceneManager>(std::move(factory));
	// シーン遷移を行う
	sceneManager->RequestSceneChange(SceneType::Test);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// キー入力検出（例：スペースキーで切り替え）
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.wParam)
				{
				case VK_SPACE:
					// テスト → Test に遷移してみる（仮）
					sceneManager->RequestSceneChange(SceneType::Test);
					break;

				case VK_RETURN:
					// Enterキーでタイトルに戻す（仮）
					sceneManager->RequestSceneChange(SceneType::Title);
					break;
				}
			}
		}
	
		sceneManager->Update(0.016f);	// 更新
		RenderSystem::BeginRender();	// 描画開始
		sceneManager->Draw();			// 描画
		RenderSystem::EndRender();		// 描画終了
	}

	RenderSystem::Finalize();
    WindowSystem::Finalize();
    return 0;
}