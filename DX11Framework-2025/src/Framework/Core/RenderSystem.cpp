/** @file   RenderSystem.cpp
*   @date   2025/06/27
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/RenderSystem.h"
#include"Framework/Core/SystemLocator.h"
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"

#include <stdexcept>

//-----------------------------------------------------------------------------
// RenderSystem Class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
 *  @param  D3D11System*    _d3d11  DirectX11デバイス関連の参照
 *  @param  WindowSystem*   _window ウィンドウ作成等を行うクラスの参照
 */
RenderSystem::RenderSystem(D3D11System* _d3d11, WindowSystem* _window) :d3d11(_d3d11), window(_window) {}

/// @brief  デストラクタ
RenderSystem::~RenderSystem() {}

/** @brief  初期化処理
 *  @return bool 初期化に成功したかどうか
 */
bool RenderSystem::Initialize()
{
    HRESULT hr = S_OK;

    // デバイス、コンテキストの取得
    ID3D11Device* device = this->d3d11->GetDevice();
    ID3D11DeviceContext* context = this->d3d11->GetContext();

    // レンダーターゲットの作成
    ComPtr<ID3D11Texture2D> renderTarget;
    hr = this->d3d11->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(renderTarget.GetAddressOf()));
    if (SUCCEEDED(hr) && renderTarget)
    {
        device->CreateRenderTargetView(renderTarget.Get(), nullptr, this->renderTargetView.GetAddressOf());
    }
    else {
        throw std::runtime_error("Failed to retrieve render target buffer.");
        return false;
    }

    // SwapChainDescの取得
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    this->d3d11->GetSwapChain()->GetDesc(&swapChainDesc);

    // 深度ステンシルの作成
    ComPtr<ID3D11Texture2D> depthStencil;
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = swapChainDesc.BufferDesc.Width;
    textureDesc.Height = swapChainDesc.BufferDesc.Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D16_UNORM;
    textureDesc.SampleDesc = swapChainDesc.SampleDesc;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = device->CreateTexture2D(&textureDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depthStencil.");
        return false;
    }

    // 深度ステンシルビューの作成
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = textureDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, this->depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depthStencilView.");
        return false;
    }

    // レンダーターゲットの設定
    context->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());


    // シンプルなビューポートを作成
    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<FLOAT>(this->window->GetWidth());
    viewport.Height = static_cast<FLOAT>(this->window->GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context->RSSetViewports(1, &viewport);

    // ラスタライザステート設定
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;

    ComPtr<ID3D11RasterizerState> rs;
    device->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    context->RSSetState(rs.Get());

    // ブレンドステートの生成
    D3D11_BLEND_DESC BlendDesc{};
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = TRUE;

    // [0]ブレンドなし
    BlendDesc.RenderTarget[0].BlendEnable = FALSE;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&BlendDesc, this->blendState[0].GetAddressOf());

    // [1]通常のアルファブレンド(半透明描画)
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    device->CreateBlendState(&BlendDesc, this->blendState[1].GetAddressOf());

    // [ATC] AlphaToCoverage 対応用
    // 使用しようと思ったらスワップチェーンのSampleDesc.Countを変更する方が良い
    BlendDesc.AlphaToCoverageEnable = TRUE;
    device->CreateBlendState(&BlendDesc, this->blendStateATC.GetAddressOf());

    // [2]加算ブレンド(明るさを加算、光エフェクト等に使用)
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    device->CreateBlendState(&BlendDesc, this->blendState[2].GetAddressOf());

    // [3]減算ブレンド(差分、シャープなエッジ表現等に使用)
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    device->CreateBlendState(&BlendDesc, this->blendState[3].GetAddressOf());

    // アルファブレンドに設定
    this->SetBlendState(BlendStateType::BS_ALPHABLEND);

    // 深度ステンシルステートの設定
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    // 深度テストあり
    device->CreateDepthStencilState(&depthStencilDesc, this->depthStateEnable.GetAddressOf());

    // 深度テストなし
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&depthStencilDesc, this->depthStateDisable.GetAddressOf());

    // 深度テストありで設定
    context->OMSetDepthStencilState(this->depthStateEnable.Get(), 0);

    // サンプラーステートの設定
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    // テクスチャ座標が[0.0,1.0]を超えてしまった際の扱い
    // テクスチャを繰り返す設定
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    //  テクスチャの端の色をそのまま使う設定
    //    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    //    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    //    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxAnisotropy = 4;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> samplerState;
    device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());
    context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

    // 定数バッファの作成
    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(DirectX::SimpleMath::Matrix);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(float);

    // ワールド変換行列
    device->CreateBuffer(&bufferDesc, nullptr, this->worldBuffer.GetAddressOf());
    context->VSSetConstantBuffers(0, 1, this->worldBuffer.GetAddressOf());

    // ビュー変換行列
    device->CreateBuffer(&bufferDesc, nullptr, this->viewBuffer.GetAddressOf());
    context->VSSetConstantBuffers(1, 1, this->viewBuffer.GetAddressOf());

    // プロジェクション変換行列
    device->CreateBuffer(&bufferDesc, nullptr, this->projectionBuffer.GetAddressOf());
    context->VSSetConstantBuffers(2, 1, this->projectionBuffer.GetAddressOf());

    // 初期化成功
    return true;
}

