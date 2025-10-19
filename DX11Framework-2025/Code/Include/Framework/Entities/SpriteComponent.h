/**@file   SpriteComponent.h
 * @date   2025/10/13
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Graphics/Sprite.h"
#include "Include/Framework/Entities/GameObject.h"

 /** @class	SpriteComponent
  *	 @brief	画像情報を保持するコンポーネント
  */
class SpriteComponent :public Component
{
public:
	SpriteComponent(GameObject* _owner, bool _isActive = true);
	~SpriteComponent()override;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/**	@brief 画像情報の設定
	 *	@param Sprite* _sprite	画像情報
	 */
	void SetSprite(Sprite* _sprite);

	/**	@brief	画像情報の取得
	 *	@return	Sprite*	画像情報
	 */
	const Sprite* GetSprite()const;

	/** @brief 画像を適用する
	 *  @param ID3D11DeviceContext* _context
	 */
	void Apply(ID3D11DeviceContext* _context);

private :
	const Sprite* sprite;	///< 画像をまとめたデータ構造
};