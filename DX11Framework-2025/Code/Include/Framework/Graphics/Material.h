/**	@file	Material.h
 *	@date	2025/10/18
 */
#pragma once
#include"Include/Framework/Utils/CommonTypes.h"
#include"Include/Framework/Shaders/ShaderCommon.h"
#include"Include/Framework/Graphics/TextureResource.h"
#include"Include/Framework/Graphics/DynamicConstantBuffer.h"
#include"Include/Framework/Core/RenderSystem.h"

#include<memory>

/** @struct MaterialParams
 *  @brief  マテリアルのオブジェクトごとに変化する値
 */
struct MaterialParams
{ 
    DX::Color Ambient;  ///< アンビエント色
    DX::Color Diffuse;  ///< 拡散反射光
    DX::Color Specular; ///< 鏡面反射光
    DX::Color Emission; ///< 自己発光色
    float Shiness;      ///< 光沢度
    BOOL TextureEnable; ///< テクスチャ使用フラグ
    float Dummy[2];     ///< 予備領域

    MaterialParams() :
        Ambient(1, 1, 1, 1),
        Diffuse(1, 1, 1, 1),
        Specular(0, 0, 0, 1),
        Emission(0, 0, 0, 1),
        Shiness(32.0f),
        TextureEnable(TRUE),
        Dummy{ 0.0f, 0.0f }
    { }
};

/**	@brief マテリアル情報
 */
struct Material
{
    ShaderCommon::ShaderProgram* shaders;                                   ///< シェーダー
    TextureResource* albedoMap;                                             ///< テクスチャ（ベースカラー）
    SamplerType samplerType;                                                ///< サンプラーの種類
    std::unique_ptr<DynamicConstantBuffer<MaterialParams>> materialBuffer;  ///< 定数バッファ
    BlendStateType blendStateType;                                          ///< ブレンドステートの種類

    Material() :
        shaders(nullptr), 
        albedoMap(nullptr), 
        samplerType(SamplerType::LinearWrap), 
        materialBuffer(std::make_unique<DynamicConstantBuffer<MaterialParams>>()),
		blendStateType(BlendStateType::BS_NONE)
    {}

    ~Material() 
    {
        this->shaders = nullptr;
        this->albedoMap = nullptr;
        this->materialBuffer.reset();
    }
};