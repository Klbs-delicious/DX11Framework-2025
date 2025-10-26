/**	@file	SpriteManager.h
*	@date	2025/09/24
*/
#pragma once
#include"Include/Framework/Core/IResourceManager.h"
#include"Include/Framework/Graphics/TextureLoader.h"
#include"Include/Framework/Graphics/TextureResource.h"
#include<unordered_map>
#include<memory>
#include <string>

/**	@class	SpriteManager
 *	@brief	画像の生成、保持破棄を管理する
 */
class SpriteManager :public IResourceManager<TextureResource>
{
public:
	//// @brief コンストラクタ
	SpriteManager();
	/// @brief デストラクタ
	~SpriteManager()override;

	/** @brief  リソースを登録する
	 *	@param  const std::string& _key	リソースのキー
	 *  @return TextureResource* 登録されていない場合nullptr
	 */
	TextureResource* Register(const std::string& _key) override;

	/**	@brief リソースの登録を解除する
	 *	@param  const std::string& _key	リソースのキー
	 */
	 void Unregister(const std::string& _key)override;

	/**	@brief	キーに対応するリソースを取得する
	 *	@param	const std::string& _key	リソースのキー
	 *	@return	const TextureResource*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	TextureResource* Get(const std::string& _key) override;
	
	/**	@brief	デフォルトのリソースを取得する
	 *	@return	const T*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	TextureResource* Default()const override { return this->defaultSprite; }

private:

	/// @brief 画像の読み込みパスを登録
	void TexturepathRegister();	

	std::unique_ptr<TextureLoader>	textureLorder;	///< テクスチャの読み込み

	std::unordered_map<std::string, std::unique_ptr<TextureResource>> spriteMap;		///< スプライトのマップ
	std::unordered_map<std::string, std::string> spritePathMap;							///< スプライトに対応する画像パスのマップ

	TextureResource* defaultSprite;					///< 未設定の場合に選ばれるスプライト
};