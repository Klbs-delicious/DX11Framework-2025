/**	@file	SpriteManager.h
*	@date	2025/09/24
*/
#pragma once
#include"Include/Framework/Core/IResourceManager.h"
#include"Include/Framework/Graphics/Sprite.h"
#include<unordered_map>

/**	@class	SpriteManager
 *	@brief	画像の生成、保持破棄を管理する
 */
class SpriteManager :public IResourceManager<Sprite>
{
public:
	//// @brief コンストラクタ
	SpriteManager();
	/// @brief デストラクタ
	~SpriteManager()override;

	/** @brief  リソースを登録する
	 *	@param  const std::string& _key	リソースのキー
	 *	@return bool	登録に成功したら true
	 */
	bool Register(const std::string& _key) override;

	/**	@brief リソースの登録を解除する
	 *	@param  const std::string& _key	リソースのキー
	 */
	 void Unregister(const std::string& _key)override;

	/**	@brief	キーに対応するリソースを取得する
	 *	@param	const std::string& _key	リソースのキー
	 *	@return	const Sprite*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	Sprite* Get(const std::string& _key) override;
	
	/**	@brief	デフォルトのリソースを取得する
	 *	@return	const T*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	Sprite* Default()const override { return this->defaultSprite; }

private:

	/**	@brief	画像の読み込み
	 *	@param	const std::u8string& _path	画像のファイルパス
	 *	@return	std::unique_ptr<Sprite>	画像データ（失敗した場合は nullptr）
	 */
	std::unique_ptr<Sprite> LoadTexture(const std::string& _path)const;

	/**	@brief 画像データをメモリから読み込む
	 *	@param	const unsigned char*	_data	画像のバイナリデータ（メモリ上にある）
	 *	@param	int						_len	そのデータのサイズ（バイト数）
	 */
	std::unique_ptr<Sprite> LoadFromMemory(const unsigned char* _data, int _len);

	/// @brief 画像の読み込みパスを登録
	void TexturepathRegister();	

	std::unordered_map<std::string, std::unique_ptr<Sprite>> spriteMap;		///< スプライトのマップ
	std::unordered_map<std::string, std::string> spritePathMap;				///< スプライトに対応する画像パスのマップ

	Sprite* defaultSprite;	///< 未設定の場合に選ばれるスプライト
};