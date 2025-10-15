/**@file   ShaderManager.h
 * @date   2025/10/08
 */
#pragma once
#include"Include/Framework/Utils/CommonTypes.h"
#include"Include/Framework/Core/D3D11System.h"
#include"Include/Framework/Core/IResourceManager.h"
#include"Include/Framework/Shaders/ShaderBase.h"
#include"Include/Framework/Shaders/ShaderCommon.h"

#include<unordered_map>
#include<memory>

using namespace ShaderCommon;

 /** @class	ShaderManager
  *	 @brief	シェーダーのリソースを管理するクラス
  */
class ShaderManager :public IResourceManager<ShaderBase>
{
public:

	ShaderManager();
	~ShaderManager();

	/** @brief  リソースを登録する
	 *	@param  const std::string& _key	リソースのキー
	 *	@return bool	登録に成功したら true
	 */
	virtual bool Register(const std::string& _key) override;

	/**	@brief リソースの登録を解除する
	 *	@param  const std::string& _key	リソースのキー
	 */
	virtual  void Unregister(const std::string& _key)override;

	/**	@brief	キーに対応するリソースを取得する
	 *	@param	const std::string& _key	リソースのキー
	 *	@return	const ShaderBase*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	virtual ShaderBase* Get(const std::string& _key)const  override;

	/**	@brief シェーダー情報を事前登録する
	 *	@param	const std::string& _key	リソースのキー
	 *	@param	const ShaderInfo& _info	シェーダー情報
	 *	@return	bool 登録に成功したら true
	 */
	bool PreRegisterShaderInfo(const std::string& _key, const ShaderInfo& _info);

private:
	/**	@brief 論理的constを使用してリソースの登録を行う
	 *	@param  const std::string& _key	リソースのキー
	 *	@return bool	登録に成功したら true
	 */
	bool RegisterInternal(const std::string& _key) const;

	D3D11System& d3d11;	///< D3D11システムの参照

	mutable std::unordered_map<std::string, std::unique_ptr<ShaderBase>> shaderMap;	///< シェーダーマップ
	mutable std::unordered_map<std::string, ShaderInfo>	shaderInfoMap;				///< シェーダー情報マップ
};
