/** @file   ModelImporter.cpp
 *  @brief  Assimpを利用したモデルデータ読み込み
 */
#include "Include/Framework/Graphics/ModelImporter.h"
#include "Include/Framework/Utils/TreeNode.h"

#include <iostream>
#include <cassert>
#include <unordered_map>
#include <memory>

 //-----------------------------------------------------------------------------
 // Assimp
 //-----------------------------------------------------------------------------
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	using Utils::TreeNode;

	/// @brief コンストラクタ
	ModelImporter::ModelImporter()
	{
		textureLoader = std::make_unique<TextureLoader>();
	}

	/// @brief デストラクタ
	ModelImporter::~ModelImporter()
	{
		textureLoader.reset();
	}

	/** @brief ノード階層を再帰的に作成
	 *  @param aiNode* _node 元ノード
	 *  @param TreeNode_t* _tree 生成先ツリー
	 */
	void ModelImporter::CreateNodeTree(aiNode* _node, TreeNode_t* _tree)
	{
		if (!_node || !_tree) { return; }

		// ノードデータを作成
		BoneNode node{};
		node.name = (_node->mName.length > 0) ? _node->mName.C_Str() : "(UnnamedNode)";
		node.localBind = _node->mTransformation;

		_tree->nodedata = node;

		for (unsigned int n = 0; n < _node->mNumChildren; n++)
		{
			// 子ノード取得
			aiNode* child = _node->mChildren[n];
			if (!child) { continue; }

			// 子ノードデータを作成
			BoneNode childData{};
			childData.name = (child->mName.length > 0) ? child->mName.C_Str() : "(UnnamedChild)";
			childData.localBind = child->mTransformation;

			// 子ノードを作成して追加
			auto childNode = std::make_unique<TreeNode_t>(childData);
			_tree->Addchild(std::move(childNode));

			// 再帰的に子ノードを作成
			TreeNode_t* added = _tree->children.back().get();
			CreateNodeTree(child, added);
		}
	}

	/** @brief 空のボーン辞書を作成
	 *  @param aiNode* _node 対象ノード
	 *  @param std::unordered_map<std::string, Bone>& _dict ボーン辞書
	 */
	void ModelImporter::CreateEmptyBoneDictionary(aiNode* _node, std::unordered_map<std::string, Bone>& _dict)
	{
		if (!_node) return;

		Bone bone{};
		bone.boneName = _node->mName.C_Str();
		bone.localBind = _node->mTransformation;

		_dict[bone.boneName] = bone;

		for (unsigned int i = 0; i < _node->mNumChildren; i++)
		{
			CreateEmptyBoneDictionary(_node->mChildren[i], _dict);
		}
	}

	/** @brief メッシュからボーン情報を取得
	 *  @param const aiMesh* _mesh メッシュデータ
	 *  @param std::unordered_map<std::string, Bone>& _dict ボーン辞書
	 *  @return std::vector<Bone> 取得したボーン配列
	 */
	std::vector<Bone> ModelImporter::GetBonesPerMesh(const aiMesh* _mesh, std::unordered_map<std::string, Bone>& _dict)
	{
		std::vector<Bone> bones;

		for (unsigned int bidx = 0; bidx < _mesh->mNumBones; bidx++)
		{
			Bone bone{};
			bone.boneName = _mesh->mBones[bidx]->mName.C_Str();
			bone.meshName = _mesh->mBones[bidx]->mNode->mName.C_Str();

			if (_mesh->mBones[bidx]->mArmature)
			{
				bone.armatureName = _mesh->mBones[bidx]->mArmature->mName.C_Str();
			}

			bone.offsetMatrix = _mesh->mBones[bidx]->mOffsetMatrix;

			for (unsigned int widx = 0; widx < _mesh->mBones[bidx]->mNumWeights; widx++)
			{
				Weight w{};
				w.meshName = bone.meshName;
				w.boneName = bone.boneName;
				w.weight = _mesh->mBones[bidx]->mWeights[widx].mWeight;
				w.vertexIndex = _mesh->mBones[bidx]->mWeights[widx].mVertexId;
				bone.weights.emplace_back(w);
			}

			bones.emplace_back(bone);
			_dict[bone.boneName].offsetMatrix = bone.offsetMatrix;
		}

		return bones;
	}

	/** @brief ボーンデータを頂点へ設定
	 *  @param ModelData& _model 対象モデル
	 */
	void ModelImporter::SetBoneDataToVertices(ModelData& _model)
	{
		// 頂点初期化
		for (auto& meshVertices : _model.vertices)
		{
			for (auto& v : meshVertices)
			{
				v.boneCount = 0;
				for (int i = 0; i < 4; i++)
				{
					v.boneIndex[i] = -1;
					v.boneWeight[i] = 0.0f;
					v.boneName[i].clear();
				}
			}
		}

		// 頂点反映
		for (auto& [boneName, bone] : _model.boneDictionary)
		{
			int meshIndex = -1;
			for (size_t i = 0; i < _model.subsets.size(); i++)
			{
				if (_model.subsets[i].meshName == bone.meshName)
				{
					meshIndex = static_cast<int>(i);
					break;
				}
			}
			if (meshIndex < 0) { continue; }

			for (auto& w : bone.weights)
			{
				auto& v = _model.vertices[meshIndex][w.vertexIndex];
				int& idx = v.boneCount;
				if (idx < 4)
				{
					v.boneName[idx] = w.boneName;
					v.boneWeight[idx] = w.weight;
					v.boneIndex[idx] = bone.index;
					idx++;
				}
			}
		}
	}

	/** @brief シーン全体からボーン情報を取得
	 *  @param const aiScene* _scene シーンデータ
	 *  @param ModelData& _model モデルデータ
	 */
	void ModelImporter::GetBone(const aiScene* _scene, ModelData& _model)
	{
		if (!_scene || !_scene->mRootNode) { return; }

		// 空のボーン辞書を作成する
		CreateEmptyBoneDictionary(_scene->mRootNode, _model.boneDictionary);

		// ボーン情報を取得して辞書に格納する
		unsigned int idx = 0;
		for (auto& [name, bone] : _model.boneDictionary)
		{
			bone.index = idx++;
		}

		// メッシュごとにボーン情報を取得する
		for (unsigned int m = 0; m < _scene->mNumMeshes; m++)
		{
			aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh) { continue; }

			auto bones = GetBonesPerMesh(mesh, _model.boneDictionary);
			for (auto& b : bones)
			{
				_model.boneDictionary[b.boneName] = b;
			}
		}

		// 頂点にボーンデータを設定する
		SetBoneDataToVertices(_model);

		// ノードツリーを作成
		_model.boneTree = TreeNode_t{};
		CreateNodeTree(_scene->mRootNode, &_model.boneTree);

		// 初期グローバル行列を計算
		BuildGlobalBindMatrices(
			_model.boneTree,
			aiMatrix4x4(),
			_model.boneDictionary
		);
	}

	/** @brief マテリアルとテクスチャを取得
	 *  @param const aiScene* _scene Assimpシーン
	 *  @param const std::string& _textureDir テクスチャディレクトリ
	 *  @param ModelData& _model モデルデータ
	 */
	void ModelImporter::GetMaterialData(const aiScene* _scene, const std::string& _textureDir, ModelData& _model)
	{
		if (!_scene) return;

		// マテリアルとテクスチャの初期化
		_model.materials.clear();
		_model.diffuseTextures.resize(_scene->mNumMaterials);

		// マテリアルごとに情報を取得
		for (unsigned int m = 0; m < _scene->mNumMaterials; m++)
		{
			aiMaterial* material = _scene->mMaterials[m];

			Material mat{};
			mat.materialName = material->GetName().C_Str();

			aiColor4D ambient{}, diffuse{}, specular{}, emission{};
			float shiness = 0.0f;

			aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission);
			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shiness);

			mat.ambient = ambient;
			mat.diffuse = diffuse;
			mat.specular = specular;
			mat.emission = emission;
			mat.shiness = shiness;

			std::vector<std::string> texPaths{};

			// ディフューズテクスチャの取得
			for (unsigned int t = 0; t < material->GetTextureCount(aiTextureType_DIFFUSE); t++)
			{
				aiString path;
				if (AI_SUCCESS == material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, t), path))
				{
					std::string texPath = path.C_Str();
					texPaths.push_back(texPath);

					// テクスチャの読み込み
					if (auto tex = _scene->GetEmbeddedTexture(path.C_Str()))
					{
						auto texture = textureLoader->FromMemory(
							(unsigned char*)tex->pcData,
							tex->mWidth
						);
						if (texture) _model.diffuseTextures[m] = std::move(texture);
					}
					// ファイルから読み込み
					else
					{
						std::string fullPath = _textureDir + "/" + texPath;
						std::cout << fullPath.c_str() << "\n";

						auto texture = textureLoader->FromFile(fullPath);
						if (texture)
							_model.diffuseTextures[m] = std::move(texture);
					}
				}
			}

			// マテリアルにテクスチャ名を設定する
			mat.diffuseTextureName = texPaths.empty() ? "" : texPaths[0];
			_model.materials.push_back(mat);
		}
	}

	void ModelImporter::BuildGlobalBindMatrices(const Utils::TreeNode<BoneNode>& _node, const aiMatrix4x4& _parent, std::unordered_map<std::string, Bone>& _dict)
	{
		const BoneNode& data = _node.nodedata;

		aiMatrix4x4 global = _parent * data.localBind;

		auto it = _dict.find(data.name);
		if (it != _dict.end())
		{
			it->second.globalBind = global;
		}

		for (const auto& child : _node.children)
		{
			BuildGlobalBindMatrices(*child, global, _dict);
		}
	}

	/** @brief モデルファイルを読み込み ModelData に変換
	 *  @param const std::string& _filename モデルファイルパス
	 *  @param const std::string& _textureDir テクスチャディレクトリ
	 *  @param ModelData& _model 出力先モデルデータ
	 *  @return 成功時 true
	 */
	bool ModelImporter::Load(const std::string& _filename, const std::string& _textureDir, ModelData& _model)
	{
		// モデルファイルの読み込み
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			_filename,
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_ConvertToLeftHanded |
			aiProcess_PopulateArmatureData 
		);

		if (!scene)
		{
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return false;
		}

		// モデルデータの初期化
		_model.vertices.clear();
		_model.indices.clear();
		_model.materials.clear();
		_model.diffuseTextures.clear();
		_model.subsets.clear();
		_model.boneDictionary.clear();

		// マテリアルとテクスチャの取得
		GetMaterialData(scene, _textureDir, _model);

		// メッシュデータの取得
		_model.vertices.resize(scene->mNumMeshes);
		_model.indices.resize(scene->mNumMeshes);
		_model.subsets.resize(scene->mNumMeshes);

		unsigned int globalVertexBase = 0;
		unsigned int globalIndexBase = 0;

		// メッシュごとに頂点・インデックス・サブセット情報を取得する
		for (unsigned int m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];
			std::string meshName = mesh->mName.C_Str();

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				Vertex vert{};
				vert.meshName = meshName;
				vert.pos = mesh->mVertices[v];
				vert.normal = mesh->HasNormals() ? mesh->mNormals[v] : aiVector3D(0, 0, 0);
				vert.color = mesh->HasVertexColors(0) ? mesh->mColors[0][v] : aiColor4D(1, 1, 1, 1);
				vert.texCoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][v] : aiVector3D(0, 0, 0);
				vert.materialIndex = mesh->mMaterialIndex;
				vert.materialName = _model.materials[vert.materialIndex].materialName;
				_model.vertices[m].push_back(vert);
			}

			// インデックス情報の取得
			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];
				for (unsigned int i = 0; i < face.mNumIndices; i++)
				{
					_model.indices[m].push_back(face.mIndices[i]);
				}
			}

			// サブセット情報の取得
			Subset subset{};
			subset.meshName = meshName;
			subset.materialIndex = mesh->mMaterialIndex;
			subset.materialName = _model.materials[subset.materialIndex].materialName;
			subset.vertexNum = static_cast<unsigned int>(_model.vertices[m].size());
			subset.indexNum = static_cast<unsigned int>(_model.indices[m].size());
			subset.vertexBase = globalVertexBase;
			subset.indexBase = globalIndexBase;

			globalVertexBase += subset.vertexNum;
			globalIndexBase += subset.indexNum;

			_model.subsets[m] = subset;
		}

		// ボーン情報の取得
		GetBone(scene, _model);

		for (unsigned int m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];
			unsigned int maxIndex = 0;

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];
				for (unsigned int i = 0; i < face.mNumIndices; i++)
				{
					if (face.mIndices[i] > maxIndex)
					{
						maxIndex = face.mIndices[i];
					}
				}
			}

			std::cout << "[Mesh " << m << "] " << mesh->mName.C_Str()
				<< " | Vertices: " << mesh->mNumVertices
				<< " | MaxIndex: " << maxIndex
				<< " | Faces: " << mesh->mNumFaces << std::endl;
		}

		return true;
	}
} // namespace Graphics::Import