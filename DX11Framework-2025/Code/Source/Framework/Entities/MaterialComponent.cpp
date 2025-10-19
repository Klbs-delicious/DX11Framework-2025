﻿/** @file   MaterialComponent.cpp
 *  @date   2025/10/19
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Graphics/MaterialManager.h"

//-----------------------------------------------------------------------------
// MaterialComponent class
//-----------------------------------------------------------------------------

MaterialComponent::MaterialComponent(GameObject* _owner, bool _isActive)
    : Component(_owner, _isActive), baseMaterial(nullptr), overrideTexture(nullptr)
{
    this->param = {};
    this->param.Ambient = { 1,1,1,1 };
    this->param.Diffuse = { 1,1,1,1 };
    this->param.Specular = { 0,0,0,1 };
    this->param.Emission = { 0,0,0,1 };
    this->param.Shiness = 32.0f;
    this->param.TextureEnable = TRUE;
}

MaterialComponent::~MaterialComponent()
{
    this->Dispose();
}

void MaterialComponent::Initialize()
{
    // マテリアルが未設定の場合はデフォルトマテリアルを設定する
    if (!this->baseMaterial) {
        auto materialManager = this->Owner()->Services()->materials;
        this->baseMaterial = materialManager->Default();
    }
}
void MaterialComponent::Dispose() 
{ 
    this->baseMaterial = nullptr;
    this->overrideTexture = nullptr;
}

void MaterialComponent::SetMaterial(Material* _baseMaterial)
{
    this->baseMaterial = _baseMaterial;
}

/**	@brief 画像情報の設定
 *	@param Sprite* _overrideSprite	画像情報
 */
void MaterialComponent::SetTexture(Sprite* _overrideSprite)
{
    this->overrideTexture = _overrideSprite;
}

/**	@brief パラメータ情報の設定
 *	@param const MaterialParams& _params	パラメータ情報の参照
 */
void MaterialComponent::SetParams(const MaterialParams& _params)
{
    this->param = _params;
}

Material* MaterialComponent::GetMaterial() const
{
    return this->baseMaterial;
}

/** @brief マテリアルを適用する
 *  @param ID3D11DeviceContext* _context
 *  @param RenderSystem* _renderSystem
 */
void MaterialComponent::Apply(ID3D11DeviceContext* _context, RenderSystem* _renderSystem)
{
    if (!this->baseMaterial) return;

    // Shader
    if (this->baseMaterial->shaders)
        this->baseMaterial->shaders->Bind(*_context);

    // Sampler 
    _renderSystem->SetSampler(this->baseMaterial->samplerType);

    // Texture（個別設定を優先）
    Sprite* useTexture = (this->overrideTexture) ? this->overrideTexture : this->baseMaterial->albedoMap;
    if (useTexture)
    {
        ID3D11ShaderResourceView* srv = useTexture->texture.Get();
        _context->PSSetShaderResources(0, 1, &srv);
    }

    // ConstantBuffer
    if (this->baseMaterial->materialBuffer)
    {
        this->baseMaterial->materialBuffer->Update(_context, this->param);
        this->baseMaterial->materialBuffer->BindPS(_context, 2);
    }
}
