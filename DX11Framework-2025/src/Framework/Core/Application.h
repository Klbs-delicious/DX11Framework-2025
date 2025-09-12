/**	@file	Application.h
*	@date	2025/09/10
*/
#pragma once
#include"Framework/Utils/NonCopyable.h"

#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"
#include"Framework/Core/RenderSystem.h"

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
		bool isFullScreen = false;		///< フルスクリーンにするのか
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
	static AppConfig appConfig;

	static std::unique_ptr<WindowSystem>   windowSystem;
	static std::unique_ptr<D3D11System>    d3d11System;
	static std::unique_ptr<RenderSystem>   renderSystem;
};