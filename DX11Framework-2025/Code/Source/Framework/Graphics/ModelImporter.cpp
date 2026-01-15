/** @file   ModelImporter.cpp
 *  @brief  Assimpを利用したモデルデータ読み込み
 */
#include "Include/Framework/Graphics/ModelImporter.h"
#include "Include/Framework/Utils/TreeNode.h"

#include <iostream>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <optional>

 //-----------------------------------------------------------------------------
 // Assimp
 //-----------------------------------------------------------------------------
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	/// @brief aiMatrix4x4 を行ベクトル運用向けに転置して返す
	static aiMatrix4x4 ToRowVectorMatrix(const aiMatrix4x4& _m)
	{
		aiMatrix4x4 out = _m;
		out.Transpose();
		return out;
	}

	/** @brief 頂点にボーン影響を追加（上位4本を維持） 
	 */
	void AddInfluence(Graphics::Import::Vertex& _v, UINT _boneIndex, float _weight)
	{
		if (_weight <= 0.0f) { return; }

		// 既に同じボーンが入っているなら加算
		for (int i = 0; i < 4; i++)
		{
			if (_v.boneWeight[i] > 0.0f && _v.boneIndex[i] == _boneIndex)
			{
				_v.boneWeight[i] += _weight;
				return;
			}
		}

		// 空き（weight==0）へ入れる
		for (int i = 0; i < 4; i++)
		{
			if (_v.boneWeight[i] <= 0.0f)
			{
				_v.boneIndex[i] = _boneIndex;
				_v.boneWeight[i] = _weight;
				return;
			}
		}

		// 空き無し：最小ウェイトより大きい場合だけ差し替え
		int minSlot = 0;
		float minW = _v.boneWeight[0];
		for (int i = 1; i < 4; i++)
		{
			if (_v.boneWeight[i] < minW)
			{
				minW = _v.boneWeight[i];
				minSlot = i;
			}
		}

		if (_weight > minW)
		{
			_v.boneIndex[minSlot] = _boneIndex;
			_v.boneWeight[minSlot] = _weight;
		}
	}

	/** @brief ウェイトを正規化し、boneCount を更新 
	 */
	void NormalizeInfluences(Graphics::Import::Vertex& _v)
	{
		float sum = 0.0f;
		for (int i = 0; i < 4; i++)
		{
			if (_v.boneWeight[i] > 0.0f)
			{
				sum += _v.boneWeight[i];
			}
		}

		if (sum > 0.0f)
		{
			const float inv = 1.0f / sum;

			int count = 0;
			for (int i = 0; i < 4; i++)
			{
				if (_v.boneWeight[i] > 0.0f)
				{
					_v.boneWeight[i] *= inv;
					count++;
				}
				else
				{
					_v.boneWeight[i] = 0.0f;
					_v.boneIndex[i] = 0; // 未使用
				}
			}
			_v.boneCount = count;
		}
		else
		{
			_v.boneCount = 0;
			for (int i = 0; i < 4; i++)
			{
				_v.boneIndex[i] = 0;
				_v.boneWeight[i] = 0.0f;
			}
		}
	}

	// ------------------------------------------------------------
	// Debug helpers
	// ------------------------------------------------------------
	static void DebugDumpOneVertexInfluencesOnce(const Graphics::Import::ModelData& _model)
	{
		static bool s_dumped = false;
		if (s_dumped) { return; }
		s_dumped = true;

		// 1頂点だけ（最初に見つかった頂点）を対象にする
		size_t meshIndex = 0;
		size_t vertexIndex = 0;
		const Graphics::Import::Vertex* vtx = nullptr;

		for (size_t mi = 0; mi < _model.vertices.size(); ++mi)
		{
			if (!_model.vertices[mi].empty())
			{
				meshIndex = mi;
				vertexIndex = 0;
				vtx = &_model.vertices[mi][0];
				break;
			}
		}

		if (!vtx)
		{
			std::cout << "[ModelImporter][Debug] vertices are empty.\n";
			return;
		}

		// index -> boneName を引けるように逆引き辞書を作る
		std::unordered_map<int, std::string> indexToName;
		indexToName.reserve(_model.boneDictionary.size());
		for (const auto& [name, bone] : _model.boneDictionary)
		{
			if (bone.index >= 0)
			{
				// 同じ index が複数名に割り当たっている場合もログで気づけるようにする
				if (indexToName.find(bone.index) == indexToName.end())
				{
					indexToName.emplace(bone.index, name);
				}
			}
		}

		std::cout << "[ModelImporter][Debug] ---- One Vertex Bone Influence Dump ----\n";
		std::cout << " meshIndex=" << meshIndex << " meshName='" << vtx->meshName << "' vertexIndex=" << vertexIndex << "\n";
		std::cout << " boneCount=" << vtx->boneCount << "\n";

		for (int i = 0; i < 4; ++i)
		{
			const int idx = static_cast<int>(vtx->boneIndex[i]);
			const float w = vtx->boneWeight[i];

			std::string resolvedName = "(unknown)";
			auto it = indexToName.find(idx);
			if (it != indexToName.end())
			{
				resolvedName = it->second;
			}

			std::cout
				<< "  slot[" << i << "] index=" << idx
				<< " weight=" << w
				<< " resolvedBoneName='" << resolvedName << "'"
				<< " vertexStoredBoneName='" << vtx->boneName[i] << "'\n";
		}

		std::cout << "[ModelImporter][Debug] -----------------------------------------\n";
		std::cout << "[ModelImporter][Debug] Please confirm whether resolvedBoneName matches the intended body part.\n";
	}
}

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

	//-----------------------------------------------------------------------------
	// ModelImporter::CreateNodeTree（この関数だけ置き換え推奨）
	//-----------------------------------------------------------------------------

	void ModelImporter::CreateNodeTree(aiNode* _node, TreeNode_t* _tree)
	{
		if (!_node || !_tree) { return; }

		BoneNode node{};
		node.name = (_node->mName.length > 0) ? _node->mName.C_Str() : "(UnnamedNode)";
		node.localBind = ToRowVectorMatrix(_node->mTransformation);

		_tree->nodedata = node;

		for (unsigned int n = 0; n < _node->mNumChildren; n++)
		{
			aiNode* child = _node->mChildren[n];
			if (!child) { continue; }

			auto childNode = std::make_unique<TreeNode_t>();
			_tree->Addchild(std::move(childNode));

			TreeNode_t* added = _tree->children.back().get();
			CreateNodeTree(child, added);
		}
	}

	//-----------------------------------------------------------------------------
	// ModelImporter::CreateEmptyBoneDictionary
	//-----------------------------------------------------------------------------

	void ModelImporter::CreateEmptyBoneDictionary(aiNode* _node, std::unordered_map<std::string, Bone>& _dict)
	{
		if (!_node) { return; }

		Bone bone{};
		bone.boneName = _node->mName.C_Str();
		bone.localBind = ToRowVectorMatrix(_node->mTransformation);

		bone.offsetMatrix = aiMatrix4x4();
		bone.globalBind = aiMatrix4x4();

		_dict[bone.boneName] = bone;

		for (unsigned int i = 0; i < _node->mNumChildren; i++)
		{
			CreateEmptyBoneDictionary(_node->mChildren[i], _dict);
		}
	}

	//-----------------------------------------------------------------------------
	// ModelImporter::SetBoneDataToVertices
	//-----------------------------------------------------------------------------
	void ModelImporter::SetBoneDataToVertices(ModelData& _model)
	{
		// メッシュ名 -> meshIndex（_model.vertices の添字）対応表を作る
		std::unordered_map<std::string, int> meshNameToIndex;
		meshNameToIndex.reserve(_model.subsets.size());
		for (size_t i = 0; i < _model.subsets.size(); i++)
		{
			meshNameToIndex.emplace(_model.subsets[i].meshName, static_cast<int>(i));
		}

		// 頂点初期化
		for (auto& meshVertices : _model.vertices)
		{
			for (auto& v : meshVertices)
			{
				v.boneCount = 0;
				for (int i = 0; i < 4; i++)
				{
					v.boneIndex[i] = 0;
					v.boneWeight[i] = 0.0f;
					v.boneName[i].clear();
				}
			}
		}

		// 影響を一旦入れる（上位4本を維持しながら）
		for (auto& [boneName, bone] : _model.boneDictionary)
		{
			if (bone.index < 0) { continue; }

			for (const auto& w : bone.weights)
			{
				auto itMesh = meshNameToIndex.find(w.meshName);
				if (itMesh == meshNameToIndex.end()) { continue; }

				const int meshIndex = itMesh->second;
				if (meshIndex < 0 || meshIndex >= static_cast<int>(_model.vertices.size())) { continue; }
				if (w.vertexIndex < 0 || w.vertexIndex >= static_cast<int>(_model.vertices[meshIndex].size())) { continue; }

				auto& v = _model.vertices[meshIndex][w.vertexIndex];

				AddInfluence(v, static_cast<UINT>(bone.index), w.weight);

				// デバッグ用途として boneName も入れておく（同じスロットに対応付けたい場合）
				for (int i = 0; i < 4; i++)
				{
					if (v.boneIndex[i] == static_cast<UINT>(bone.index))
					{
						v.boneName[i] = boneName;
					}
				}
			}
		}

		// 正規化して boneCount を確定
		for (auto& meshVertices : _model.vertices)
		{
			for (auto& v : meshVertices)
			{
				NormalizeInfluences(v);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Bone helper
	//-----------------------------------------------------------------------------

	void ModelImporter::AssignBoneIndicesFromTree(const Utils::TreeNode<BoneNode>& _node, unsigned int& _idx, std::unordered_map<std::string, Bone>& _dict)
	{
		const std::string& name = _node.nodedata.name;

		auto it = _dict.find(name);
		if (it != _dict.end())
		{
			it->second.index = static_cast<int>(_idx);
			_idx++;
		}

		for (const auto& child : _node.children)
		{
			AssignBoneIndicesFromTree(*child, _idx, _dict);
		}
	}

	void ModelImporter::BuildGlobalBindMatrices(const Utils::TreeNode<BoneNode>& _node, const aiMatrix4x4& _parent, std::unordered_map<std::string, Bone>& _dict)
	{
		const BoneNode& data = _node.nodedata;

		// Row-vector convention: global = parent * local
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

	//-----------------------------------------------------------------------------
	// GetBone
	//-----------------------------------------------------------------------------

	void ModelImporter::GetBone(const aiScene* _scene, ModelData& _model)
	{
		if (!_scene || !_scene->mRootNode) { return; }

		// 辞書とツリーを先に作る（初期ローカル行列を確保する）
		_model.boneDictionary.clear();

		_model.boneTree = TreeNode_t{};
		CreateNodeTree(_scene->mRootNode, &_model.boneTree);

		CreateEmptyBoneDictionary(_scene->mRootNode, _model.boneDictionary);

		// メッシュ側の bone 情報を辞書へ
		for (unsigned int m = 0; m < _scene->mNumMeshes; m++)
		{
			const aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh) { continue; }

			const std::string meshName = (mesh->mName.length > 0) ? mesh->mName.C_Str() : "";

			for (unsigned int bidx = 0; bidx < mesh->mNumBones; bidx++)
			{
				const aiBone* aiBonePtr = mesh->mBones[bidx];
				if (!aiBonePtr) { continue; }

				const std::string boneName = aiBonePtr->mName.C_Str();

				auto it = _model.boneDictionary.find(boneName);
				if (it == _model.boneDictionary.end())
				{
					Bone newBone{};
					newBone.boneName = boneName;
					_model.boneDictionary.emplace(boneName, newBone);
					it = _model.boneDictionary.find(boneName);
				}
				Bone& dst = it->second;

				dst.boneName = boneName;

				// オフセット行列（inverse bind）は row-vector 規約に合わせて転置して保持する。
				// CPU 側は row-vectorで運用し、GPU 送信時に transpose する前提。
				dst.offsetMatrix = ToRowVectorMatrix(aiBonePtr->mOffsetMatrix);

				// 頂点反映は Weight::meshName を使う+
				dst.meshName = meshName;

				if (aiBonePtr->mArmature)
				{
					dst.armatureName = aiBonePtr->mArmature->mName.C_Str();
				}

				for (unsigned int widx = 0; widx < aiBonePtr->mNumWeights; widx++)
				{
					Weight w{};
					w.meshName = meshName;
					w.boneName = dst.boneName;
					w.weight = aiBonePtr->mWeights[widx].mWeight;
					w.vertexIndex = aiBonePtr->mWeights[widx].mVertexId;
					dst.weights.emplace_back(w);
				}
			}
		}

		// boneTree を走査して index を安定順で振る
		unsigned int idx = 0;
		AssignBoneIndicesFromTree(_model.boneTree, idx, _model.boneDictionary);

		// 頂点へ index/weight を反映（index が確定してから）
		SetBoneDataToVertices(_model);

		// デバッグ: 1頂点分だけ boneIndex/boneWeight と対応ボーン名を出す（1回だけ）
		DebugDumpOneVertexInfluencesOnce(_model);

		// 初期グローバル行列（Bind姿勢の global）を計算
		aiMatrix4x4 identity;
		identity = aiMatrix4x4();
		BuildGlobalBindMatrices(_model.boneTree, identity, _model.boneDictionary);
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
