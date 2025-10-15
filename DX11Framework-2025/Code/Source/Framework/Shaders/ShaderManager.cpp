/** @file   ShaderManager.cpp
*   @date   2025/10/08
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Shaders/ShaderManager.h"
#include"Include/Framework/Shaders/VertexShader.h"
#include"Include/Framework/Shaders/PixelShader.h"

#include<iostream>

//-----------------------------------------------------------------------------
// ShaderManager class
//-----------------------------------------------------------------------------

ShaderManager::ShaderManager() :d3d11(SystemLocator::Get<D3D11System>()), shaderMap(), shaderInfoMap()
{
	// シェーダー情報の事前登録
	this->PreRegisterShaderInfo("TestVS", ShaderInfo(ShaderType::VertexShader, L"VertexShader/VS_Test"));
	this->PreRegisterShaderInfo("TestPS", ShaderInfo(ShaderType::PixelShader, L"PixelShader/PS_Test"));
}

ShaderManager::~ShaderManager()
{
	this->shaderInfoMap.clear();
	this->shaderMap.clear();
}

/** @brief  リソースを登録する
 *	@param  const std::string& _key	リソースのキー
 *	@return bool	登録に成功したら true
 */
bool ShaderManager::Register(const std::string& _key)
{
	return this->RegisterInternal(_key);
}

/**	@brief リソースの登録を解除する
 *	@param  const std::string& _key	リソースのキー
 */
void ShaderManager::Unregister(const std::string& _key)
{
	if (!this->shaderMap.erase(_key))
	{
		// キーが存在しなかった
		std::cerr << "ShaderManager::Unregister: Key not found: " << _key << std::endl;
	}
}

/**	@brief	キーに対応するリソースを取得する
 *	@param	const std::string& _key	リソースのキー
 *	@return	const ShaderBase*	リソースのポインタ、見つからなかった場合は nullptr
 */
ShaderBase* ShaderManager::Get(const std::string& _key)const
{
	// シェーダー情報が存在しない
	if (!this->shaderInfoMap.contains(_key)) { return nullptr; }

	if (!this->shaderMap.contains(_key))
	{
		// 登録されていなければ登録を試みる
		// const のままlazy-load
		if (!this->RegisterInternal(_key)) 
		{
			std::cerr << "ShaderManager::Get: Failed to register shader for key: " << _key << std::endl;
			return nullptr;
		}
	}
	// キーに対応するシェーダーを返す
	return this->shaderMap.at(_key).get();
}

/**	@brief シェーダー情報を事前登録する
 *	@param	const std::string& _key	リソースのキー
 *	@param	const ShaderInfo& _info	シェーダー情報
 *	@return	bool 登録に成功したら true
 */
bool ShaderManager::PreRegisterShaderInfo(const std::string& _key, const ShaderInfo& _info)
{
	// すでに登録されている
	if (this->shaderInfoMap.contains(_key)) { return false; }

	this->shaderInfoMap.insert({ _key, _info });
	return true;
}

/**	@brief 論理的constを使用してリソースの登録を行う
 *	@param  const std::string& _key	リソースのキー
 *	@return bool	登録に成功したら true
 */
bool ShaderManager::RegisterInternal(const std::string& _key) const
{
	if (this->shaderMap.contains(_key)) return true;
	auto it = this->shaderInfoMap.find(_key);
	if (it == this->shaderInfoMap.end()) return false;

	const ShaderInfo& info = it->second;
	std::unique_ptr<ShaderBase> shader;
	switch (info.shaderType) {
	case ShaderType::VertexShader: shader = std::make_unique<VertexShader>(); break;
	case ShaderType::PixelShader: shader = std::make_unique<PixelShader>();  break;
	default: return false;
	}
	if (!shader) return false;

	if (!shader->CreateShader(*d3d11.GetDevice(), info)) return false;

	shaderMap.emplace(_key, std::move(shader)); // shaderMap は mutable
	return true;
}