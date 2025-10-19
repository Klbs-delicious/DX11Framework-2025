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

using namespace ShaderCommon;

//-----------------------------------------------------------------------------
// ShaderManager class
//-----------------------------------------------------------------------------

ShaderManager::ShaderManager() :
	d3d11(SystemLocator::Get<D3D11System>()), 
	shaderMap(), 
	shaderInfoMap(),
	defaultShadersMap(), 
	shaderProgramMap(),
	deafultShader()
{
	// シェーダー情報の事前登録
	this->PreRegisterShaderInfo("TestVS", ShaderInfo(ShaderType::VertexShader, L"VertexShader/VS_Test"));
	this->PreRegisterShaderInfo("TestPS", ShaderInfo(ShaderType::PixelShader, L"PixelShader/PS_Test"));

	// シェーダープログラムの登録
	this->CreateShaderProgram("Default", { "TestVS","TestPS","","","" });

	// デフォルト設定のシェーダーリソースを登録
	this->defaultShadersMap[ShaderType::VertexShader] = this->Get("TestVS");
	this->defaultShadersMap[ShaderType::PixelShader] = this->Get("TestPS");

	// デフォルト設定のシェーダープログラムを登録
	this->deafultProgram = this->GetShaderProgram("Default");
}

ShaderManager::~ShaderManager()
{
	this->deafultShader = nullptr;
	this->defaultShadersMap.clear();
	this->shaderInfoMap.clear();
	this->shaderMap.clear();
}

/** @brief  リソースを登録する
 *	@param  const std::string& _key	リソースのキー
 *  @return ShaderBase* 登録されていない場合nullptr
 */
ShaderBase* ShaderManager::Register(const std::string& _key)
{
	if (this->shaderMap.contains(_key)) return this->Get(_key);
	auto it = this->shaderInfoMap.find(_key);
	if (it == this->shaderInfoMap.end()) { return nullptr; }

	const ShaderInfo& info = it->second;
	std::unique_ptr<ShaderBase> shader;
	switch (info.shaderType)
	{
	case ShaderType::VertexShader: shader = std::make_unique<VertexShader>(); break;
	case ShaderType::PixelShader: shader = std::make_unique<PixelShader>();  break;
	default: return nullptr;
	}
	if (!shader) { return nullptr; }

	if (!shader->CreateShader(*d3d11.GetDevice(), info)) { return nullptr; }

	// 登録
	ShaderBase* rawPtr = shader.get();
	this->shaderMap.emplace(_key, std::move(shader));
	return rawPtr;
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

/**	@brief シェーダープログラムの作成
 *	@param const std::string& _programName プログラム名
 *	@param std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> _shaderNames 各シェーダー名
 *	@return 登録が無事済んだら true
 */
bool ShaderManager::CreateShaderProgram(const std::string& _programName, std::array<std::string, static_cast<size_t>(ShaderType::MaxShaderType)> _shaderNames)
{
	//順にシェーダーを取得していく
	std::array < ShaderBase*, static_cast<size_t>(ShaderType::MaxShaderType)> shaders;
	for (size_t i = 0; i < static_cast<size_t>(ShaderType::MaxShaderType); ++i)
	{
		const auto& name = _shaderNames[i];
		if (name.empty()) continue; // 空文字はスキップ

		shaders[i] = this->Get(name.c_str());
		if (!shaders[i])
		{
			OutputDebugStringA(("Shader not found: " + name + "\n").c_str());
			return false;
		}
	}
	// シェーダープログラムに格納する
	ShaderProgram program = {};
	program.vs = shaders[static_cast<size_t>(ShaderType::VertexShader)];
	program.ps = shaders[static_cast<size_t>(ShaderType::PixelShader)];

	// マップに登録する
	if (this->shaderProgramMap.contains(_programName)) return true;
	this->shaderProgramMap.emplace(_programName, program);
	return true;
}

/**	@brief シェーダープログラムの取得
 *	@param const std::string& _programName プログラム名
 *	@return ShaderProgram*	シェーダープログラムの参照
 */
ShaderProgram* ShaderManager::GetShaderProgram(const std::string& _programName)
{
	// シェーダープログラムが存在しない
	if (!this->shaderProgramMap.contains(_programName))
	{
		std::cerr << "ShaderManager::Get: Failed to register ShaderProgram for ProgramName: " << _programName << std::endl;
		return nullptr;
	}
	// キーに対応するシェーダープログラムを返す
	return &this->shaderProgramMap.at(_programName);
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

/**	@brief	指定したタイプのデフォルト設定のシェーダーを取得する
 *	@param	ShaderType _type	シェーダーの種類
 *	@return	ShaderBase*	リソースのポインタ、ない場合は nullptrが返される
 */
ShaderBase* ShaderManager::Default(ShaderType _type)const
{
	// シェーダー情報が存在しない
	if (!this->defaultShadersMap.contains(_type)) { return nullptr; }

	// キーに対応するシェーダーを返す
	return this->defaultShadersMap.at(_type);
}