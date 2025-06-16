/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"

//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#else
    HWND consoleWindow = GetConsoleWindow();    // コンソールウィンドウのハンドルを取得
    ShowWindow(consoleWindow, SW_HIDE);         // コンソールウィンドウを非表示にする
#endif//defined(DEBUG) || defined(_DEBUG)

    if (!WindowSystem::Initialize(500, 300)) {
        return -1;
    }

    // メッセージループ開始
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WindowSystem::Finalize();
    return static_cast<int>(msg.wParam);
}