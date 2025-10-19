/**@file   ShaderManager.h
 * @date   2025/10/08
 */
#pragma once
#include"Include/Framework/Core/D3D11System.h"
#include"Include/Framework/Core/IResourceManager.h"
#include"Include/Framework/Shaders/ShaderBase.h"
#include"Include/Framework/Shaders/ShaderCommon.h"

#include<unordered_map>
#include<memory>
#include<array>

using namespace ShaderCommon;

/** @class	IShaderManager
 *	@brief	シェーダー特有の機能インターフェース
 *	@details
 *		- IResourceManager<ShaderBase>のままではシェーダーの種類ごとにデフォルトのリソースを返すことができないため
 *		  専用のインターフェースを作成した
 */
class IShaderManager : public IResourceManager<ShaderBase> 
{
public:
	virtual ShaderBase* Default(ShaderType _type) const = 0;
};

 /** @class	ShaderManager
  *	 @brief	シェーダーのリソースを管理するクラス
  *	@details
  *		- シェーダーの種類ごとにデフォルトのリソースを返す為に専用インターフェースIShaderManagerを挟んでいる
  *		- IResourceManager<ShaderBase>::Default()は使用しない想定で、共通リソースとして頂点シェーダーを返す
  *		- 管理する際はIResourceManager<ShaderBase> で、取得時はShaderManagerで取得する想定
  */
class ShaderManager :public IShaderManager
{
public:
	// ResourceHubで型名を判定するのに使用する
	// 間に中間クラスIShaderManagerが挟まっているため、依存名としての型を明示している
	using ResourceType = ShaderBase; 

	ShaderManager();
	~ShaderManager()override;

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
	virtual ShaderBase* Get(const std::string& _key) override;

	/**	@brief シェーダープログラムの作成
	 *	@param const std::string& _programName プログラム名
	 *	@param std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> _shaderNames 各シェーダー名
	 *	@return 登録が無事済んだら true
	 */
	bool CreateShaderProgram(const std::string& _programName, std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> _shaderNames);

	/**	@brief シェーダープログラムの取得
	 *	@param const std::string& _programName プログラム名
	 *	@return ShaderProgram*	シェーダープログラムの参照
	 */
	ShaderProgram* GetShaderProgram(const std::string& _programName);

	/**	@brief シェーダー情報を事前登録する
	 *	@param	const std::string& _key	リソースのキー
	 *	@param	const ShaderInfo& _info	シェーダー情報
	 *	@return	bool 登録に成功したら true
	 */
	bool PreRegisterShaderInfo(const std::string& _key, const ShaderInfo& _info);

	/**	@brief	デフォルトのリソースを取得する（共通デフォルトとして頂点シェーダーを返す）
	 *	@return	ShaderBase*	共通デフォルトとして頂点シェーダーのポインタ、ない場合は nullptrが返される
	 */
	ShaderBase* Default()const override { return this->deafultShader; }

	/**	@brief	指定したタイプのデフォルト設定のシェーダーを取得する
	 *	@param	ShaderType _type	シェーダーの種類	
	 *	@return	ShaderBase*	リソースのポインタ、ない場合は nullptrが返される
	 */
	ShaderBase* Default(ShaderType _type)const override;

	/**	@brief	デフォルト設定のシェーダープログラムを取得する
	 *	@return ShaderProgram* 
	 */
	ShaderProgram* DefaultProgram()const { return this->deafultProgram; }

	D3D11System& d3d11;	///< D3D11システムの参照

	std::unordered_map<std::string, std::unique_ptr<ShaderBase>> shaderMap;	///< シェーダーマップ
	std::unordered_map<std::string, ShaderInfo>	shaderInfoMap;				///< シェーダー情報マップ
	std::unordered_map<std::string, ShaderProgram>	shaderProgramMap;		///< シェーダーの組み合わせマップ

	std::unordered_map<ShaderType, ShaderBase*> defaultShadersMap;	///< 未設定の場合に使用するデフォルトシェーダー
	ShaderBase* deafultShader;										///<　共通デフォルト（頂点シェーダー）
	ShaderProgram* deafultProgram;									///<　デフォルトのシェーダープログラム
};