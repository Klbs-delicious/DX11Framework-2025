/** @file   ShaderManager.cpp
*   @date   2025/10/08
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/SystemLocator.h"
#include"Framework/Core/ShaderManager.h"
#include"Framework/Shaders/VertexShader.h"
#include"Framework/Shaders/PixelShader.h"

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

ShaderManager::~ShaderManager() {}

/** @brief  リソースを登録する
 *	@param  const std::string& _key	リソースのキー
 *	@return bool	登録に成功したら true
 */
bool ShaderManager::Register(const std::string& _key)
{
	// すでに登録されている
	if (this->shaderMap.contains(_key)) { return false; }

	// シェーダー情報が登録されていない
	if (!this->shaderInfoMap.contains(_key)) { return false; }

	// シェーダー情報を取得
	const ShaderInfo& info = this->shaderInfoMap.at(_key);

	// シェーダーの種類に応じてインスタンスを生成
	std::unique_ptr<ShaderBase> shader;
	switch (info.shaderType)
	{
	case ShaderType::VertexShader:
		shader = std::make_unique<VertexShader>();
		break;
	case ShaderType::PixelShader:
		shader = std::make_unique<PixelShader>();
		break;
	}
	if (!shader) { return false; }

	// シェーダーの生成
	shader->CreateShader(*this->d3d11.GetDevice(), info);
	if (!shader->GetBlob()) { return false; }

	// 作成したシェーダーをマップに登録する
	this->shaderMap.emplace(_key, std::move(shader));
	return true;
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
 *	@return	T*	リソースのポインタ、見つからなかった場合は nullptr
 */
ShaderBase* ShaderManager::Get(const std::string& _key)
{
	// シェーダー情報が存在しない
	if (!this->shaderInfoMap.contains(_key)) { return nullptr; }

	if (!this->shaderMap.contains(_key))
	{
		// 登録されていなければ登録を試みる
		if (!this->Register(_key))
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