/// @brief  描画関連の終了処理
void RenderSystem::Finalize()
{
    // ブレンドステート（他のリソースに依存しない）
    for (auto& bs : this->blendState) { bs.Reset(); }
    this->blendStateATC.Reset();

    // 深度ステンシルステート（depthStencilViewが生きてても問題なし）
    this->depthStateEnable.Reset();
    this->depthStateDisable.Reset();

    // 定数バッファ（描画ターゲットが参照する可能性がある）
    this->worldBuffer.Reset();
    this->viewBuffer.Reset();
    this->projectionBuffer.Reset();

    // 描画対象（最後）
    this->depthStencilView.Reset();
    this->renderTargetView.Reset();
}

/// @brief  描画開始時の処理
void RenderSystem::BeginRender()
{
    float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    auto context = this->d3d11->GetContext();
    context->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());
    context->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
    context->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

/// @brief  描画終了時の処理
void RenderSystem::EndRender()
{
    // バックバッファとフロントバッファを入れ替えて画面に表示
    HRESULT hr = this->d3d11->GetSwapChain()->Present(1, 0);
    if (FAILED(hr)) { OutputDebugString(L"Present failed!\n"); }
}

/** @brief  ワールド変換行列をGPUに送る
 *  @param  DirectX::SimpleMath::Matrix*    _worldMatrix    ワールド変換行列
 */
