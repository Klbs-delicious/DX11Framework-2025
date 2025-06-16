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
 * @brief	�E�B���h�E�쐬�̊Ǘ����s��
 * @details	���̃N���X�̓R�s�[�A������֎~���Ă���
 */
class WindowSystem :private NonCopyable
{
public:
	/** @brief	�R���X�g���N�^
	*/
	WindowSystem();

	/** @brief �f�X�g���N�^
	*/
	~WindowSystem();

	/** @brief �E�B���h�E�̏���������
	*	@param	const uint32_t �E�B���h�E�̏c��
	*	@param	const uint32_t �E�B���h�E�̉���
	 */
	static bool Initialize(const uint32_t _width, const uint32_t _height);

	/** @brief �E�B���h�E�̏I������
	*/
	static void Finalize();

	/** @brief	�E�B���h�E�^�C�g���̕ύX
	*	@param	const std::wstring_view _windowTitle	�E�B���h�E�̃^�C�g��
	*/
	static void SetWindowTitle(const std::wstring_view _windowTitle);

	/**	@brief	�E�B���h�E�T�C�Y�̐ݒ�
	*	@param	const uint32_t �E�B���h�E�̏c��
	*	@param	const uint32_t �E�B���h�E�̉���
	*/
	static void SetWindowSize(const uint32_t _width, const uint32_t _height);

	/**	@brief	�E�B���h�E�̉����̎擾
	*	@return	uint32_t �E�B���h�E�̉���
	*/
	inline static uint32_t GetWidth() { return WindowSystem::width; }

	/**	@brief	�E�B���h�E�̏c���̎擾
	*	@return	uint32_t �E�B���h�E�̏c��
	*/
	inline static uint32_t GetHeight() { return WindowSystem::height; }

	/**	@brief	�E�B���h�E�n���h���̎擾
	*	@return	HWND �E�B���h�E�n���h��
	*/
	inline static HWND GetWindow() { return WindowSystem::hWnd; }

	/**	@brief	�C���X�^���X�n���h���̎擾
	*	@return	HINSTANCE �C���X�^���X�n���h��
	*/
	inline static HINSTANCE GetHInstance() { return WindowSystem::hInstance; }

	/**@brief �E�B���h�E�v���V�[�W��
	 * @param	HWND	_hWnd	�E�B���h�E�n���h��
	 * @param	UINT	_msg	���b�Z�[�W
	 * @param	WPARAM	_wp		�p�����[�^
	 * @param	LPARAM	_lp		�p�����[�^
	 * @return	LRESULT			��������
	 * @details �E�B���h�E�ɑ���ꂽ���b�Z�[�W����������
	 */
	static LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wp, LPARAM _lp);

private:
	//�E�B���h�E�̏����w�i�F
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

	static const std::wstring	className;		// �E�B���h�E�N���X��
	static std::wstring			windowTitle;	// �E�B���h�E�^�C�g����(�f�o�b�O���V�[�����ȂǂŕύX���邽��const�ł͂Ȃ�)

	static uint32_t width;		// �E�B���h�E����
	static uint32_t height;		// �E�B���h�E�c��

	static HINSTANCE	hInstance;	// �C���X�^���X�n���h��
	static HWND			hWnd;		// �E�B���h�E�n���h��
};