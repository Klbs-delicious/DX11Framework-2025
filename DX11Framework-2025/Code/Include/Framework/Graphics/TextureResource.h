/** @file   TextureResource.h
 *  @date   2025/10/21
 */
#pragma once
#include <d3d11.h>
#include "Include/Framework/Utils/CommonTypes.h"

/** @struct TextureResource
 *  @brief GPU上のテクスチャリソース情報（SRV保持のみ）
 *  @details
 *      - GPUメモリ上のShaderResourceView（SRV）を保持するだけ。
 *      - バインド処理などは行わず、利用側コンポーネントが行う。
 */ 
struct TextureResource
{
    DX::ComPtr<ID3D11ShaderResourceView> texture; ///< テクスチャSRV
    int width = 0;   ///< 幅
    int height = 0;  ///< 高さ
    int bpp = 0;     ///< ピクセルあたりのビット数

    TextureResource() = default;
    ~TextureResource()
    {
        if (this->texture) this->texture.Reset();
    }

    /** @brief 有効なリソースを保持しているか
     *  @return bool 有効なら true 
     */
    bool IsValid() const { return this->texture != nullptr; }
};