void RenderSystem::SetWorldMatrix(DirectX::SimpleMath::Matrix* _worldMatrix)
{
    DirectX::SimpleMath::Matrix mat = _worldMatrix->Transpose();
    this->d3d11->GetContext()->UpdateSubresource(this->worldBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/** @brief  プロジェクション変換行列をGPUに送る
 *  @param  DirectX::SimpleMath::Matrix*    _projectionMatrix   プロジェクション変換行列
 */
void RenderSystem::SetProjectionMatrix(DirectX::SimpleMath::Matrix* _projectionMatrix)
{
    DirectX::SimpleMath::Matrix mat = _projectionMatrix->Transpose();
    this->d3d11->GetContext()->UpdateSubresource(this->projectionBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/** @brief  ビュー変換行列をGPUに送る
 *  @param  DirectX::SimpleMath::Matrix*    _viewMatrix ビュー変換行列
 */
void RenderSystem::SetViewMatrix(DirectX::SimpleMath::Matrix* _viewMatrix)
{
    DirectX::SimpleMath::Matrix mat = _viewMatrix->Transpose();
    this->d3d11->GetContext()->UpdateSubresource(this->viewBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/** @brief 指定したブレンドステートを設定
 *  @param BlendStateType _blendState 使用するブレンドステートの種類
 */
void RenderSystem::SetBlendState(BlendStateType _blendState)
{
    if (static_cast<int>(_blendState) >= 0 && static_cast<int>(_blendState) < static_cast<int>(BlendStateType::MAX_BLENDSTATE))
    {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        this->d3d11->GetContext()->OMSetBlendState(this->blendState[static_cast<int>(_blendState)].Get(), blendFactor, 0xffffffff);
    }
}

/** @brief Alpha To Coverage（マルチサンプリング対応の透明処理）用のON/OFFを切り替える
 *  @param bool _enable true：ATC有効  false：無効
 *  @details    マルチサンプリング＋アルファブレンドの高度な合成を行う
 */
void RenderSystem::SetATCEnable(bool _enable)
{
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    this->d3d11->GetContext()->OMSetBlendState(
        _enable ? this->blendStateATC.Get() : this->blendState[0].Get(),
        blendFactor, 0xffffffff);
}

/** @brief 面の除外（カリング）を無効、有効にする
 *  @param bool _cullflag = false true：カリングON（通常）　false：カリングOFF（両面描画）
 */
void RenderSystem::DisableCulling(bool _cullflag)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = _cullflag ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    ComPtr<ID3D11RasterizerState> pRasterizerState;
    HRESULT hr = this->d3d11->GetDevice()->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
    if (FAILED(hr))
        return;

    this->d3d11->GetContext()->RSSetState(pRasterizerState.Get());
}

/** @brief ラスタライザステートのフィルモード（塗りつぶし、ワイヤーフレーム）を設定する
 *  @param D3D11_FILL_MODE _fillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
 */
void RenderSystem::SetFillMode(D3D11_FILL_MODE _fillMode)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = _fillMode;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;

    ComPtr<ID3D11RasterizerState> rs;
    this->d3d11->GetDevice()->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    this->d3d11->GetContext()->RSSetState(rs.Get());
}

/** @brief 深度テストを常にパスさせる設定に変更する
 *  @details
 * - 深度テストは有効（DepthEnable = TRUE）
 * - ただし、常に「描画OK」（DepthFunc = D3D11_COMPARISON_ALWAYS）
 * - 深度バッファにも書き込む（DepthWriteMask = ALL）
 */
void RenderSystem::SetDepthAllwaysWrite()
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;   // 常に深度テスト成功
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.StencilEnable = FALSE;                 // ステンシルテストは無効

    ComPtr<ID3D11DepthStencilState> pDepthStencilState;
    HRESULT hr = this->d3d11->GetDevice()->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        this->d3d11->GetContext()->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
    }
}

/////**   @brief  ビューポートを追加
//// *    @param  const D3D11_VIEWPORT& _viewport 追加するビューポート
//// */
//void RenderSystem::AddViewport(const D3D11_VIEWPORT& _viewport)
//{
//    this->viewportList.push_back(_viewport);
//}
//
/////**   @brief  指定のビューポートを削除
//// *    @param  const int _viewportType ビューポートの番号
//// */
//void RenderSystem::RemoveViewport(const int _viewportType)
//{
//    if (_viewportType >= 0 && _viewportType < static_cast<int>(this->viewportList.size()))
//    {
//        this->viewportList.erase(this->viewportList.begin() + _viewportType);
//    }
//}
//
/////**   @brief  指定のビューポートを
//// *    @param  const int _viewportType ビューポートの番号
//// */
//void RenderSystem::RemoveViewport(const int _viewportType)
//{
//    if (_viewportType >= 0 && _viewportType < static_cast<int>(this->viewportList.size()))
//    {
//        this->viewportList.erase(this->viewportList.begin() + _viewportType);
//    }
//}
