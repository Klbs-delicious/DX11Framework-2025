/**@file   ShaderManager.h
 * @date   2025/10/08
 */
#pragma once
#include"Framework/Utils/CommonTypes.h"
#include"Framework/Core/D3D11System.h"
#include"Framework/Core/IResourceManager.h"
#include"Framework/Shaders/ShaderBase.h"
#include"Framework/Shaders/ShaderCommon.h"

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
	virtual bool Register(const std::string& _key)override;

	/**	@brief リソースの登録を解除する
	 *	@param  const std::string& _key	リソースのキー
	 */
	virtual  void Unregister(const std::string& _key)override;

	/**	@brief	キーに対応するリソースを取得する
	 *	@param	const std::string& _key	リソースのキー
	 *	@return	T*	リソースのポインタ、見つからなかった場合は nullptr
	 */
	virtual ShaderBase* Get(const std::string& _key) override;

	/**	@brief シェーダー情報を事前登録する
	 *	@param	const std::string& _key	リソースのキー
	 *	@param	const ShaderInfo& _info	シェーダー情報
	 *	@return	bool 登録に成功したら true
	 */
	bool PreRegisterShaderInfo(const std::string& _key, const ShaderInfo& _info);

private:
	D3D11System& d3d11;	///< D3D11システムの参照

	std::unordered_map<std::string, std::unique_ptr<ShaderBase>> shaderMap;	///< シェーダーマップ
	std::unordered_map<std::string, ShaderInfo>	shaderInfoMap;				///< シェーダー情報マップ
};
