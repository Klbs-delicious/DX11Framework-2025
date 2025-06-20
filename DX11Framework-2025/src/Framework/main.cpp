/**	@file	main.cpp
*	@brief 	�G���g���|�C���g
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
    HWND consoleWindow = GetConsoleWindow();    // �R���\�[���E�B���h�E�̃n���h�����擾
    ShowWindow(consoleWindow, SW_HIDE);         // �R���\�[���E�B���h�E���\���ɂ���
#endif//defined(DEBUG) || defined(_DEBUG)
    !WindowSystem::Initialize(500, 300);
    D3D11System::Initialize();

    D3D11System::Finalize();
    WindowSystem::Finalize();

    return 0;
}