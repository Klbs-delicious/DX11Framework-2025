/**	@file	RenderSystem.cpp
*	@date	2025/06/27
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/RenderSystem.h"
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/D3D11System.h"

#include <stdexcept>

//-----------------------------------------------------------------------------
// Class Static
//-----------------------------------------------------------------------------

//std::vector<D3D11_VIEWPORT>		RenderSystem::viewportList;		///< ビューポートのリスト
ComPtr<ID3D11RenderTargetView>	RenderSystem::renderTargetView;	///< 描画ターゲット
ComPtr<ID3D11DepthStencilView>	RenderSystem::depthStencilView;	///< 深度、ステンシル用のバッファ
ComPtr<ID3D11Buffer>			RenderSystem::worldBuffer;		///< ワールド変換行列を保持するバッファ
ComPtr<ID3D11Buffer>			RenderSystem::projectionBuffer;	///< プロジェクション変換行列を保持するバッファ
ComPtr<ID3D11Buffer>			RenderSystem::viewBuffer;		///< ビュー変換行列を保持するバッファ

ComPtr<ID3D11DepthStencilState> RenderSystem::depthStateEnable;	    ///< 深度テストを有効にした状態
ComPtr<ID3D11DepthStencilState> RenderSystem::depthStateDisable;    ///< 深度テストを無効にした状態

ComPtr<ID3D11BlendState> RenderSystem::blendState[static_cast<int>(BlendStateType::MAX_BLENDSTATE)];	///< 各種ブレンドステートを保持する配列
ComPtr<ID3D11BlendState> RenderSystem::blendStateATC;													///< Alpha To Coverage（マルチサンプリング対応の透明処理）用の専用ブレンドステート

//-----------------------------------------------------------------------------
// RenderSystem Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
RenderSystem::RenderSystem() {}

/// @brief	デストラクタ
RenderSystem::~RenderSystem()
{
    // D3D11Systemの終了処理
    D3D11System::Finalize();
}

/// @brief	初期化処理
void RenderSystem::Initialize()
{
    // D3D11Systemの初期化
    D3D11System::Initialize();

    HRESULT hr = S_OK;

    // デバイス、コンテキストの取得
    ID3D11Device* device = D3D11System::GetDevice();
    ID3D11DeviceContext* context = D3D11System::GetContext();

    // レンダーターゲットの作成
    ComPtr<ID3D11Texture2D> renderTarget;
    hr = D3D11System::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(renderTarget.GetAddressOf()));
    if (SUCCEEDED(hr) && renderTarget)
    {
        device->CreateRenderTargetView(renderTarget.Get(), nullptr, RenderSystem::renderTargetView.GetAddressOf());
    }
    else { throw std::runtime_error("Failed to retrieve render target buffer."); }

    // SwapChainDescの取得
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    D3D11System::GetSwapChain()->GetDesc(&swapChainDesc);

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
    if (FAILED(hr)) { throw std::runtime_error("Failed to create depthStencil."); }

    // 深度ステンシルビューの作成
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = textureDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    hr = device->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, RenderSystem::depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create depthStencilView.");
    }

    // レンダーターゲットの設定
   context->OMSetRenderTargets(1, RenderSystem::renderTargetView.GetAddressOf(), RenderSystem::depthStencilView.Get());

    // ビューポートの作成
    // シンプルなビューポートを作成
    D3D11_VIEWPORT viewport;
    viewport.Width = static_cast<FLOAT>(WindowSystem::GetWidth());
    viewport.Height = static_cast<FLOAT>(WindowSystem::GetHeight());
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
    device->CreateBlendState(&BlendDesc, RenderSystem::blendState[0].GetAddressOf());

    // [1]通常のアルファブレンド(半透明描画)
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    device->CreateBlendState(&BlendDesc, RenderSystem::blendState[1].GetAddressOf());

    // [ATC] AlphaToCoverage 対応用
    // 使用しようと思ったらスワップチェーンのSampleDesc.Countを変更する方が良い
    BlendDesc.AlphaToCoverageEnable = TRUE;
    device->CreateBlendState(&BlendDesc, RenderSystem::blendStateATC.GetAddressOf());

    // [2]加算ブレンド(明るさを加算、光エフェクト等に使用)
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    device->CreateBlendState(&BlendDesc, RenderSystem::blendState[2].GetAddressOf());

    // [3]減算ブレンド(差分、シャープなエッジ表現等に使用)
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    device->CreateBlendState(&BlendDesc, RenderSystem::blendState[3].GetAddressOf());

    // アルファブレンドに設定
    RenderSystem::SetBlendState(BlendStateType::BS_ALPHABLEND);

    // 深度ステンシルステートの設定
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.StencilEnable = FALSE;

    // 深度テストあり
    device->CreateDepthStencilState(&depthStencilDesc, RenderSystem::depthStateEnable.GetAddressOf());

    // 深度テストなし
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    device->CreateDepthStencilState(&depthStencilDesc, RenderSystem::depthStateDisable.GetAddressOf());

    // 深度テストありで設定
    context->OMSetDepthStencilState(RenderSystem::depthStateEnable.Get(), 0);

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
    device->CreateBuffer(&bufferDesc, nullptr, RenderSystem::worldBuffer.GetAddressOf());
    context->VSSetConstantBuffers(0, 1, RenderSystem::worldBuffer.GetAddressOf());

    // ビュー変換行列
    device->CreateBuffer(&bufferDesc, nullptr, RenderSystem::viewBuffer.GetAddressOf());
    context->VSSetConstantBuffers(1, 1, RenderSystem::viewBuffer.GetAddressOf());

    // プロジェクション変換行列
    device->CreateBuffer(&bufferDesc, nullptr, RenderSystem::projectionBuffer.GetAddressOf());
    context->VSSetConstantBuffers(2, 1, RenderSystem::projectionBuffer.GetAddressOf());
}

/// @brief	描画関連の終了処理
void RenderSystem::Finalize()
{
    // ブレンドステート（他のリソースに依存しない）
    for (auto& bs : RenderSystem::blendState) { bs.Reset(); }
    RenderSystem::blendStateATC.Reset();

    // 深度ステンシルステート（depthStencilViewが生きてても問題なし）
    RenderSystem::depthStateEnable.Reset();
    RenderSystem::depthStateDisable.Reset();

    // 定数バッファ（描画ターゲットが参照する可能性がある）
    RenderSystem::worldBuffer.Reset();
    RenderSystem::viewBuffer.Reset();
    RenderSystem::projectionBuffer.Reset();

    // 描画対象（最後）
    RenderSystem::depthStencilView.Reset();
    RenderSystem::renderTargetView.Reset();
}

/// @brief	描画開始時の処理
void RenderSystem::BeginRender()
{
    float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    D3D11System::GetContext()->ClearRenderTargetView(RenderSystem::renderTargetView.Get(), clearColor);
    D3D11System::GetContext()->ClearDepthStencilView(RenderSystem::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

/// @brief	描画終了時の処理
void RenderSystem::EndRender()
{
    // バックバッファとフロントバッファを入れ替えて画面に表示
    D3D11System::GetSwapChain()->Present(1, 0);
}

/**	@brief	ワールド変換行列をGPUに送る
 *	@param	DirectX::SimpleMath::Matrix*	_worldMatrix	ワールド変換行列
 */
