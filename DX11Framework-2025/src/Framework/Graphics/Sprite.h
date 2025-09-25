/**	@file	Sprite.h
*	@date	2025/09/24
*/
#pragma once
#include"Framework/Utils/CommonTypes.h"
#include <d3d11.h>

/**	@brief スプライト情報
 */
struct Sprite
{
	DX::ComPtr<ID3D11ShaderResourceView> texture;	///< テクスチャ情報
	int width;										///< テクスチャの幅
	int height;										///< テクスチャの高さ
	int bpp;										///< テクスチャの1ピクセルあたりのビット数

	Sprite() :texture(nullptr), width(0), height(0), bpp(0) {}

	~Sprite() {
		if (texture) { texture.Reset(); }
	}
};