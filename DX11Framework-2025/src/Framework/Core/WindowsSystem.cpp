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
const std::wstring	WindowSystem::className = L"2025_FrameWork";    // �E�B���h�E�N���X��
std::wstring        WindowSystem::windowTitle = L"2025_Framework";	// �E�B���h�E�^�C�g����(�f�o�b�O���V�[�����ȂǂŕύX���邽��const�ł͂Ȃ�)

uint32_t            WindowSystem::width;        // �E�B���h�E����
uint32_t            WindowSystem::height;       // �E�B���h�E�c��
HINSTANCE           WindowSystem::hInstance;    // �C���X�^���X�n���h��
HWND                WindowSystem::hWnd;         // �E�B���h�E�n���h��


//-----------------------------------------------------------------------------
// WindowSystem Class
//-----------------------------------------------------------------------------

/** @brief	�R���X�g���N�^
*/
WindowSystem::WindowSystem()
{
}

/** @brief �f�X�g���N�^
*/
WindowSystem::~WindowSystem()
{
}

/** @brief �E�B���h�E�̏���������
 *	@param	const uint32_t �E�B���h�E�̏c��
 *	@param	const uint32_t �E�B���h�E�̉���
 */
bool WindowSystem::Initialize(const uint32_t _width, const uint32_t _height)
{
    // ��ʂ̃T�C�Y��ݒ�
    WindowSystem::width = _width;
    WindowSystem::height = _height;

    // �C���X�^���X�n���h���̎擾
    auto hInst = GetModuleHandle(nullptr);
    if (!hInst){ return false; }

    // �E�B���h�E�̐ݒ�
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

    // �E�B���h�E�̓o�^
    if (!RegisterClassEx(&wc)){ return false; }

    // �C���X�^���X�n���h����ݒ�
    WindowSystem::hInstance = hInst;

    // �E�B���h�E�̃T�C�Y��ݒ�
    RECT rc = {};
    rc.right = static_cast<LONG>(WindowSystem::width);
    rc.bottom = static_cast<LONG>(WindowSystem::height);

    // �E�B���h�E�T�C�Y�𒲐�
    auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);

    // �E�B���h�E�𐶐�
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

    // �E�B���h�E��\��
    ShowWindow(WindowSystem::hWnd, SW_SHOWNORMAL);

    // �E�B���h�E���X�V
    UpdateWindow(WindowSystem::hWnd);

    // �E�B���h�E�Ƀt�H�[�J�X��ݒ�
    SetFocus(WindowSystem::hWnd);

    return true;
}

/** @brief �E�B���h�E�̏I������
*/
void WindowSystem::Finalize()
{
    // �E�B���h�E�̓o�^������
    if (WindowSystem::hInstance != nullptr)
    {
        UnregisterClass(WindowSystem::className.c_str(), WindowSystem::hInstance);
    }

    WindowSystem::hInstance = nullptr;
    WindowSystem::hWnd = nullptr;
}

/** @brief	�E�B���h�E�^�C�g���̕ύX
*	@param	const std::wstring_view _windowTitle	�E�B���h�E�̃^�C�g��
*/
void WindowSystem::SetWindowTitle(const std::wstring_view _windowTitle)
{
    if(!WindowSystem::hWnd){return;}
    WindowSystem::windowTitle.assign(_windowTitle); 
    SetWindowTextW(WindowSystem::hWnd, WindowSystem::windowTitle.c_str());
}

/**	@brief	�E�B���h�E�T�C�Y�̐ݒ�
*	@param	const uint32_t �E�B���h�E�̏c��
*	@param	const uint32_t �E�B���h�E�̉���
*/
void WindowSystem::SetWindowSize(const uint32_t _width, const uint32_t _height)
{
    if (!WindowSystem::hWnd || _width == 0 || _height == 0) return;

    WindowSystem::width = _width;
    WindowSystem::height = _height;

    // �E�B���h�E�X�^�C���ɍ������t���[���������������E�B���h�E�S�̂̃T�C�Y���쐬
    RECT rc = { 0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height) };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    // �E�B���h�E�T�C�Y�̍X�V
    SetWindowPos(
        WindowSystem::hWnd,
        nullptr,
        0, 0,
        rc.right - rc.left,
        rc.bottom - rc.top,
        SWP_NOMOVE | SWP_NOZORDER
    );
}

/**@brief �E�B���h�E�v���V�[�W��
 * @param	HWND	_hWnd	�E�B���h�E�n���h��
 * @param	UINT	_msg	���b�Z�[�W
 * @param	WPARAM	_wp		�p�����[�^
 * @param	LPARAM	_lp		�p�����[�^
 * @return	LRESULT			��������
 * @details �E�B���h�E�ɑ���ꂽ���b�Z�[�W����������
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