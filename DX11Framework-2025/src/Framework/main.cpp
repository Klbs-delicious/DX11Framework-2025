/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"

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
    !WindowSystem::Initialize(500, 300);
    D3D11System::Initialize();

    D3D11System::Finalize();
    WindowSystem::Finalize();

    return 0;
}