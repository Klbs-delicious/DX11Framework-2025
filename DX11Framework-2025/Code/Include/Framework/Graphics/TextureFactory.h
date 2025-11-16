/** @file   TextureFactory.h
 *  @date   2025/11/15
 */
#pragma once

#include <memory>
#include <d3d11.h>
#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Graphics/TextureResource.h"

 /** @class TextureFactory
  *  @brief 動的にテクスチャを生成するユーティリティ
  *  @details
  *      - 単色テクスチャなどファイルを使用しない動的生成用。
  *      - TextureLoader と役割を分離して管理を明確化する。
  */
class TextureFactory
{
public:

    /** @brief 単色テクスチャを生成する
     *  @param _device   D3D11デバイス
     *  @param _color    単色 (RGBA, 0〜255)
     *  @param _width    幅
     *  @param _height   高さ
     *  @return TextureResource*
     */
    static std::unique_ptr<TextureResource> CreateSolidColorTexture(
        ID3D11Device* _device,
        const DX::Color& _color,
        int _width = 1,
        int _height = 1);
};