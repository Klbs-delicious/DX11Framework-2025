/**	@file	D3D11System.cpp
*	@date	2025/06/18
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/D3D11System.h"
#include"Framework/Core/WindowSystem.h"

#include <dxgi1_2.h>    // CreateSwapChainForHwnd 用
#include <stdlib.h>     // _countof を使うために必要
//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------

// DX11の機能レベル
D3D_FEATURE_LEVEL			D3D11System::featureLevel = D3D_FEATURE_LEVEL_11_0;		

ComPtr<ID3D11Device>        D3D11System::device;            // デバイス
ComPtr<ID3D11DeviceContext>	D3D11System::deviceContext;		// 描画コマンドを出す
ComPtr<IDXGISwapChain>		D3D11System::swapChain;			// フレームバッファの管理
ComPtr<IDXGIFactory>        D3D11System::factory;			// アダプタ(GPU)情報

//-----------------------------------------------------------------------------
// D3D11System Class
//-----------------------------------------------------------------------------

/** @brief	コンストラクタ
*/
D3D11System::D3D11System() {}

/** @brief	デストラクタ
*/
D3D11System::~D3D11System() {}

/** @brief	DX11の初期化
*/
void D3D11System::Initialize()
{
    HRESULT hr = S_OK;

    // デバイスの設定
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
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
        D3D11System::device.GetAddressOf(),
        &D3D11System::featureLevel,
        D3D11System::deviceContext.GetAddressOf()
    );

    if (FAILED(hr)) {
        MessageBox(nullptr, L"DirectX 11 デバイス作成に失敗", L"エラー", MB_OK);
        return;
    }

    // スワップチェーンの設定
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.Width = WindowSystem::GetWidth();
    scDesc.Height = WindowSystem::GetHeight();
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
    D3D11System::device.As(&dxgiDevice);

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
        return;
    }

    // スワップチェーンの作成
    ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory2->CreateSwapChainForHwnd(
        D3D11System::device.Get(),
        WindowSystem::GetWindow(),
        &scDesc,
        nullptr, nullptr,
        swapChain1.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBox(nullptr, L"スワップチェーン作成失敗", L"エラー", MB_OK);
        return;
    }

    // swapChain1 を D3D11System::swapChain に変換
    swapChain1.As(&D3D11System::swapChain);

    // dxgiFactory2 を IDXGIFactory にキャストして保持
    dxgiFactory2.As(&D3D11System::factory);
}

/** @brief	DX11の終了処理
*/
void D3D11System::Finalize()
{
    // 1. レンダーターゲットの解除とフラッシュを行う
    if (D3D11System::deviceContext)
    {
        D3D11System::deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        D3D11System::deviceContext->ClearState();
        D3D11System::deviceContext->Flush();
    }

    // 2. フルスクリーンモードを解除してからスワップチェーンを解放
    if (D3D11System::swapChain)
    {
        HRESULT hr = D3D11System::swapChain->SetFullscreenState(FALSE, nullptr);
        if (FAILED(hr)) {
            MessageBox(nullptr, L"フルスクリーン状態の解除に失敗しました", L"エラー", MB_OK);
        }
    }

    // 3. 解放は依存順：factory → swapChain → context → device
    D3D11System::factory.Reset();
    D3D11System::swapChain.Reset();
    D3D11System::deviceContext.Reset();
    D3D11System::device.Reset();
}