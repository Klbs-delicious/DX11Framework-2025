/** @file   ModelManager.cpp
 *  @date   2026/01/13
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/ModelManager.h"

#include "Include/Framework/Core/ResourceHub.h"

#include <iostream>

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	std::string MakeMaterialKey(const std::string& _modelKey, int _index)
	{
		return _modelKey + "/mat/" + std::to_string(_index);
	}
}

//-----------------------------------------------------------------------------
// ModelManager class
//-----------------------------------------------------------------------------

ModelManager::ModelManager() : modelImporter()
{
	// モデル情報登録
	 this->modelInfoTable.emplace(
	     "Player",
	     ModelInfo{ "Assets/Models/Stickman/source/stickman.fbx", "Assets/Models/Stickman/textures" }
	 );

	this->defaultModel = nullptr;
}

ModelManager::~ModelManager()
{
	this->Clear();
}

ModelEntry* ModelManager::Register(const std::string& _key)
{
	// 既に登録済みならそれを返す
	{
		auto it = this->modelTable.find(_key);
		if (it != this->modelTable.end())
		{
			return it->second.get();
		}
	}

	// 読み込み情報が無いなら登録できない
	auto infoIt = this->modelInfoTable.find(_key);
	if (infoIt == this->modelInfoTable.end())
	{
		std::cerr << "[Error] ModelManager::Register: ModelInfo not found: " << _key << std::endl;
		return nullptr;
	}

	const ModelInfo& info = infoIt->second;

	// Import して ModelData を作る
	auto modelData = std::make_unique<Graphics::Import::ModelData>();
	if (!this->modelImporter.Load(info.filename, info.textureDir, *modelData))
	{
		std::cerr << "[Error] ModelManager::Register: Import failed: " << _key << std::endl;
		return nullptr;
	}

	// Mesh を生成して MeshManager に登録
	auto& meshManager = ResourceHub::Get<MeshManager>();

	auto meshUnique = meshManager.CreateFromModelData(*modelData);
	if (!meshUnique)
	{
		std::cerr << "[Error] ModelManager::Register: CreateFromModelData failed: " << _key << std::endl;
		return nullptr;
	}

	Graphics::Mesh* meshRaw = meshUnique.get();
	meshManager.Register(_key, meshUnique.release());

	// Material を生成して MaterialManager に登録（当面 0 番のみ）
	auto& materialManager = ResourceHub::Get<MaterialManager>();

	const std::string matKey = MakeMaterialKey(_key, 0);
	Material* matRaw = materialManager.Register(matKey);

	// モデル描画用のマテリアル設定を行う
	matRaw->shaders = ResourceHub::Get<ShaderManager>().GetShaderProgram("ModelBasic");

	// ModelData 側にテクスチャがあるなら差し替える（当面 0 番のみ）
	if (matRaw && !modelData->diffuseTextures.empty())
	{
		if (modelData->diffuseTextures[0])
		{
			matRaw->albedoMap = modelData->diffuseTextures[0].get();
		}
	}

	// Entry を作って保持（ModelData の寿命は Entry が握る）
	auto entry = std::make_unique<ModelEntry>();
	entry->mesh = meshRaw;
	entry->material = matRaw;
	entry->SetModelData(std::move(modelData));

	ModelEntry* entryRaw = entry.get();
	this->modelTable.emplace(_key, std::move(entry));

	return entryRaw;
}

void ModelManager::Register(const std::string& _key, std::unique_ptr<Graphics::Import::ModelData> _model)
{
	if (!_model)
	{
		std::cerr << "[Error] ModelManager::Register: null model data: " << _key << std::endl;
		return;
	}

	// 既に登録済みなら何もしない（必要なら Unregister 後に呼ぶ）
	if (this->modelTable.contains(_key))
	{
		std::cerr << "[Info] ModelManager::Register: Key already exists: " << _key << std::endl;
		return;
	}

	// Mesh を生成して MeshManager に登録
	auto& meshManager = ResourceHub::Get<MeshManager>();

	auto meshUnique = meshManager.CreateFromModelData(*_model);
	if (!meshUnique)
	{
		std::cerr << "[Error] ModelManager::Register: CreateFromModelData failed: " << _key << std::endl;
		return;
	}

	Graphics::Mesh* meshRaw = meshUnique.get();
	meshManager.Register(_key, meshUnique.release());

	// Material を生成して MaterialManager に登録（当面 0 番のみ）
	auto& materialManager = ResourceHub::Get<MaterialManager>();

	const std::string matKey = MakeMaterialKey(_key, 0);
	Material* matRaw = materialManager.Register(matKey);

	if (matRaw && !_model->diffuseTextures.empty() && _model->diffuseTextures[0])
	{
		matRaw->albedoMap = _model->diffuseTextures[0].get();
	}

	// Entry を作って保持
	auto entry = std::make_unique<ModelEntry>();
	entry->mesh = meshRaw;
	entry->material = matRaw;
	entry->SetModelData(std::move(_model));

	this->modelTable.emplace(_key, std::move(entry));
}

void ModelManager::Unregister(const std::string& _key)
{
	auto it = this->modelTable.find(_key);
	if (it == this->modelTable.end())
	{
		return;
	}

	// 先に外部 Manager 側の登録を解除する
	auto& meshManager = ResourceHub::Get<MeshManager>();
	meshManager.Unregister(_key);

	auto& materialManager = ResourceHub::Get<MaterialManager>();
	materialManager.Unregister(MakeMaterialKey(_key, 0));

	// Entry を破棄（ModelData もここで破棄される）
	this->modelTable.erase(it);
}

ModelEntry* ModelManager::Get(const std::string& _key)
{
	auto it = this->modelTable.find(_key);
	if (it == this->modelTable.end())
	{
		return nullptr;
	}

	return it->second.get();
}

ModelEntry* ModelManager::Default() const
{
	return this->defaultModel.get();
}

void ModelManager::Clear()
{
	while (!this->modelTable.empty())
	{
		auto it = this->modelTable.begin();
		this->Unregister(it->first);
	}
}
