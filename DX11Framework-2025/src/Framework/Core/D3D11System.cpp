/**	@file	D3D11System.cpp
*	@date	2025/06/18
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/D3D11System.h"
#include"Framework/Core/WindowSystem.h"

#include <stdlib.h>  // _countof ���g�����߂ɕK�v
//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------

// DX11�̋@�\���x��
D3D_FEATURE_LEVEL			D3D11System::featureLevel = D3D_FEATURE_LEVEL_11_0;		

ComPtr<ID3D11Device>        D3D11System::device;            // �f�o�C�X
ComPtr<ID3D11DeviceContext>	D3D11System::deviceContext;		// �`��R�}���h���o��
ComPtr<IDXGISwapChain>		D3D11System::swapChain;			// �t���[���o�b�t�@�̊Ǘ�
ComPtr<IDXGIFactory>        D3D11System::factory;			// �A�_�v�^(GPU)���

//-----------------------------------------------------------------------------
// D3D11System Class
//-----------------------------------------------------------------------------

/** @brief	�R���X�g���N�^
*/
D3D11System::D3D11System() {}

/** @brief	�f�X�g���N�^
*/
D3D11System::~D3D11System() {}

#include <dxgi1_2.h>  // CreateSwapChainForHwnd �p

/** @brief	DX11�̏�����
*/
void D3D11System::Initialize()
{
    HRESULT hr = S_OK;

    // �f�o�C�X�̐ݒ�
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    // �f�o�C�X�̍쐬
    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION,
        D3D11System::device.GetAddressOf(),
        &D3D11System::featureLevel,
        D3D11System::deviceContext.GetAddressOf()
    );

    if (FAILED(hr)) {
        MessageBox(nullptr, L"DirectX 11 �f�o�C�X�쐬�Ɏ��s", L"�G���[", MB_OK);
        return;
    }

    // �X���b�v�`�F�[���̐ݒ�
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.Width = WindowSystem::GetWidth();
    scDesc.Height = WindowSystem::GetHeight();
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.SampleDesc.Count = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 2;
    scDesc.Scaling = DXGI_SCALING_STRETCH;
    scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    //scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    //scDesc.Flags = 0;

    // DXGIFactory2 �o�R�ŃX���b�v�`�F�[�����쐬
    ComPtr<IDXGIDevice> dxgiDevice;
    D3D11System::device.As(&dxgiDevice);

    // GPU���̎擾
    ComPtr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(&adapter);

    // �g�p��GPU���̊m�F
    // �f�o�b�O�p�Ɏc���Ă���
    /*
    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);
    MessageBox(nullptr, desc.Description, L"�g�p����GPU", MB_OK);
    */

    // DXGIFactory2�̎擾
    ComPtr<IDXGIFactory2> dxgiFactory2;
    adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));

    // �X���b�v�`�F�[���̍쐬
    ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory2->CreateSwapChainForHwnd(
        D3D11System::device.Get(),
        WindowSystem::GetWindow(),
        &scDesc,
        nullptr, nullptr,
        swapChain1.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBox(nullptr, L"�X���b�v�`�F�[���쐬���s", L"�G���[", MB_OK);
        return;
    }

    // swapChain1 �� D3D11System::swapChain �ɕϊ�
    swapChain1.As(&D3D11System::swapChain);

    // dxgiFactory2 �� IDXGIFactory �ɃL���X�g���ĕێ�
    dxgiFactory2.As(&D3D11System::factory);
}

/** @brief	DX11�̏I������
*/
void D3D11System::Finalize()
{
    if (D3D11System::swapChain)
    {
        D3D11System::swapChain->SetFullscreenState(FALSE, nullptr); // �ꉞ�t���X�N���[������E�B���h�E���[�h�ɖ߂�
        D3D11System::swapChain.Reset();                             // ���\�[�X�̉��
    }
    if (D3D11System::deviceContext)
    {
        D3D11System::deviceContext->OMSetRenderTargets(0, nullptr, nullptr);    // �����I�Ƀo�C���h������
        D3D11System::deviceContext->ClearState();                               // ���݃o�C���h���̃X�e�[�g������
        D3D11System::deviceContext->Flush();                                    // �ۗ����̃R�}���h���������s
        D3D11System::deviceContext.Reset();                                     // ���\�[�X�̉��
    }
    D3D11System::device.Reset();
    D3D11System::factory.Reset();  
}