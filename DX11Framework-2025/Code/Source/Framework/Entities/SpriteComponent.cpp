#include "Include/Framework/Entities/SpriteComponent.h"
#include "Include/Framework/Graphics/SpriteManager.h"

SpriteComponent::SpriteComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	sprite(nullptr)
{}

SpriteComponent::~SpriteComponent(){}

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
 *	@param Sprite* _sprite	画像情報
 */
void SpriteComponent::SetSprite(Sprite* _sprite)
{
	this->sprite = _sprite;
}

/**	@brief	画像情報の取得
 *	@return	Sprite*	画像情報
 */
const Sprite* SpriteComponent::GetSprite()const 
{
	return this->sprite;
}