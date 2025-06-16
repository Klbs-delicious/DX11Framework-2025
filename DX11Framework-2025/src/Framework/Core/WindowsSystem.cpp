/**	@file	WindowSystem.cpp
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include<string>
#include"Framework/Core/WindowSystem.h"

//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------
const std::wstring	WindowSystem::className = L"2025_FrameWork";    // ウィンドウクラス名
std::wstring        WindowSystem::windowTitle = L"2025_Framework";	// ウィンドウタイトル名(デバッグ時シーン名などで変更するためconstではない)

uint32_t            WindowSystem::width;        // ウィンドウ横幅
uint32_t            WindowSystem::height;       // ウィンドウ縦幅
HINSTANCE           WindowSystem::hInstance;    // インスタンスハンドル
HWND                WindowSystem::hWnd;         // ウィンドウハンドル


//-----------------------------------------------------------------------------
// WindowSystem Class
//-----------------------------------------------------------------------------

/** @brief	コンストラクタ
*/
WindowSystem::WindowSystem()
{
}

/** @brief デストラクタ
*/
WindowSystem::~WindowSystem()
{
}

/** @brief ウィンドウの初期化処理
 *	@param	const uint32_t ウィンドウの縦幅
 *	@param	const uint32_t ウィンドウの横幅
 */
bool WindowSystem::Initialize(const uint32_t _width, const uint32_t _height)
{
    // 画面のサイズを設定
    WindowSystem::width = _width;
    WindowSystem::height = _height;

    // インスタンスハンドルの取得
    auto hInst = GetModuleHandle(nullptr);
    if (!hInst){ return false; }

    // ウィンドウの設定
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowSystem::WndProc;
    wc.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wc.hCursor = LoadCursor(hInst, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(static_cast<int>(BackColorBrush::GRAY));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = WindowSystem::className.c_str();
    wc.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

    // ウィンドウの登録
    if (!RegisterClassEx(&wc)){ return false; }

    // インスタンスハンドルを設定
    WindowSystem::hInstance = hInst;

    // ウィンドウのサイズを設定
    RECT rc = {};
    rc.right = static_cast<LONG>(WindowSystem::width);
    rc.bottom = static_cast<LONG>(WindowSystem::height);

    // ウィンドウサイズを調節
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウを生成
    WindowSystem::hWnd = CreateWindowEx(
        0,
        WindowSystem::className.c_str(),
        WindowSystem::windowTitle.c_str(),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        WindowSystem::hInstance,
        nullptr);
    if (!WindowSystem::hWnd){ return false; }

    // ウィンドウを表示
    ShowWindow(WindowSystem::hWnd, SW_SHOWNORMAL);

    // ウィンドウを更新
    UpdateWindow(WindowSystem::hWnd);

    // ウィンドウにフォーカスを設定
    SetFocus(WindowSystem::hWnd);

    return true;
}

/** @brief ウィンドウの終了処理
*/
void WindowSystem::Finalize()
{
    // ウィンドウの登録を解除
    if (WindowSystem::hInstance != nullptr)
    {
        UnregisterClass(WindowSystem::className.c_str(), WindowSystem::hInstance);
    }

    WindowSystem::hInstance = nullptr;
    WindowSystem::hWnd = nullptr;
}

/** @brief	ウィンドウタイトルの変更
*	@param	const std::wstring_view _windowTitle	ウィンドウのタイトル
*/
void WindowSystem::SetWindowTitle(const std::wstring_view _windowTitle)
{
    if(!WindowSystem::hWnd){return;}
    WindowSystem::windowTitle.assign(_windowTitle); 
    SetWindowTextW(WindowSystem::hWnd, WindowSystem::windowTitle.c_str());
}

/**	@brief	ウィンドウサイズの設定
*	@param	const uint32_t ウィンドウの縦幅
*	@param	const uint32_t ウィンドウの横幅
*/
void WindowSystem::SetWindowSize(const uint32_t _width, const uint32_t _height)
{
    if (!WindowSystem::hWnd || _width == 0 || _height == 0) return;

    WindowSystem::width = _width;
    WindowSystem::height = _height;

    // ウィンドウスタイルに合ったフレーム分を加味したウィンドウ全体のサイズを作成
    RECT rc = { 0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height) };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    // ウィンドウサイズの更新
    SetWindowPos(
        WindowSystem::hWnd,
        nullptr,
        0, 0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOMOVE | SWP_NOZORDER
    );
}

/**@brief ウィンドウプロシージャ
 * @param	HWND	_hWnd	ウィンドウハンドル
 * @param	UINT	_msg	メッセージ
 * @param	WPARAM	_wp		パラメータ
 * @param	LPARAM	_lp		パラメータ
 * @return	LRESULT			処理結果
 * @details ウィンドウに送られたメッセージを処理する
 */
LRESULT CALLBACK WindowSystem::WndProc(HWND _hWnd, UINT _msg, WPARAM _wp, LPARAM _lp)
{
    switch (_msg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;

    default:
    { /* DO_NOTHING */ }
    break;
    }

    return DefWindowProc(_hWnd, _msg, _wp, _lp);
}