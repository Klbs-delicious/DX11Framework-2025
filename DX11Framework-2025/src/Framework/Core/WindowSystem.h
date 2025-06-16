/**	@file	WindowSystem.h
*	@date	2025/06/12
*/
#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <string_view>
#include"Framework/Utils/NonCopyable.h"

/**@class	WindowSystem
 * @brief	ウィンドウ作成の管理を行う
 * @details	このクラスはコピー、代入を禁止している
 */
class WindowSystem :private NonCopyable
{
public:
	/** @brief	コンストラクタ
	*/
	WindowSystem();

	/** @brief デストラクタ
	*/
	~WindowSystem();

	/** @brief ウィンドウの初期化処理
	*	@param	const uint32_t ウィンドウの縦幅
	*	@param	const uint32_t ウィンドウの横幅
	 */
	static bool Initialize(const uint32_t _width, const uint32_t _height);

	/** @brief ウィンドウの終了処理
	*/
	static void Finalize();

	/** @brief	ウィンドウタイトルの変更
	*	@param	const std::wstring_view _windowTitle	ウィンドウのタイトル
	*/
	static void SetWindowTitle(const std::wstring_view _windowTitle);

	/**	@brief	ウィンドウサイズの設定
	*	@param	const uint32_t ウィンドウの縦幅
	*	@param	const uint32_t ウィンドウの横幅
	*/
	static void SetWindowSize(const uint32_t _width, const uint32_t _height);

	/**	@brief	ウィンドウの横幅の取得
	*	@return	uint32_t ウィンドウの横幅
	*/
	inline static uint32_t GetWidth() { return WindowSystem::width; }

	/**	@brief	ウィンドウの縦幅の取得
	*	@return	uint32_t ウィンドウの縦幅
	*/
	inline static uint32_t GetHeight() { return WindowSystem::height; }

	/**	@brief	ウィンドウハンドルの取得
	*	@return	HWND ウィンドウハンドル
	*/
	inline static HWND GetWindow() { return WindowSystem::hWnd; }

	/**	@brief	インスタンスハンドルの取得
	*	@return	HINSTANCE インスタンスハンドル
	*/
	inline static HINSTANCE GetHInstance() { return WindowSystem::hInstance; }

	/**@brief ウィンドウプロシージャ
	 * @param	HWND	_hWnd	ウィンドウハンドル
	 * @param	UINT	_msg	メッセージ
	 * @param	WPARAM	_wp		パラメータ
	 * @param	LPARAM	_lp		パラメータ
	 * @return	LRESULT			処理結果
	 * @details ウィンドウに送られたメッセージを処理する
	 */
	static LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wp, LPARAM _lp);

private:
	//ウィンドウの初期背景色
	enum class BackColorBrush
	{
		WHITE = WHITE_BRUSH,	//0
		LTGRAY = LTGRAY_BRUSH,	//1
		GRAY = GRAY_BRUSH,		//2
		DKGRAY = DKGRAY_BRUSH,	//3
		BLACK = BLACK_BRUSH,	//4
		NOTHING = NULL_BRUSH,	//5
		HOLLOW = HOLLOW_BRUSH,	//=NULL_BRUSH
	};

	static const std::wstring	className;		// ウィンドウクラス名
	static std::wstring			windowTitle;	// ウィンドウタイトル名(デバッグ時シーン名などで変更するためconstではない)

	static uint32_t width;		// ウィンドウ横幅
	static uint32_t height;		// ウィンドウ縦幅

	static HINSTANCE	hInstance;	// インスタンスハンドル
	static HWND			hWnd;		// ウィンドウハンドル
};