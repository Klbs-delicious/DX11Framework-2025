/** @file   D3D11System.cpp
*   @date   2025/06/18
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Core/D3D11System.h"

#include <dxgi1_2.h>    // CreateSwapChainForHwnd 用
#include <stdlib.h>     // _countof を使うために必要

//-----------------------------------------------------------------------------
// D3D11System Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
*   @param  WindowSystem* _window   ウィンドウ作成等を行うクラスの参照
*/
D3D11System::D3D11System(WindowSystem* _window) :window(_window) {}

/// @brief デストラクタ
D3D11System::~D3D11System() {}

/** @brief DX11の初期化
*   @return bool 初期化に成功したかどうかを返す
*/
bool D3D11System::Initialize()
{
    HRESULT hr = S_OK;

    // デバイスの設定
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    // デバッグレイヤーを追加
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    // デバイスの作成
    hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels, _countof(featureLevels),
        D3D11_SDK_VERSION,
        this->device.GetAddressOf(),
        &this->featureLevel,
        this->deviceContext.GetAddressOf()
    );

    if (FAILED(hr)) {
        MessageBox(nullptr, L"DirectX 11 デバイス作成に失敗", L"エラー", MB_OK);
        return false;
    }

    // スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.Width = this->window->GetWidth();
    scDesc.Height = this->window->GetHeight();
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.SampleDesc.Count = 1;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = 2;
    scDesc.Scaling = DXGI_SCALING_STRETCH;
    scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // DXGIFactory2 経由でスワップチェーンを作成
    ComPtr<IDXGIDevice> dxgiDevice;
    this->device.As(&dxgiDevice);

    // GPU情報の取得
    ComPtr<IDXGIAdapter> adapter;
    dxgiDevice->GetAdapter(&adapter);

    // 使用中GPU名の確認
    // デバッグ用に残している
    /*
    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);
    MessageBox(nullptr, desc.Description, L"使用中のGPU", MB_OK);
    */

    // DXGIFactory2の取得
    ComPtr<IDXGIFactory2> dxgiFactory2;
    hr = adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));
    if (FAILED(hr)) {
        MessageBox(nullptr, L"IDXGIFactory2 の取得に失敗しました", L"エラー", MB_OK);
        return false;
    }

    // スワップチェーンの作成
    ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory2->CreateSwapChainForHwnd(
        this->device.Get(),
        this->window->GetWindow(),
        &scDesc,
        nullptr, nullptr,
        swapChain1.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBox(nullptr, L"スワップチェーン作成失敗", L"エラー", MB_OK);
        return false;
    }

    // swapChain1 を D3D11System::swapChain に変換
    swapChain1.As(&this->swapChain);

    // dxgiFactory2 を IDXGIFactory にキャストして保持
    dxgiFactory2.As(&this->factory);

    return true;
}

/// @brief DX11の終了処理
void D3D11System::Finalize()
{
    // 描画ターゲット解除 + 状態クリア
    if (this->deviceContext)
    {
        this->deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        ID3D11Buffer* nullBuffer = nullptr;
        this->deviceContext->VSSetConstantBuffers(0, 1, &nullBuffer); // ← バインド解除
        this->deviceContext->ClearState();
        this->deviceContext->Flush();
    }

    // フルスクリーン解除
    if (this->swapChain)
    {
        this->swapChain->SetFullscreenState(FALSE, nullptr);
    }

    this->factory.Reset();
    this->swapChain.Reset();
    this->deviceContext.Reset();
    this->device.Reset();
}