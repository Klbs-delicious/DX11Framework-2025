/**	@file	Application.h
*	@date	2025/09/10
*/
#pragma once
#include"Framework/Utils/NonCopyable.h"

#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"
#include"Framework/Core/RenderSystem.h"
#include"Framework/Core/GameLoop.h"

#include<cstdint>
#include <memory>

/**	@class	Application
 *	@brief	ゲームシステムの制御
 *	@details	
 *  - このクラスはコピー、代入を禁止している
 *  - システム全体の制御、及びライフサイクルの管理を行う(アプリケーションの初期化、終了処理、メインループ)
 */
class Application :private NonCopyable
{
public:
	/// @struct アプリケーション実行時の設定
	struct AppConfig
	{
		uint32_t screenWidth = 600;		///< 画面横サイズ
		uint32_t screenHeight = 300;	///< 画面縦サイズ
		bool isFullScreen = false;		///< フルスクリーンにするのか	[TODO] 使用するようにする
	};

	/** @brief  コンストラクタ
	 *	@param	const AppConfig _config	アプリケーション実行時の設定
	 */
	Application(const AppConfig _config);

	/// @brief	デストラクタ
	~Application();
	
	/// @brief 初期化処理
	static bool Initialize();

	/// @brief 実行処理
	static void Run();

	/// @brief	メインループ処理
	static void MainLoop();

	/// @brief	終了処理
	static void ShutDown();

private:
	static AppConfig appConfig;	///< 環境構築に必要な情報

	static std::unique_ptr<WindowSystem>   windowSystem;	///< ウィンドウを作成、更新する
	static std::unique_ptr<D3D11System>    d3d11System;		///< DirectX11のデバイス関連
	static std::unique_ptr<RenderSystem>   renderSystem;	///< 描画に必要な処理

	static std::unique_ptr<GameLoop>   gameLoop;			///< ゲーム進行の管理
};