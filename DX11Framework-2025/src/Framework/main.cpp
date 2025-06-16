/**	@file	main.cpp
*	@brief 	�G���g���|�C���g
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
    HWND consoleWindow = GetConsoleWindow();    // �R���\�[���E�B���h�E�̃n���h�����擾
    ShowWindow(consoleWindow, SW_HIDE);         // �R���\�[���E�B���h�E���\���ɂ���
#endif//defined(DEBUG) || defined(_DEBUG)

    if (!WindowSystem::Initialize(500, 300)) {
        return -1;
    }

    // ���b�Z�[�W���[�v�J�n
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WindowSystem::Finalize();
    return static_cast<int>(msg.wParam);
}