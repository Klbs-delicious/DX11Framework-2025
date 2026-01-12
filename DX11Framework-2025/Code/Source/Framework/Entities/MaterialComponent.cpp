/** @file   MaterialComponent.cpp
 *  @date   2025/10/19
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Graphics/SpriteManager.h"

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
    if (!this->baseMaterial) 
    {
        auto materialManager = this->Owner()->Services()->materials;
        this->baseMaterial = materialManager->Default();

        // GPUに送る
        this->baseMaterial->materialBuffer->Update(SystemLocator::Get<D3D11System>().GetContext(), this->param);
    }
    if (!this->baseMaterial->albedoMap)
    {
		// デフォルトテクスチャを設定する
        auto& texMgr = this->Owner()->Services()->sprites;
        this->baseMaterial->albedoMap = texMgr->Default();
    }

    //// GPUに送る
    //this->baseMaterial->materialBuffer->Update(SystemLocator::Get<D3D11System>().GetContext(), this->param);
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
 *	@param TextureResource* _overrideSprite	画像情報
 */
void MaterialComponent::SetTexture(TextureResource* _overrideSprite)
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

void MaterialComponent::SetDiffuse(const DX::Color& _diffuse)
{
    this->param.Diffuse = _diffuse;
}

/** @brief 指定したブレンドステートを設定
 *  @param BlendStateType _blendState 使用するブレンドステートの種類
 */
void MaterialComponent::SetBlendState(BlendStateType _blendState)
{
    if (this->baseMaterial)
    {
        this->baseMaterial->blendStateType = _blendState;
	}
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
    TextureResource* useTexture = (this->overrideTexture) ? this->overrideTexture : this->baseMaterial->albedoMap;
    if (useTexture)
    {
        ID3D11ShaderResourceView* srv = useTexture->texture.Get();
        _context->PSSetShaderResources(0, 1, &srv);
    }

    // ConstantBuffer
    if (this->baseMaterial->materialBuffer)
    {
        this->baseMaterial->materialBuffer->Update(_context, this->param);
        this->baseMaterial->materialBuffer->BindVS(_context, 3);
        this->baseMaterial->materialBuffer->BindPS(_context, 3);
    }

	// BlendState
	_renderSystem->SetBlendState(this->baseMaterial->blendStateType);
}