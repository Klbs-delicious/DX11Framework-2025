/** @file   WindowSystem.cpp
*   @date   2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Core/WindowSystem.h"

#include<string>

//-----------------------------------------------------------------------------
// WindowSystem Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
*/
WindowSystem::WindowSystem()
{
    this->className = L"2025_FrameWork";    // ウィンドウクラス名
    this->windowTitle = L"2025_Framework";  // ウィンドウタイトル名
    this->width = 0;
    this->height = 0;
    this->hInstance = nullptr;
    this->hWnd = nullptr;
}

/** @brief デストラクタ
*/
WindowSystem::~WindowSystem()
{
}

/** @brief ウィンドウの初期化処理
 *  @param  const uint32_t ウィンドウの縦幅
 *  @param  const uint32_t ウィンドウの横幅
 */
bool WindowSystem::Initialize(const uint32_t _width, const uint32_t _height)
{
    // 画面のサイズを設定
    this->width = _width;
    this->height = _height;

    // インスタンスハンドルの取得
    this->hInstance = GetModuleHandle(nullptr);
    if (!this->hInstance) { return false; }

    // ウィンドウの設定
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowSystem::WndProc;
    wc.hIcon = LoadIcon(this->hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(this->hInstance, IDC_ARROW);
    wc.hbrBackground = GetSysColorBrush(static_cast<int>(BackColorBrush::GRAY));
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = this->className.c_str();
    wc.hIconSm = LoadIcon(this->hInstance, IDI_APPLICATION);

    // ウィンドウの登録
    if (!RegisterClassEx(&wc)) { return false; }

    // ウィンドウのサイズを設定
    RECT rc = {};
    rc.right = static_cast<LONG>(this->width);
    rc.bottom = static_cast<LONG>(this->height);

    // ウィンドウサイズを調節
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // ウィンドウを生成
    this->hWnd = CreateWindowEx(
        0,
        this->className.c_str(),
        this->windowTitle.c_str(),
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        this->hInstance,
        nullptr);
    if (!this->hWnd) { return false; }

    // ウィンドウを表示
    ShowWindow(this->hWnd, SW_SHOWNORMAL);

    // ウィンドウを更新
    UpdateWindow(this->hWnd);

    // ウィンドウにフォーカスを設定
    SetFocus(this->hWnd);

    return true;
}

/** @brief ウィンドウの終了処理
*/
void WindowSystem::Finalize()
{
    // 動的な文字列のメモリ解放（ヒープリーク検出の抑制）
    std::wstring emptyTitle, emptyClass;

    this->windowTitle.swap(emptyTitle);
    this->className.swap(emptyClass);

    // ウィンドウの登録を解除
    if (this->hInstance != nullptr)
    {
        UnregisterClass(this->className.c_str(), this->hInstance);
        this->hInstance = nullptr;
    }
    this->hWnd = nullptr;
}

/** @brief  ウィンドウタイトルの変更
*   @param  const std::wstring_view _windowTitle    ウィンドウのタイトル
*/
void WindowSystem::SetWindowTitle(const std::wstring_view _windowTitle)
{
    if (!this->hWnd) { return; }
    this->windowTitle.assign(_windowTitle);
    SetWindowTextW(this->hWnd, this->windowTitle.c_str());
}

/** @brief  ウィンドウサイズの設定
*   @param  const uint32_t ウィンドウの縦幅
*   @param  const uint32_t ウィンドウの横幅
*/
void WindowSystem::SetWindowSize(const uint32_t _width, const uint32_t _height)
{
    if (!this->hWnd || _width == 0 || _height == 0) return;

    this->width = _width;
    this->height = _height;

    // ウィンドウスタイルに合ったフレーム分を加味したウィンドウ全体のサイズを作成
    RECT rc = { 0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height) };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    // ウィンドウサイズの更新
    SetWindowPos(
        this->hWnd,
        nullptr,
        0, 0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOMOVE | SWP_NOZORDER
    );
}

/**@brief ウィンドウプロシージャ
 * @param   HWND    _hWnd   ウィンドウハンドル
 * @param   UINT    _msg    メッセージ
 * @param   WPARAM  _wp     パラメータ
 * @param   LPARAM  _lp     パラメータ
 * @return  LRESULT         処理結果
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