void RenderSystem::SetWorldMatrix(DirectX::SimpleMath::Matrix*	_worldMatrix)
{
    DirectX::SimpleMath::Matrix mat = _worldMatrix->Transpose();
    D3D11System::GetContext()->UpdateSubresource(RenderSystem::worldBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**	@brief	プロジェクション変換行列をGPUに送る
 *	@param	DirectX::SimpleMath::Matrix*	_projectionMatrix	プロジェクション変換行列
 */
void RenderSystem::SetProjectionMatrix(DirectX::SimpleMath::Matrix* _projectionMatrix)
{
    DirectX::SimpleMath::Matrix mat = _projectionMatrix->Transpose();
    D3D11System::GetContext()->UpdateSubresource(RenderSystem::projectionBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**	@brief	ビュー変換行列をGPUに送る
 *	@param	DirectX::SimpleMath::Matrix*	_viewMatrix	ビュー変換行列
 */
void RenderSystem::SetViewMatrix(DirectX::SimpleMath::Matrix* _viewMatrix)
{
    DirectX::SimpleMath::Matrix mat = _viewMatrix->Transpose();
    D3D11System::GetContext()->UpdateSubresource(RenderSystem::viewBuffer.Get(), 0, nullptr, &mat, 0, 0);
}

/**	@brief 指定したブレンドステートを設定
 *	@param BlendStateType _blendState 使用するブレンドステートの種類
 */
void RenderSystem::SetBlendState(BlendStateType _blendState)
{
    if (static_cast<int>(_blendState) >= 0 && static_cast<int>(_blendState) < static_cast<int>(BlendStateType::MAX_BLENDSTATE))
    {
        float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        D3D11System::GetContext()->OMSetBlendState(RenderSystem::blendState[static_cast<int>(_blendState)].Get(), blendFactor, 0xffffffff);
    }
}

/** @brief Alpha To Coverage（マルチサンプリング対応の透明処理）用のON/OFFを切り替える
 *  @param bool	_enable	true：ATC有効	false：無効
 *  @details    マルチサンプリング＋アルファブレンドの高度な合成を行う
 */
void RenderSystem::SetATCEnable(bool _enable)
{
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    D3D11System::GetContext()->OMSetBlendState(
        _enable ? RenderSystem::blendStateATC.Get() : RenderSystem::blendState[0].Get(),
        blendFactor, 0xffffffff);
}

/**	@brief 面の除外（カリング）を無効、有効にする
 *	@param bool _cullflag = false true：カリングON（通常）　false：カリングOFF（両面描画）
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
    HRESULT hr = D3D11System::GetDevice()->CreateRasterizerState(&rasterizerDesc, pRasterizerState.GetAddressOf());
    if (FAILED(hr))
        return;

    D3D11System::GetContext()->RSSetState(pRasterizerState.Get());
}

/**	@brief ラスタライザステートのフィルモード（塗りつぶし、ワイヤーフレーム）を設定する
 *	@param D3D11_FILL_MODE _fillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
 */
void RenderSystem::SetFillMode(D3D11_FILL_MODE _fillMode)
{
    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = _fillMode;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;

    ComPtr<ID3D11RasterizerState> rs;
    D3D11System::GetDevice()->CreateRasterizerState(&rasterizerDesc, rs.GetAddressOf());
    D3D11System::GetContext()->RSSetState(rs.Get());
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
    HRESULT hr = D3D11System::GetDevice()->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf());
    if (SUCCEEDED(hr))
    {
        D3D11System::GetContext()->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
    }
}

///**	@brief	ビューポートを追加
// *	@param	const D3D11_VIEWPORT& _viewport	追加するビューポート
// */
//void RenderSystem::AddViewport(const D3D11_VIEWPORT& _viewport)
//{
//
//}
//
///**	@brief	指定のビューポートを削除
// *	@param	const int _viewportType	ビューポートの番号
// */
//void RenderSystem::RemoveViewport(const int _viewportType)
//{
//
//}
//
///**	@brief	指定のビューポートを
// *	@param	const int _viewportType	ビューポートの番号
// */
//void RenderSystem::RemoveViewport(const int _viewportType)
//{
//
//}