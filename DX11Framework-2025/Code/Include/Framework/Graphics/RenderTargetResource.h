/** @file   RenderTargetResource.h
 *  @brief  レンダーターゲット用リソース定義
 *  @date   2026/02/24
 */
#pragma once

#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Graphics/TextureResource.h"

 /** @struct RenderTargetResource
  *  @brief レンダーターゲット用のリソースクラス
  */
struct RenderTargetResource : public TextureResource
{
	/// @brief 有効なリソースを保持しているか
	bool IsValid() const;

	/** @brief レンダーターゲットをクリアする
	 *  @param _context D3D11デバイスコンテキスト
	 *  @param _r 赤
	 *  @param _g 緑
	 *  @param _b 青
	 *  @param _a 透明度
	 */
	void Clear(ID3D11DeviceContext* _context, float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 1.0f);

	/** @brief レンダーターゲットを生成する
	 *  @param _device D3D11デバイス
	 *  @param _width 幅
	 *  @param _height 高さ
	 *  @param _format フォーマット
	 *  @return 作成成功で true
	 */
	bool CreateRenderTarget(ID3D11Device* _device, int _width, int _height, DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM);

	/** @brief 既存のテクスチャをレンダーターゲットとして使用する
	 *  @param _existingTexture 既存のテクスチャ
	 *  @param _device D3D11デバイス
	 *  @return 成功で true
	 */
	bool Attach(ID3D11Texture2D* _existingTexture, ID3D11Device* _device);

	/// @brief リソースを解放する
	void Release();

	DX::ComPtr<ID3D11RenderTargetView> renderTargetView;	///< 書き込み用ビュー
	DX::ComPtr<ID3D11Texture2D> texture2D;					///< テクスチャ本体
	D3D11_VIEWPORT viewport = {};							///< ビューポート
};
