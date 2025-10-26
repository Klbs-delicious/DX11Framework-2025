/** @file   SpriteManager.cpp
*   @date   2025/09/25
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Utils/CommonTypes.h"

#include	<iostream>

//-----------------------------------------------------------------------------
// SpriteManager class
//-----------------------------------------------------------------------------

/// @brief 画像の読み込みパスを登録
void SpriteManager::TexturepathRegister()
{
	// 画像のパスを登録していく
	this->spritePathMap["Default"] = u8"Assets/Textures/Default.png";
	this->spritePathMap["Eidan"] = u8"Assets/Textures/Eidan.png";
}

//// @brief コンストラクタ
SpriteManager::SpriteManager() :textureLorder(nullptr), spriteMap(), spritePathMap(), defaultSprite(nullptr)
{
	// 画像の読み込みパスを登録
	this->TexturepathRegister();

	// テクスチャ読み込みクラスの生成
	this->textureLorder = std::make_unique<TextureLoader>();

	// デフォルト画像を設定しておく
	this->defaultSprite = this->Get("Default");
}

/// @brief デストラクタ
SpriteManager::~SpriteManager()
{
	this->spriteMap.clear();		
	this->spritePathMap.clear();
}

/** @brief  リソースを登録する
 *	@param  const std::string& _key	リソースのキー
 *  @return TextureResource* 登録されていない場合nullptr
 */
TextureResource* SpriteManager::Register(const std::string& _key)
{
	// すでに登録済みならそのまま返す
	if (this->spriteMap.contains(_key)) return this->Get(_key);

	// パスが存在するか確認
	auto it = this->spritePathMap.find(_key);
	if (it == this->spritePathMap.end()) { return nullptr; }

	// 画像読み込み処理
	auto tex = this->textureLorder->FromFile(it->second);
	if (!tex) { return nullptr; }

	// 登録
	TextureResource* rawPtr = tex.get();
	this->spriteMap.emplace(_key, std::move(tex));
	return rawPtr;
}

/**	@brief リソースの登録を解除する
 *	@param  const std::string& _key	リソースのキー
 */
void SpriteManager::Unregister(const std::string& _key)
{
	// 存在しない場合は処理を行わない
	if (!this->spriteMap.contains(_key)) { return; }

	auto it = this->spriteMap.find(_key);
	if (it != this->spriteMap.end())
	{
		it->second.reset();			// リソースの中身（unique_ptr）を解放        
		this->spriteMap.erase(it);	// キーと空のポインタをマップから削除
	}
}

/**	@brief	キーに対応するリソースを取得する
 *	@param	const std::string& _key	リソースのキー
 *	@return	const TextureResource*	リソースのポインタ、見つからなかった場合は nullptr
 */
TextureResource* SpriteManager::Get(const std::string& _key)
{
	// すでに登録済みならそのまま返す
	auto it = this->spriteMap.find(_key);
	if (it != this->spriteMap.end()) 
	{
		return it->second.get();
	}
	// パスが存在するか確認
	auto pathIt = this->spritePathMap.find(_key);
	if (pathIt == this->spritePathMap.end()) 
	{
		std::cerr << "TextureResource path not found for key: " << _key << std::endl;
		return nullptr;
	}

	// 登録処理
	if (!this->Register(_key)) {
		std::cerr << "Failed to register sprite for key: " << _key << std::endl;
		return nullptr;
	}

	return this->spriteMap.at(_key).get();
}