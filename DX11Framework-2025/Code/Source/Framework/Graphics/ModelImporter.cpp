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
	/** @brief テクスチャパスを textureDir と結合
	 *  @param _textureDir	テクスチャディレクトリ
	 *  @param _texPath		テクスチャパス
	 *  @return 結合されたテクスチャパス
	 */
	static std::string MakeTextureFullPath(const std::string& _textureDir, const std::string& _texPath)
	{
		if (_texPath.empty())
		{
			// 空のテクスチャパスの場合
			return "";
		}

		if (_texPath.size() >= 2 && _texPath[1] == ':')
		{
			// すでに絶対パスっぽい場合
			return _texPath;
		}

		if (_textureDir.empty())
		{
			// テクスチャディレクトリが空の場合
			return _texPath;
		}

		const char last = _textureDir.back();
		if (last == '/' || last == '\\')
		{
			// テクスチャディレクトリの末尾が / または \ の場合
			return _textureDir + _texPath;
		}

		// それ以外
		return _textureDir + "/" + _texPath;
	}

	/** @brief aiMaterial からマテリアル名を取得
	 *  @param _mat aiMaterial
	 */
	static std::string GetMaterialName(const aiMaterial* _mat)
	{
		if (!_mat)
		{
			// null ポインタの場合
			return "";
		}

		aiString name;
		if (_mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
		{
			// 取得成功
			return name.C_Str();
		}

		// 取得失敗
		return "";
	}

	/** @brief aiMaterial から色を取得（失敗時は既定値維持）
	 *  @param _mat		aiMaterial
	 *  @param _key		キー
	 *  @param _type	タイプ
	 *  @param _index	インデックス
	 *  @param _ioColor	出力先カラー
	 */
	static void TryGetColor(
		const aiMaterial* _mat,
		const char* _key,
		unsigned int _type,
		unsigned int _index,
		aiColor4D& _ioColor)
	{
		if (!_mat) { return; }

		// 色を取得
		aiColor4D c;
		if (aiGetMaterialColor(_mat, _key, _type, _index, &c) == AI_SUCCESS)
		{
			_ioColor = c;
		}
	}

	/** @brief aiMaterial から色を取得（失敗時は既定値維持）
	 *  @param _mat		aiMaterial
	 *  @param _keyColor	色キー
	 *  @param _ioColor	出力先カラー
	 */
	static void TryGetColor(const aiMaterial* _mat, const char* _keyColor, aiColor4D& _ioColor)
	{
		// aiGetMaterialColor は (key,type,index) が必要なのでラップする
		::TryGetColor(_mat, _keyColor, 0, 0, _ioColor);
	}

	/** @brief aiMaterial から shininess を取得
	 *  @param _mat			aiMaterial
	 *  @param _ioShininess	出力先 shininess
	 */
	static void TryGetShininess(const aiMaterial* _mat, float& _ioShininess)
	{
		if (!_mat) { return; }

		// shininess を取得
		float s = 0.0f;
		unsigned int max = 1;
		if (aiGetMaterialFloatArray(_mat, AI_MATKEY_SHININESS, &s, &max) == AI_SUCCESS)
		{
			_ioShininess = s;
		}
	}

	/** @brief Diffuse テクスチャパスを取得（最初の1枚）
	 *  @param _mat aiMaterial
	 */
	static std::string GetDiffuseTexturePath(const aiMaterial* _mat)
	{
		if (!_mat) { return ""; }

		if (_mat->GetTextureCount(aiTextureType_DIFFUSE) == 0)
		{
			// テクスチャが存在しない場合
			return "";
		}

		aiString path;
		if (_mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			//	取得成功
			return path.C_Str();
		}

		// 取得失敗
		return "";
	}

	/** @struct VertexInfluence
	 *  @brief 一時的に頂点ごとの影響（ボーンindexと重み）を保持する
	 */
	struct VertexInfluence
	{
		int boneIndex = -1;
		float weight = 0.0f;
		std::string boneName = "";
	};
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

	//-------------------------------------------------------------
	// Assimp シーンから Material / DiffuseTexture を構築する
	//-------------------------------------------------------------
	void ModelImporter::BuildMaterials(const aiScene* _scene, ModelData& _modelData, const std::string& _textureDir)const
	{
		assert(_scene != nullptr);
		assert(textureLoader != nullptr);

		// マテリアル情報を構築
		const unsigned int materialCount = _scene->mNumMaterials;

		_modelData.materials.clear();
		_modelData.diffuseTextures.clear();
		_modelData.materials.resize(materialCount);
		_modelData.diffuseTextures.resize(materialCount);

		//-------------------------------------------------------------
		// マテリアル情報を設定
		//-------------------------------------------------------------
		for (unsigned int i = 0; i < materialCount; i++)
		{
			const aiMaterial* mat = _scene->mMaterials[i];

			Material out{};
			out.materialName = GetMaterialName(mat);

			// 既定値
			out.ambient = aiColor4D(0, 0, 0, 1);
			out.diffuse = aiColor4D(1, 1, 1, 1);
			out.specular = aiColor4D(1, 1, 1, 1);
			out.emission = aiColor4D(0, 0, 0, 1);
			out.shiness = 0.0f;

			// 各種色を取得
			::TryGetColor(mat, AI_MATKEY_COLOR_AMBIENT, out.ambient);
			::TryGetColor(mat, AI_MATKEY_COLOR_DIFFUSE, out.diffuse);
			::TryGetColor(mat, AI_MATKEY_COLOR_SPECULAR, out.specular);
			::TryGetColor(mat, AI_MATKEY_COLOR_EMISSIVE, out.emission);
			::TryGetShininess(mat, out.shiness);

			//-------------------------------------------------------------
			// Diffuse テクスチャ読み込み
			//-------------------------------------------------------------
			out.diffuseTextureName = GetDiffuseTexturePath(mat);
			if (!out.diffuseTextureName.empty())
			{
				const std::string fullPath = MakeTextureFullPath(_textureDir, out.diffuseTextureName);

				// 既存方針：メンバ変数の TextureLoader を使用する
				_modelData.diffuseTextures[i] = textureLoader->FromFile(fullPath);
			}
			else
			{
				// テクスチャ無し
				_modelData.diffuseTextures[i] = nullptr;
			}

			_modelData.materials[i] = std::move(out);
		}
	}


	//-------------------------------------------------------------
	// メッシュごとの頂点配列とインデックス配列を構築する
	//-------------------------------------------------------------
	void ModelImporter::BuildMeshBuffers(const aiScene* _scene, ModelData& _modelData)const
	{
		assert(_scene != nullptr);

		// メッシュごとに頂点配列とインデックス配列を構築
		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.vertices.clear();
		_modelData.indices.clear();
		_modelData.vertices.resize(meshCount);
		_modelData.indices.resize(meshCount);

		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			// メッシュ取得
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			// メッシュ情報取得
			const std::string meshName = mesh->mName.C_Str();
			const unsigned int vCount = mesh->mNumVertices;

			// 頂点配列構築
			auto& outVerts = _modelData.vertices[meshIndex];
			outVerts.clear();
			outVerts.resize(vCount);

			// 頂点属性の有無を確認
			const bool hasNormals = (mesh->mNormals != nullptr);
			const bool hasColors0 = (mesh->mColors[0] != nullptr);
			const bool hasTex0 = (mesh->mTextureCoords[0] != nullptr);

			//-------------------------------------------------------------
			// マテリアル情報設定
			//-------------------------------------------------------------
			const int materialIndex = static_cast<int>(mesh->mMaterialIndex);
			std::string materialName = "";
			if (materialIndex >= 0 && static_cast<size_t>(materialIndex) < _modelData.materials.size())
			{
				materialName = _modelData.materials[materialIndex].materialName;
			}

			//-------------------------------------------------------------
			// 頂点情報設定
			//-------------------------------------------------------------
			for (unsigned int v = 0; v < vCount; v++)
			{
				Vertex vert{};
				vert.meshName = meshName;

				// 位置
				vert.pos = mesh->mVertices[v];

				// 法線
				if (hasNormals)
				{
					vert.normal = mesh->mNormals[v];
				}
				else
				{
					// 法線情報が無い場合は上向きに設定
					vert.normal = aiVector3D(0.0f, 1.0f, 0.0f);
				}

				// 頂点カラー
				if (hasColors0)
				{
					vert.color = mesh->mColors[0][v];
				}
				else
				{
					// 頂点カラー情報が無い場合は白に設定
					vert.color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}

				// テクスチャ座標
				if (hasTex0)
				{
					// テクスチャ座標は3Dベクトルで格納されているが、2Dテクスチャ座標として扱うためZ成分は0にする
					const aiVector3D uvw = mesh->mTextureCoords[0][v];
					vert.texCoord = aiVector3D(uvw.x, uvw.y, 0.0f);
				}
				else
				{
					// テクスチャ座標情報が無い場合は(0,0)に設定
					vert.texCoord = aiVector3D(0.0f, 0.0f, 0.0f);
				}

				// マテリアル情報を設定
				vert.materialIndex = materialIndex;
				vert.materialName = materialName;

				outVerts[v] = vert;
			}

			//-------------------------------------------------------------
			// インデックス情報の設定
			//-------------------------------------------------------------
			auto& outIdx = _modelData.indices[meshIndex];
			outIdx.clear();
			outIdx.reserve(mesh->mNumFaces * 3);

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];

				if (face.mNumIndices < 3)
				{
					// 3頂点未満のポリゴンは無視する
					continue;
				}

				// 三角形としてインデックスを追加
				outIdx.push_back(static_cast<unsigned int>(face.mIndices[0]));
				outIdx.push_back(static_cast<unsigned int>(face.mIndices[1]));
				outIdx.push_back(static_cast<unsigned int>(face.mIndices[2]));

				if (face.mNumIndices == 4)
				{
					// 四角形ポリゴンの場合、2つ目の三角形も追加する
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[0]));
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[2]));
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[3]));
				}
			}
		}
	}

	//-------------------------------------------------------------
	// メッシュ単位の Subset 情報を構築する
	//-------------------------------------------------------------
	void ModelImporter::BuildSubsets(const aiScene* _scene, ModelData& _modelData, bool _useUnifiedBuffers)const
	{
		assert(_scene != nullptr);

		// サブセット情報を構築
		_modelData.subsets.clear();
		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.subsets.reserve(meshCount);

		//-------------------------------------------------------------
		// サブセット情報の作成
		//-------------------------------------------------------------
		unsigned int vertexCursor = 0;
		unsigned int indexCursor = 0;
		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			// サブセット情報を構築
			Subset subset{};
			subset.meshName = mesh->mName.C_Str();
			subset.materialIndex = static_cast<int>(mesh->mMaterialIndex);

			// マテリアル名を設定
			if (subset.materialIndex >= 0 && static_cast<size_t>(subset.materialIndex) < _modelData.materials.size())
			{
				subset.materialName = _modelData.materials[subset.materialIndex].materialName;
			}
			else
			{
				// マテリアル情報が無い場合は空文字列を設定する
				subset.materialName = "";
			}

			// 頂点数・インデックス数を設定
			if (meshIndex < _modelData.vertices.size())
			{
				subset.vertexNum = static_cast<unsigned int>(_modelData.vertices[meshIndex].size());
			}
			else
			{
				// メッシュ情報が無い場合は0を設定する
				subset.vertexNum = 0;
			}

			// インデックス数を設定
			if (meshIndex < _modelData.indices.size())
			{
				subset.indexNum = static_cast<unsigned int>(_modelData.indices[meshIndex].size());
			}
			else
			{
				// メッシュ情報が無い場合は0を設定する
				subset.indexNum = 0;
			}

			// ベースインデックスを設定
			if (_useUnifiedBuffers)
			{
				subset.vertexBase = vertexCursor;
				subset.indexBase = indexCursor;

				vertexCursor += subset.vertexNum;
				indexCursor += subset.indexNum;
			}
			else
			{
				// メッシュごとのバッファを使用する場合は0を設定する
				subset.vertexBase = 0;
				subset.indexBase = 0;
			}

			_modelData.subsets.push_back(subset);
		}
	}

	//-------------------------------------------------------------
	// ボーン辞書（boneDictionary）と頂点の boneIndex/boneWeight を構築する
	//-------------------------------------------------------------
	void ModelImporter::BuildBonesAndSkinWeights(const aiScene* _scene, ModelData& _modelData)const
	{
		assert(_scene != nullptr);
		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.boneDictionary.clear();

		//-------------------------------------------------------------
		// 頂点のスキン情報を初期化（前のデータが残らないように）
		//-------------------------------------------------------------
		for (unsigned int m = 0; m < meshCount && m < _modelData.vertices.size(); m++)
		{
			for (auto& vtx : _modelData.vertices[m])
			{
				for (int i = 0; i < 4; i++)
				{
					vtx.boneIndex[i] = 0;
					vtx.boneWeight[i] = 0.0f;
					vtx.boneName[i].clear();
				}
				vtx.boneCount = 0;
			}
		}

		//-------------------------------------------------------------
		// 一時: meshごと、vertexごとに影響リストを貯める
		//-------------------------------------------------------------
		std::vector<std::vector<std::vector<VertexInfluence>>> influences;
		influences.resize(meshCount);
		for (unsigned int m = 0; m < meshCount; m++)
		{
			const aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh)
			{
				continue;
			}

			// メッシュの頂点数に合わせてリストを確保する
			const size_t vCount = (m < _modelData.vertices.size()) ? _modelData.vertices[m].size() : 0;
			influences[m].resize(vCount);
		}

		//-------------------------------------------------------------
		// ボーン収集 & 影響リストを作成
		//-------------------------------------------------------------
		int nextBoneIndex = 0;
		for (unsigned int m = 0; m < meshCount; m++)
		{
			const aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh)
			{
				continue;
			}

			const std::string meshName = mesh->mName.C_Str();

			// このメッシュの頂点数と ModelData 側が一致している前提で処理を行う
			if (m >= _modelData.vertices.size())
			{
				continue;
			}

			for (unsigned int b = 0; b < mesh->mNumBones; b++)
			{
				// ボーンを取得
				const aiBone* aiBonePtr = mesh->mBones[b];
				if (!aiBonePtr)
				{
					continue;
				}

				// ボーン名を取得
				const std::string boneName = aiBonePtr->mName.C_Str();

				// ボーン辞書に登録
				auto it = _modelData.boneDictionary.find(boneName);
				if (it == _modelData.boneDictionary.end())
				{
					Bone bone{};
					bone.boneName = boneName;
					bone.meshName = meshName;
					bone.armatureName = "";

					// ここは nodeTree 構築時に埋める想定
					bone.localBind = aiMatrix4x4();
					bone.globalBind = aiMatrix4x4();

					bone.animationLocal = aiMatrix4x4(); // 毎フレーム更新される想定
					bone.offsetMatrix = aiBonePtr->mOffsetMatrix;

					bone.index = nextBoneIndex;
					nextBoneIndex++;

					// 辞書に登録する
					auto inserted = _modelData.boneDictionary.emplace(boneName, std::move(bone));
					it = inserted.first;
				}
				else
				{
					// 既にある場合でも、このメッシュ名を上書きしたくないなら何もしない
					// it->second.meshName は最初に見つけた meshName のままになる
				}

				//-------------------------------------------------------------
				// ウェイト収集 & 影響リストへ追加
				//-------------------------------------------------------------
				Bone& bone = it->second;
				for (unsigned int w = 0; w < aiBonePtr->mNumWeights; w++)
				{
					const aiVertexWeight& vw = aiBonePtr->mWeights[w];

					// 頂点ID と 重み を取得
					const int vertexIndex = static_cast<int>(vw.mVertexId);
					const float weight = static_cast<float>(vw.mWeight);
					if (weight <= 0.0f)
					{
						// 重み0以下は無視する
						continue;
					}
					if (vertexIndex < 0 || static_cast<size_t>(vertexIndex) >= _modelData.vertices[m].size())
					{
						// 頂点インデックスが範囲外なら無視する
						continue;
					}

					// ボーンのウェイトリストへ追加
					Weight outW{};
					outW.boneName = boneName;
					outW.meshName = meshName;
					outW.weight = weight;
					outW.vertexIndex = vertexIndex;
					outW.meshIndex = static_cast<int>(m);
					bone.weights.push_back(outW);

					// 影響リストへ追加
					VertexInfluence inf{};
					inf.boneIndex = bone.index;
					inf.weight = weight;
					inf.boneName = boneName;

					// メッシュmの頂点vertexIndexに対する影響リストへ追加
					influences[m][static_cast<size_t>(vertexIndex)].push_back(std::move(inf));
				}
			}
		}

		//-------------------------------------------------------------
		// 各頂点の影響を「最大4本」に落として正規化して書き込む
		//-------------------------------------------------------------
		for (unsigned int m = 0; m < meshCount && m < _modelData.vertices.size(); m++)
		{
			// メッシュの頂点配列を取得
			auto& meshVerts = _modelData.vertices[m];
			for (size_t v = 0; v < meshVerts.size(); v++)
			{
				auto& list = influences[m][v];
				if (list.empty())
				{
					// 影響が無い頂点はスキップ
					continue;
				}

				// 重み降順で上位4本まで採用する
				std::sort(
					list.begin(),
					list.end(),
					[](const VertexInfluence& a, const VertexInfluence& b)
					{
						return a.weight > b.weight;
					});
				const size_t count = std::min<size_t>(4, list.size());

				// 合計を計算
				float sum = 0.0f;
				for (size_t i = 0; i < count; i++)
				{
					sum += list[i].weight;
				}

				if (sum <= 0.0f)
				{
					// 合計が0以下は無視する
					continue;
				}

				//-------------------------------------------------------------
				// 頂点データへ書き込み
				//-------------------------------------------------------------
				Vertex& outV = meshVerts[v];
				for (size_t i = 0; i < count; i++)
				{
					// ボーンインデックスと正規化した重みを設定する
					outV.boneIndex[i] = static_cast<UINT>(list[i].boneIndex);
					outV.boneWeight[i] = list[i].weight / sum;

					// デバッグ用：採用した上位4本だけ入れる
					outV.boneName[i] = list[i].boneName;
				}

				// 有効ボーン数を設定する
				outV.boneCount = static_cast<int>(count);
			}
		}
	}

	//-------------------------------------------------------------
	// ノードツリーを再帰的に構築する
	//-------------------------------------------------------------
	void ModelImporter::BuildNodeTreeRecursive(const aiNode* _aiNode, TreeNode<BoneNode>& _outNode)const
	{
		assert(_aiNode != nullptr);

		//　現在のノードを作成
		_outNode.nodedata.name = _aiNode->mName.C_Str();
		_outNode.nodedata.localBind = _aiNode->mTransformation;

		// 子ノード配列を初期化
		_outNode.children.clear();

		// 子ノードを再帰的に追加
		for (unsigned int i = 0; i < _aiNode->mNumChildren; i++)
		{
			const aiNode* child = _aiNode->mChildren[i];
			if (!child)
			{
				// 無効な子ノードは無視する
				continue;
			}

			// 子ノードを作成
			auto childNode = std::make_unique<TreeNode<BoneNode>>();

			// 再帰的に子ノードを構築して追加していく
			BuildNodeTreeRecursive(child, *childNode);

			// 親子関係を設定して追加する
			_outNode.Addchild(std::move(childNode));
		}
	}

	//-------------------------------------------------------------
	// ノードツリーを構築して ModelData に格納する
	//-------------------------------------------------------------
	void ModelImporter::BuildNodeTree(const aiScene* _scene, ModelData& _modelData)const
	{
		assert(_scene != nullptr);
		assert(_scene->mRootNode != nullptr);

		// ルートからツリーを構築する
		_modelData.nodeTree = TreeNode<BoneNode>();
		BuildNodeTreeRecursive(_scene->mRootNode, _modelData.nodeTree);
	}

	/** @brief モデルファイルを読み込み ModelData に変換
	 *  @param const std::string& _filename モデルファイルパス
	 *  @param const std::string& _textureDir テクスチャディレクトリ
	 *  @param ModelData& _model 出力先モデルデータ
	 *  @return 成功時 true
	 */
	bool ModelImporter::Load(const std::string& _filename, const std::string& _textureDir, ModelData& _model)
	{
		//-----------------------------------------------------------------------------
		// Assimpでモデルを読み込む
		//-----------------------------------------------------------------------------
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

		//-----------------------------------------------------------------------------
		// モデルデータ初期化
		//-----------------------------------------------------------------------------
		_model.vertices.clear();
		_model.indices.clear();
		_model.materials.clear();
		_model.diffuseTextures.clear();
		_model.subsets.clear();
		_model.boneDictionary.clear();

		//-----------------------------------------------------------------------------
		// マテリアルとテクスチャを作成
		//-----------------------------------------------------------------------------
		BuildMaterials(scene, _model, _textureDir);

		//-----------------------------------------------------------------------------
		// メッシュデータを作成
		//-----------------------------------------------------------------------------
		BuildMeshBuffers(scene, _model);

		//-----------------------------------------------------------------------------
		// サブセットを作成
		//-----------------------------------------------------------------------------
		BuildSubsets(scene, _model, false);

		//-----------------------------------------------------------------------------
		// スキニング情報を読み込む
		//-----------------------------------------------------------------------------
		BuildBonesAndSkinWeights(scene, _model);

		//-----------------------------------------------------------------------------
		// ノードツリーを作成
		//-----------------------------------------------------------------------------
		BuildNodeTree(scene, _model);

		return true;
	}
} // namespace Graphics::Import
