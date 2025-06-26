/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"
#include "Framework/Utils/DebugHooks.h"

//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{    
	// Debug、Releseで適切なハンドラをセット
	DebugHooks::Install();  

    WindowSystem::Initialize(640, 480);
	D3D11System::Initialize();

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// ゲームループなど
	}

	D3D11System::Finalize();
    WindowSystem::Finalize();
    return 0;
}