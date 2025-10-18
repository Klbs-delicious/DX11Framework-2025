/**	@file	IResourceManager.h
*	@date	2025/09/24
*/
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include<string>
#include<memory>

template<typename T>
/** @class	リソース管理インターフェース
 *  @brief  テンプレートクラスでリソース管理のインターフェースを定義する
 */
class IResourceManager : private NonCopyable
{
public:
	using ResourceType = T; // ResourceHubで型名を判定するのに使用する

	/// @brief 仮想デストラクタ
    virtual ~IResourceManager() = default;

	/** @brief  リソースを登録する
	 *	@param  const std::string& _key	リソースのキー
	 *	@return bool	登録に成功したら true
     */
    virtual bool Register(const std::string& _key) = 0;

	/**	@brief リソースの登録を解除する
	 *	@param  const std::string& _key	リソースのキー
	 */
	virtual  void Unregister(const std::string& _key) = 0;

	/**	@brief	キーに対応するリソースを取得する
	 *	@param	const std::string& _key	リソースのキー
	 *	@return	const T*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	virtual T* Get(const std::string& _key) = 0;

	/**	@brief	デフォルトのリソースを取得する
	 *	@return	const T*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	virtual T* Default()const = 0;
};