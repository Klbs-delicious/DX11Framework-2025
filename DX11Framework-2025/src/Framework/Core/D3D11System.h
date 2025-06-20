/**	@file	D3D11System.h
*	@date	2025/06/16
*/
#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include"Framework/Utils/NonCopyable.h"

// �����N���ׂ��O�����C�u����
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

using Microsoft::WRL::ComPtr;

/**@class	D3D11System
 * @brief	D3D11�̏������A��n�����s���N���X
 * @details	���̃N���X�̓R�s�[�A������֎~���Ă���
 */
class D3D11System :private NonCopyable
{
public:
	/** @brief	�R���X�g���N�^
	*/
	D3D11System();

	/** @brief	�f�X�g���N�^
	*/
	~D3D11System();

	/** @brief	DX11�̏�����
	*/
	static void Initialize();

	/** @brief	DX11�̏I������
	*/
	static void Finalize();

	/** @brief	�f�o�C�X�̎擾
	*	@return	ID3D11Device*
	*/
	inline static ID3D11Device* GetDevice() { return D3D11System::device.Get(); }

	/** @brief	�f�o�C�X�R���e�L�X�g�̎擾
	*	@return	ID3D11DeviceContext*
	*/
	inline static ID3D11DeviceContext* GetContext() { return D3D11System::deviceContext.Get(); }

private:
	static D3D_FEATURE_LEVEL			featureLevel;		// DX11�̋@�\���x��
	static ComPtr<ID3D11Device>			device;				// �f�o�C�X
	static ComPtr<ID3D11DeviceContext>	deviceContext;		// �`��R�}���h���o��
	static ComPtr<IDXGISwapChain>		swapChain;			// �t���[���o�b�t�@�̊Ǘ�
	static ComPtr<IDXGIFactory>			factory;			// �A�_�v�^(GPU)���
};
/*
	static ComPtr<ID3D11RenderTargetView>	targetView;			// �`��^�[�Q�b�g
	static ComPtr<ID3D11DepthStencilView>	depthStencilView;	// �[�x�A�X�e���V���p�̃o�b�t�@
*/