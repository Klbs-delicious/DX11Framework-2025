/** @file   RenderTargetResource.cpp
 *  @brief  レンダーターゲット用リソース実装
 *  @date   2026/02/24
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/RenderTargetResource.h"

//-----------------------------------------------------------------------------
// RenderTargetResource
//-----------------------------------------------------------------------------

bool RenderTargetResource::IsValid() const
{
	return TextureResource::IsValid()
		&& this->renderTargetView != nullptr
		&& this->texture2D != nullptr;
}

//-----------------------------------------------------------------------------

void RenderTargetResource::Clear(ID3D11DeviceContext* _context,
	float _r,
	float _g,
	float _b,
	float _a)
{
	if (!this->renderTargetView)
	{
		return;
	}

	float color[4] = { _r, _g, _b, _a };
	_context->ClearRenderTargetView(this->renderTargetView.Get(), color);
}

//-----------------------------------------------------------------------------

bool RenderTargetResource::CreateRenderTarget(ID3D11Device* _device,
	int _width,
	int _height,
	DXGI_FORMAT _format)
{
	if (!_device){ return false; }

	// テクスチャ本体の設定
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = _format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// SRVとRTVの両方で使用する
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	// テクスチャの生成
	HRESULT hr = _device->CreateTexture2D(&textureDesc, nullptr, this->texture2D.GetAddressOf());
	if (FAILED(hr)){ return false; }

	hr = _device->CreateRenderTargetView(this->texture2D.Get(), nullptr, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)){ return false; }

	hr = _device->CreateShaderResourceView(this->texture2D.Get(), nullptr, this->texture.GetAddressOf());
	if (FAILED(hr)){ return false; }

	// ビューポートの設定
	this->viewport.TopLeftX = 0.0f;
	this->viewport.TopLeftY = 0.0f;
	this->viewport.Width = static_cast<float>(_width);
	this->viewport.Height = static_cast<float>(_height);
	this->viewport.MinDepth = 0.0f;
	this->viewport.MaxDepth = 1.0f;

	return true;
}

//-----------------------------------------------------------------------------

bool RenderTargetResource::Attach(ID3D11Texture2D* _existingTexture, ID3D11Device* _device)
{
	this->texture2D = _existingTexture; 

	// バックバッファはSRVを作成できないため空にする
	this->texture.Reset();				

	// サイズ情報を取得してメンバに保存
	D3D11_TEXTURE2D_DESC desc;
	_existingTexture->GetDesc(&desc);
	this->width = desc.Width;
	this->height = desc.Height;

	this->viewport.TopLeftX = 0.0f;
	this->viewport.TopLeftY = 0.0f;
	this->viewport.Width = static_cast<float>(desc.Width);
	this->viewport.Height = static_cast<float>(desc.Height);
	this->viewport.MinDepth = 0.0f;
	this->viewport.MaxDepth = 1.0f;

	// RTVの作成
	return SUCCEEDED(_device->CreateRenderTargetView(_existingTexture, nullptr, &this->renderTargetView));
}

//-----------------------------------------------------------------------------

void RenderTargetResource::Release()
{
	this->renderTargetView.Reset();
	this->texture2D.Reset();
	this->texture.Reset();
	this->width = 0;
	this->height = 0;
	this->bpp = 0;
}