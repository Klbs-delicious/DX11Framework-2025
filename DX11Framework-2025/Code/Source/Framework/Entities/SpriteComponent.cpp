/** @file   SpriteComponent.cpp
 *  @date   2025/10/13
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/SpriteComponent.h"
#include "Include/Framework/Graphics/SpriteManager.h"

//-----------------------------------------------------------------------------
// SpriteComponent class
//-----------------------------------------------------------------------------

SpriteComponent::SpriteComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	sprite(nullptr)
{}

SpriteComponent::~SpriteComponent() {}

/// @brief 初期化処理
void SpriteComponent::Initialize()
{
	if(!this->sprite)
	{
		// 未設定の場合はデフォルト画像を設定する
		SpriteManager* spriteManager = this->Owner()->Services()->sprites;
		this->sprite = spriteManager->Default();
	}
}

/// @brief 終了処理
void SpriteComponent::Dispose()
{
	this->sprite = nullptr;
}

/**	@brief 画像情報の設定
 *	@param TextureResource* _sprite	画像情報
 */
void SpriteComponent::SetSprite(TextureResource* _sprite)
{
	this->sprite = _sprite;
}

/**	@brief	画像情報の取得
 *	@return	TextureResource*	画像情報
 */
const TextureResource* SpriteComponent::GetSprite()const 
{
	return this->sprite;
}

/** @brief 画像を適用する
 *  @param ID3D11DeviceContext* _context
 */
void SpriteComponent::Apply(ID3D11DeviceContext* _context)
{
	// テクスチャを送る
	ID3D11ShaderResourceView* srv = this->sprite ? this->sprite->texture.Get() : nullptr;
	_context->PSSetShaderResources(0, 1, &srv);
}