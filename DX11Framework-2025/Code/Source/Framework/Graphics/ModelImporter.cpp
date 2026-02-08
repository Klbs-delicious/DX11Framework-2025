/** @file	ModelImporter.cpp
 *  @brief	Assimpを利用したモデルデータ読み込み
 *
 *  このファイルは Assimp を用いてファイルからメッシュ・マテリアル・ボーン等の
 *  モデルデータを読み込み、独自の ModelData / SkeletonCache 構造に変換する処理を提供します。
 *
 *  主な責務:
 *   - マテリアルとテクスチャの読み込み
 *   - 頂点バッファおよびインデックスバッファの構築
 *   - サブセット情報の作成
 *   - ボーン辞書と頂点ウェイトの収集と正規化
 *   - ノード階層からのツリーノード構築
 *   - スケルトンキャッシュ（実行時で使う骨->ノード割り当てなど）の構築
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/ModelImporter.h"
#include "Include/Framework/Utils/TreeNode.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

//-----------------------------------------------------------------------------
// Assimp
//-----------------------------------------------------------------------------
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

//-----------------------------------------------------------------------------
// DirectXMath
//-----------------------------------------------------------------------------
#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	//-----------------------------------------------------------------------------
	// Debug switches (最小ログ用)
	//-----------------------------------------------------------------------------
	static constexpr bool EnableSkinningDebugLog = true;

	static constexpr float BindPoseWarnThreshold = 1.0e-3f;      ///< bind pose の誤差しきい値
	static constexpr int MaxBindPoseWarnBones = 6;              ///< warn 出力する最大件数
	static constexpr int MaxProbePrintBones = 6;               ///< probe 詳細出力の最大件数

	//-----------------------------------------------------------------------------
	// Assimp matrix helpers
	//-----------------------------------------------------------------------------
	/** @brief Assimp の aiMatrix4x4 を DX::Matrix4x4 に変換（転置なし）
	 *  @param _aiMatrix 入力行列
	 *  @return 変換後行列
	 */
	static DX::Matrix4x4 ConvertAiMatrixToDxMatrix_NoTranspose(const aiMatrix4x4& _aiMatrix)
	{
		return DX::Matrix4x4(
			_aiMatrix.a1, _aiMatrix.a2, _aiMatrix.a3, _aiMatrix.a4,
			_aiMatrix.b1, _aiMatrix.b2, _aiMatrix.b3, _aiMatrix.b4,
			_aiMatrix.c1, _aiMatrix.c2, _aiMatrix.c3, _aiMatrix.c4,
			_aiMatrix.d1, _aiMatrix.d2, _aiMatrix.d3, _aiMatrix.d4
		);
	}

	/** @brief Assimp の aiMatrix4x4 を DX::Matrix4x4 に変換（転置あり）
	 *  @param _aiMatrix 入力行列
	 *  @return 変換後行列（転置済み）
	 */
	static DX::Matrix4x4 ConvertAiMatrixToDxMatrix_Transpose(const aiMatrix4x4& _aiMatrix)
	{
		// Assimp の行列レイアウトと DirectX の扱いの違いに備えて明示的に転置して取り扱う
		return DX::TransposeMatrix(::ConvertAiMatrixToDxMatrix_NoTranspose(_aiMatrix));
	}

	//-----------------------------------------------------------------------------
	// Debug math helpers
	//-----------------------------------------------------------------------------
	/** @brief 必要なら転置する（デバッグ用）
	 *  @param _matrix 入力行列
	 *  @param _needTranspose true の場合のみ転置
	 *  @return 結果
	 */
	static DX::Matrix4x4 TransposeIfNeeded(const DX::Matrix4x4& _matrix, bool _needTranspose)
	{
		if (!_needTranspose)
		{
			return _matrix;
		}

		return DX::TransposeMatrix(_matrix);
	}

	/** @brief 恒等行列との差の最大絶対値を返す（デバッグ用）
	 *  @param _matrix 入力行列
	 *  @return 最大絶対誤差
	 */
	static float MaxAbsDiffFromIdentity(const DX::Matrix4x4& _matrix)
	{
		const float identity[16] =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		const float* p = reinterpret_cast<const float*>(&_matrix);

		float maxAbs = 0.0f;

		for (int i = 0; i < 16; i++)
		{
			const float d = std::fabs(p[i] - identity[i]);
			if (d > maxAbs)
			{
				maxAbs = d;
			}
		}

		return maxAbs;
	}

	//-----------------------------------------------------------------------------
	// Logging helpers (最小限)
	//-----------------------------------------------------------------------------
	/** @brief セクション見出しを出力する（デバッグ用）
	 *  @param _title 見出し文字列
	 */
	static void LogHeader(const char* _title)
	{
		if (!EnableSkinningDebugLog)
		{
			return;
		}

		std::cout << "\n";
		std::cout << "============================================================\n";
		std::cout << _title << "\n";
		std::cout << "============================================================\n";
	}

	/** @brief 4x4行列を読みやすい形で出力する（デバッグ用）
	 *  @param _label ラベル
	 *  @param _matrix 行列
	 */
	static void PrintMatrix4x4(const char* _label, const DX::Matrix4x4& _matrix)
	{
		if (!EnableSkinningDebugLog)
		{
			return;
		}

		std::cout << _label << std::endl;

		std::cout
			<< "  [r0] " << _matrix.m[0][0] << ", " << _matrix.m[0][1] << ", " << _matrix.m[0][2] << ", " << _matrix.m[0][3] << "\n"
			<< "  [r1] " << _matrix.m[1][0] << ", " << _matrix.m[1][1] << ", " << _matrix.m[1][2] << ", " << _matrix.m[1][3] << "\n"
			<< "  [r2] " << _matrix.m[2][0] << ", " << _matrix.m[2][1] << ", " << _matrix.m[2][2] << ", " << _matrix.m[2][3] << "\n"
			<< "  [r3] " << _matrix.m[3][0] << ", " << _matrix.m[3][1] << ", " << _matrix.m[3][2] << ", " << _matrix.m[3][3] << std::endl;
	}

	//-----------------------------------------------------------------------------
	// Minimal debug: bind pose identity の転置パターン総当たり
	//-----------------------------------------------------------------------------
	/** @brief bind pose の identity 成否を「転置の有無 8通り」で総当たりして、最小誤差の組み合わせを出す
	 *  @param _boneIndex ボーンインデックス
	 *  @param _boneName ボーン名
	 *  @param _bindGlobalBone ボーンの bind global
	 *  @param _globalInverseMeshRoot meshRoot の globalInverse
	 *  @param _boneOffset ボーン offset
	 */
	static void DebugBindPoseIdentityTransposeProbe(
		int _boneIndex,
		const std::string& _boneName,
		const DX::Matrix4x4& _bindGlobalBone,
		const DX::Matrix4x4& _globalInverseMeshRoot,
		const DX::Matrix4x4& _boneOffset)
	{
		if (!EnableSkinningDebugLog)
		{
			return;
		}

		float bestError = FLT_MAX;
		int bestMask = 0;
		DX::Matrix4x4 bestSkinMatrix = DX::Matrix4x4::Identity;

		// 3ビットのフラグで bind/globalInv/offset の転置を切り替え、最小の誤差を探す
		for (int transposeMask = 0; transposeMask < 8; ++transposeMask)
		{
			const bool transposeBindGlobal = ((transposeMask & 1) != 0);
			const bool transposeGlobalInv = ((transposeMask & 2) != 0);
			const bool transposeOffset = ((transposeMask & 4) != 0);

			const DX::Matrix4x4 bindGlobal = ::TransposeIfNeeded(_bindGlobalBone, transposeBindGlobal);
			const DX::Matrix4x4 globalInv = ::TransposeIfNeeded(_globalInverseMeshRoot, transposeGlobalInv);
			const DX::Matrix4x4 offset = ::TransposeIfNeeded(_boneOffset, transposeOffset);

			// row vector 前提：skin = offset * bindGlobal(bone) * globalInverse(meshRoot)
			const DX::Matrix4x4 skin = offset * bindGlobal * globalInv;

			const float error = ::MaxAbsDiffFromIdentity(skin);
			if (error < bestError)
			{
				bestError = error;
				bestMask = transposeMask;
				bestSkinMatrix = skin;
			}
		}

		const bool bestTransposeBindGlobal = ((bestMask & 1) != 0);
		const bool bestTransposeGlobalInv = ((bestMask & 2) != 0);
		const bool bestTransposeOffset = ((bestMask & 4) != 0);

		std::cout
			<< "[BindPoseTransposeProbe] boneIndex=" << _boneIndex
			<< " boneName=\"" << _boneName << "\""
			<< " bestMaxAbs=" << bestError
			<< " bestTranspose={ bindGlobal:" << (bestTransposeBindGlobal ? "T" : "N")
			<< ", globalInv:" << (bestTransposeGlobalInv ? "T" : "N")
			<< ", offset:" << (bestTransposeOffset ? "T" : "N")
			<< " }"
			<< std::endl;

		if (bestError > 1e-3f)
		{
			::PrintMatrix4x4("  bestSkin", bestSkinMatrix);
		}
	}

	//-----------------------------------------------------------------------------
	// Material helpers
	//-----------------------------------------------------------------------------
	/** @brief テクスチャパスをディレクトリと結合してフルパス化する
	 *  @param _textureDir テクスチャディレクトリ
	 *  @param _texPath モデルに書かれているテクスチャパス
	 *  @return フルパス
	 */
	static std::string MakeTextureFullPath(const std::string& _textureDir, const std::string& _texPath)
	{
		if (_texPath.empty())
		{
			return "";
		}

		// 絶対パス（Windows）かを判定
		if (_texPath.size() >= 2 && _texPath[1] == ':')
		{
			return _texPath;
		}

		if (_textureDir.empty())
		{
			return _texPath;
		}

		const char last = _textureDir.back();
		if (last == '/' || last == '\\')
		{
			return _textureDir + _texPath;
		}

		return _textureDir + "/" + _texPath;
	}

	/// @brief マテリアル名を取得する
	static std::string GetMaterialName(const aiMaterial* _mat)
	{
		if (!_mat)
		{
			return "";
		}

		aiString name;
		if (_mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
		{
			return name.C_Str();
		}

		return "";
	}

	/** @brief マテリアルのカラーを取得する（取得できた場合のみ上書き）
	 *  @param _mat マテリアル
	 *  @param _key キー
	 *  @param _type 種別
	 *  @param _index インデックス
	 *  @param _ioColor 入出力カラー
	 */
	static void TryGetColor(
		const aiMaterial* _mat,
		const char* _key,
		unsigned int _type,
		unsigned int _index,
		aiColor4D& _ioColor)
	{
		if (!_mat)
		{
			return;
		}

		aiColor4D c;
		if (aiGetMaterialColor(_mat, _key, _type, _index, &c) == AI_SUCCESS)
		{
			_ioColor = c;
		}
	}

	/** @brief マテリアルのカラーを取得する（簡易版）
	 *  @param _mat マテリアル
	 *  @param _keyColor キー
	 *  @param _ioColor 入出力カラー
	 */
	static void TryGetColor(const aiMaterial* _mat, const char* _keyColor, aiColor4D& _ioColor)
	{
		::TryGetColor(_mat, _keyColor, 0, 0, _ioColor);
	}

	/** @brief 光沢を取得する（取得できた場合のみ上書き）
	 *  @param _mat マテリアル
	 *  @param _ioShininess 入出力
	 */
	static void TryGetShininess(const aiMaterial* _mat, float& _ioShininess)
	{
		if (!_mat)
		{
			return;
		}

		float shininess = 0.0f;
		unsigned int maxCount = 1;

		if (aiGetMaterialFloatArray(_mat, AI_MATKEY_SHININESS, &shininess, &maxCount) == AI_SUCCESS)
		{
			_ioShininess = shininess;
		}
	}

	/// @brief diffuse テクスチャパスを取得する
	static std::string GetDiffuseTexturePath(const aiMaterial* _mat)
	{
		if (!_mat)
		{
			return "";
		}

		if (_mat->GetTextureCount(aiTextureType_DIFFUSE) == 0)
		{
			return "";
		}

		aiString path;
		if (_mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			return path.C_Str();
		}

		return "";
	}

	//-----------------------------------------------------------------------------
	// Skeleton cache building helpers
	//-----------------------------------------------------------------------------
	/** @brief ノード階層を一次元配列に展開して、親子関係と bindLocalMatrix を保持する
	 *  @param _node Assimp ノード
	 *  @param _parentIndex 親ノードインデックス
	 *  @param _outNodes 出力ノード配列
	 *  @param _outOrder トポロジ順（親→子）に並ぶインデックス配列
	 *  @param _outNodeNameToIndex 名前→ノードインデックスの辞書
	 */
	static void BuildSkeletonNodesRecursive(
		const aiNode* _node,
		int _parentIndex,
		std::vector<Graphics::Import::SkeletonNodeCache>& _outNodes,
		std::vector<int>& _outOrder,
		std::unordered_map<std::string, int>& _outNodeNameToIndex)
	{
		if (!_node)
		{
			return;
		}

		const int nodeIndex = static_cast<int>(_outNodes.size());

		Graphics::Import::SkeletonNodeCache nodeCache{};
		nodeCache.name = _node->mName.C_Str();
		nodeCache.parentIndex = _parentIndex;
		// Assimp の行列はここで転置して Model 側と一致させて保持する設計
		nodeCache.bindLocalMatrix = ::ConvertAiMatrixToDxMatrix_Transpose(_node->mTransformation);
		nodeCache.hasMesh = (_node->mNumMeshes > 0);
		nodeCache.boneIndex = -1;

		_outNodes.push_back(nodeCache);
		_outOrder.push_back(nodeIndex);
		_outNodeNameToIndex.emplace(nodeCache.name, nodeIndex);

		for (unsigned int childIndex = 0; childIndex < _node->mNumChildren; childIndex++)
		{
			BuildSkeletonNodesRecursive(_node->mChildren[childIndex], nodeIndex, _outNodes, _outOrder, _outNodeNameToIndex);
		}
	}

	/// @brief hasMesh を持つ最初のノードを meshRoot として扱う
	static int FindMeshRootNodeIndex(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	{
		for (int nodeIndex = 0; nodeIndex < static_cast<int>(_nodes.size()); nodeIndex++)
		{
			if (_nodes[nodeIndex].hasMesh)
			{
				return nodeIndex;
			}
		}

		return 0;
	}

	/// @brief parentIndex < 0 を持つ最初のノードをシーンルートとして返す
	static int FindSceneRootNodeIndex(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	{
		for (int nodeIndex = 0; nodeIndex < static_cast<int>(_nodes.size()); nodeIndex++)
		{
			if (_nodes[nodeIndex].parentIndex < 0)
			{
				return nodeIndex;
			}
		}

		return 0;
	}

	//-----------------------------------------------------------------------------
	// Vertex influence temp struct
	//-----------------------------------------------------------------------------
	/// @brief 一時的に頂点へ積む影響情報（後で 4つに正規化して格納する）
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

	//-----------------------------------------------------------------------------
	// Constructor / Destructor
	//-----------------------------------------------------------------------------
	ModelImporter::ModelImporter()
	{
		textureLoader = std::make_unique<TextureLoader>();
	}

	ModelImporter::~ModelImporter()
	{
		textureLoader.reset();
	}

	//-----------------------------------------------------------------------------
	// BuildMaterials
	//-----------------------------------------------------------------------------
	/** @brief Assimp のシーンから Material 情報と diffuse テクスチャを ModelData に構築する
	 *  @param _scene Assimp シーン（非 null）
	 *  @param _modelData 出力先 ModelData
	 *  @param _textureDir テクスチャディレクトリ（相対パス/絶対パスを許容）
	 */
	void ModelImporter::BuildMaterials(const aiScene* _scene, ModelData& _modelData, const std::string& _textureDir) const
	{
		assert(_scene != nullptr);
		assert(textureLoader != nullptr);

		const unsigned int materialCount = _scene->mNumMaterials;

		_modelData.materials.clear();
		_modelData.diffuseTextures.clear();
		_modelData.materials.resize(materialCount);
		_modelData.diffuseTextures.resize(materialCount);

		for (unsigned int materialIndex = 0; materialIndex < materialCount; materialIndex++)
		{
			const aiMaterial* material = _scene->mMaterials[materialIndex];

			Material outMaterial{};
			outMaterial.materialName = ::GetMaterialName(material);

			// デフォルト値の設定（マテリアル情報が欠落していても安全に扱えるように）
			outMaterial.ambient = aiColor4D(0, 0, 0, 1);
			outMaterial.diffuse = aiColor4D(1, 1, 1, 1);
			outMaterial.specular = aiColor4D(1, 1, 1, 1);
			outMaterial.emission = aiColor4D(0, 0, 0, 1);
			outMaterial.shiness = 0.0f;

			::TryGetColor(material, AI_MATKEY_COLOR_AMBIENT, outMaterial.ambient);
			::TryGetColor(material, AI_MATKEY_COLOR_DIFFUSE, outMaterial.diffuse);
			::TryGetColor(material, AI_MATKEY_COLOR_SPECULAR, outMaterial.specular);
			::TryGetColor(material, AI_MATKEY_COLOR_EMISSIVE, outMaterial.emission);
			::TryGetShininess(material, outMaterial.shiness);

			// テクスチャのパスを取得して TextureLoader で読み込む（存在しない場合は nullptr）
			outMaterial.diffuseTextureName = ::GetDiffuseTexturePath(material);
			if (!outMaterial.diffuseTextureName.empty())
			{
				const std::string textureFullPath = ::MakeTextureFullPath(_textureDir, outMaterial.diffuseTextureName);
				_modelData.diffuseTextures[materialIndex] = textureLoader->FromFile(textureFullPath);
			}
			else
			{
				_modelData.diffuseTextures[materialIndex] = nullptr;
			}

			_modelData.materials[materialIndex] = std::move(outMaterial);
		}
	}

	//-----------------------------------------------------------------------------
	// BuildMeshBuffers（頂点バッファ・インデックスバッファ構築）
	//-----------------------------------------------------------------------------
	/** @brief Assimp の各メッシュから頂点配列とインデックス配列を ModelData に作る
	 *  @param _scene Assimp シーン
	 *  @param _modelData 出力先 ModelData
	 */
	void ModelImporter::BuildMeshBuffers(const aiScene* _scene, ModelData& _modelData) const
	{
		assert(_scene != nullptr);

		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.vertices.clear();
		_modelData.indices.clear();
		_modelData.vertices.resize(meshCount);
		_modelData.indices.resize(meshCount);

		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			const std::string meshName = mesh->mName.C_Str();
			const unsigned int vertexCount = mesh->mNumVertices;

			auto& outVertices = _modelData.vertices[meshIndex];
			outVertices.clear();
			outVertices.resize(vertexCount);

			// メッシュに含まれるチャンネルの有無を確認
			const bool hasNormals = (mesh->mNormals != nullptr);
			const bool hasColors0 = (mesh->mColors[0] != nullptr);
			const bool hasTex0 = (mesh->mTextureCoords[0] != nullptr);

			const int materialIndex = static_cast<int>(mesh->mMaterialIndex);
			std::string materialName = "";

			if (materialIndex >= 0 && static_cast<size_t>(materialIndex) < _modelData.materials.size())
			{
				materialName = _modelData.materials[materialIndex].materialName;
			}

			for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
			{
				Vertex vertex{};
				vertex.meshName = meshName;

				// 位置
				vertex.pos = mesh->mVertices[vertexIndex];

				// 法線が無い場合は上方向をデフォルト法線として埋める
				if (hasNormals)
				{
					vertex.normal = mesh->mNormals[vertexIndex];
				}
				else
				{
					vertex.normal = aiVector3D(0.0f, 1.0f, 0.0f);
				}

				// 頂点カラー
				if (hasColors0)
				{
					vertex.color = mesh->mColors[0][vertexIndex];
				}
				else
				{
					vertex.color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}

				// テクスチャ座標（Assimp は最大 3 成分を返すが UV には x,y を使う）
				if (hasTex0)
				{
					const aiVector3D uvw = mesh->mTextureCoords[0][vertexIndex];
					vertex.texCoord = aiVector3D(uvw.x, uvw.y, 0.0f);
				}
				else
				{
					vertex.texCoord = aiVector3D(0.0f, 0.0f, 0.0f);
				}

				vertex.materialIndex = materialIndex;
				vertex.materialName = materialName;

				outVertices[vertexIndex] = vertex;
			}

			auto& outIndices = _modelData.indices[meshIndex];
			outIndices.clear();
			outIndices.reserve(mesh->mNumFaces * 3);

			// 各フェースを走査してインデックスを構築。四角ポリゴンが来た場合に分割する
			for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
			{
				const aiFace& face = mesh->mFaces[faceIndex];

				if (face.mNumIndices < 3)
				{
					continue;
				}

				outIndices.push_back(static_cast<unsigned int>(face.mIndices[0]));
				outIndices.push_back(static_cast<unsigned int>(face.mIndices[1]));
				outIndices.push_back(static_cast<unsigned int>(face.mIndices[2]));

				// 四角形の場合はもう一枚分の三角形を追加
				if (face.mNumIndices == 4)
				{
					outIndices.push_back(static_cast<unsigned int>(face.mIndices[0]));
					outIndices.push_back(static_cast<unsigned int>(face.mIndices[2]));
					outIndices.push_back(static_cast<unsigned int>(face.mIndices[3]));
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// BuildSubsets
	//-----------------------------------------------------------------------------
	/** @brief メッシュ単位の Subset 情報を構築する
	 *  @param _scene Assimp シーン
	 *  @param _modelData 出力先 ModelData
	 *  @param _useUnifiedBuffers true の場合、vertexBase/indexBase を連続割り当てする
	 */
	void ModelImporter::BuildSubsets(const aiScene* _scene, ModelData& _modelData, bool _useUnifiedBuffers) const
	{
		assert(_scene != nullptr);

		_modelData.subsets.clear();
		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.subsets.reserve(meshCount);

		unsigned int vertexCursor = 0;
		unsigned int indexCursor = 0;

		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			Subset subset{};
		subset.meshName = mesh->mName.C_Str();
		subset.materialIndex = static_cast<int>(mesh->mMaterialIndex);

			if (subset.materialIndex >= 0 && static_cast<size_t>(subset.materialIndex) < _modelData.materials.size())
			{
				subset.materialName = _modelData.materials[subset.materialIndex].materialName;
			}
			else
			{
				subset.materialName = "";
			}

			subset.vertexNum = (meshIndex < _modelData.vertices.size())
				? static_cast<unsigned int>(_modelData.vertices[meshIndex].size())
				: 0;

			subset.indexNum = (meshIndex < _modelData.indices.size())
				? static_cast<unsigned int>(_modelData.indices[meshIndex].size())
				: 0;

			if (_useUnifiedBuffers)
			{
				subset.vertexBase = vertexCursor;
				subset.indexBase = indexCursor;

				vertexCursor += subset.vertexNum;
				indexCursor += subset.indexNum;
			}
			else
			{
				subset.vertexBase = 0;
				subset.indexBase = 0;
			}

			_modelData.subsets.push_back(subset);
		}
	}

	//-----------------------------------------------------------------------------
	// BuildBonesAndSkinWeights
	//-----------------------------------------------------------------------------
	/** @brief Assimp のボーン情報を ModelData の boneDictionary に収集し、
	 *         頂点ごとのボーン影響を最大4つに正規化して Vertex に書き込む
	 *  @param _scene Assimp シーン
	 *  @param _modelData 入出力の ModelData（vertices/boneDictionary を更新）
	 */
	void ModelImporter::BuildBonesAndSkinWeights(const aiScene* _scene, ModelData& _modelData) const
	{
		assert(_scene != nullptr);

		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.boneDictionary.clear();

		// 初期化：各頂点の bone slot をクリア
		for (unsigned int meshIndex = 0; meshIndex < meshCount && meshIndex < _modelData.vertices.size(); meshIndex++)
		{
			for (auto& vertex : _modelData.vertices[meshIndex])
			{
				for (int slotIndex = 0; slotIndex < 4; slotIndex++)
				{
					vertex.boneIndex[slotIndex] = 0;
					vertex.boneWeight[slotIndex] = 0.0f;
					vertex.boneName[slotIndex].clear();
				}
				vertex.boneCount = 0;
			}
		}

		// influences[meshIndex][vertexIndex] = 影響リスト
		std::vector<std::vector<std::vector<::VertexInfluence>>> vertexInfluencesPerMesh;
		vertexInfluencesPerMesh.resize(meshCount);

		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			const size_t vertexCount = (meshIndex < _modelData.vertices.size())
				? _modelData.vertices[meshIndex].size()
				: 0;

			vertexInfluencesPerMesh[meshIndex].resize(vertexCount);
		}

		int nextBoneIndex = 0;

		// 各メッシュのボーンを走査してボーン辞書を構築
		for (unsigned int meshIndex = 0; meshIndex < meshCount; meshIndex++)
		{
			const aiMesh* mesh = _scene->mMeshes[meshIndex];
			if (!mesh)
			{
				continue;
			}

			if (meshIndex >= _modelData.vertices.size())
			{
				continue;
			}

			const std::string meshName = mesh->mName.C_Str();

			for (unsigned int meshBoneIndex = 0; meshBoneIndex < mesh->mNumBones; meshBoneIndex++)
			{
				const aiBone* aiBonePtr = mesh->mBones[meshBoneIndex];
				if (!aiBonePtr)
				{
					continue;
				}

				const std::string boneName = aiBonePtr->mName.C_Str();

				// boneDictionary に登録（初出のボーンなら index 採番）
				auto itBone = _modelData.boneDictionary.find(boneName);
				if (itBone == _modelData.boneDictionary.end())
				{
					Bone bone{};
					bone.boneName = boneName;
					bone.meshName = meshName;
					bone.armatureName = "";

					bone.localBind = aiMatrix4x4();
					bone.globalBind = aiMatrix4x4();

					bone.animationLocal = aiMatrix4x4();
					bone.offsetMatrix = aiBonePtr->mOffsetMatrix;

					bone.index = nextBoneIndex;
					nextBoneIndex++;

					auto inserted = _modelData.boneDictionary.emplace(boneName, std::move(bone));
					itBone = inserted.first;
				}

				Bone& bone = itBone->second;

				// 頂点ウェイト収集
				for (unsigned int weightIndex = 0; weightIndex < aiBonePtr->mNumWeights; weightIndex++)
				{
					const aiVertexWeight& vertexWeight = aiBonePtr->mWeights[weightIndex];

					const int vertexIndex = static_cast<int>(vertexWeight.mVertexId);
					const float weightValue = static_cast<float>(vertexWeight.mWeight);

					if (weightValue <= 0.0f)
					{
						continue;
					}

					if (vertexIndex < 0 || static_cast<size_t>(vertexIndex) >= _modelData.vertices[meshIndex].size())
					{
						continue;
					}

					// 既存設計に合わせて Weight も保持（後で検証ログに使える）
					Weight outWeight{};
					outWeight.boneName = boneName;
					outWeight.meshName = meshName;
					outWeight.weight = weightValue;
					outWeight.vertexIndex = vertexIndex;
					outWeight.meshIndex = static_cast<int>(meshIndex);
					bone.weights.push_back(outWeight);

					// 一時影響リストへ積む
					::VertexInfluence influence{};
					influence.boneIndex = bone.index;
					influence.weight = weightValue;
					influence.boneName = boneName;

					vertexInfluencesPerMesh[meshIndex][static_cast<size_t>(vertexIndex)].push_back(std::move(influence));
				}
			}
		}

		// 各頂点について上位4つに絞って正規化し、Vertex に格納
		for (unsigned int meshIndex = 0; meshIndex < meshCount && meshIndex < _modelData.vertices.size(); meshIndex++)
		{
			auto& meshVertices = _modelData.vertices[meshIndex];

			for (size_t vertexIndex = 0; vertexIndex < meshVertices.size(); vertexIndex++)
			{
				auto& influences = vertexInfluencesPerMesh[meshIndex][vertexIndex];
				if (influences.empty())
				{
					continue;
				}

				// 影響順にソート（重い順）
				std::sort(
					influences.begin(),
					influences.end(),
					[](const ::VertexInfluence& a, const ::VertexInfluence& b)
					{
						return a.weight > b.weight;
					});

				const size_t influenceCount = std::min<size_t>(4, influences.size());

				float weightSum = 0.0f;
				for (size_t i = 0; i < influenceCount; i++)
				{
					weightSum += influences[i].weight;
				}

				if (weightSum <= 0.0f)
				{
					continue;
				}

				Vertex& outVertex = meshVertices[vertexIndex];

				// 上位 influence を Vertex のスロットへ詰める。合計が 1 になるよう正規化
				for (size_t slotIndex = 0; slotIndex < influenceCount; slotIndex++)
				{
					outVertex.boneIndex[slotIndex] = static_cast<UINT>(influences[slotIndex].boneIndex);
					outVertex.boneWeight[slotIndex] = influences[slotIndex].weight / weightSum;
					outVertex.boneName[slotIndex] = influences[slotIndex].boneName;
				}

				outVertex.boneCount = static_cast<int>(influenceCount);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Debug: Stick_Body(meshRoot) と mixamorig:Hips の関係を出力
	// BuildSkeletonCache の nodeNameToIndex を作った後、nodeCount を確認した後に呼ぶ
	//-----------------------------------------------------------------------------
	/** @brief 指定ノードから親を辿って系譜を出力する（デバッグ用）
	 *  @param _nodes ノード配列
	 *  @param _nodeIndex 起点ノード
	 *  @param _label 表示ラベル
	 */
	static void PrintNodeLineage(
		const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes,
		int _nodeIndex,
		const char* _label)
	{
		if (_nodeIndex < 0 || _nodeIndex >= static_cast<int>(_nodes.size()))
		{
			std::cout << _label << " : nodeIndex invalid = " << _nodeIndex << "\n";
			return;
		}

		std::cout << _label << " : index=" << _nodeIndex
			<< " name=\"" << _nodes[_nodeIndex].name << "\"\n";

		int cur = _nodeIndex;
		int step = 0;

		while (cur >= 0 && cur < static_cast<int>(_nodes.size()) && step < 64)
		{
			const int parent = _nodes[cur].parentIndex;

			std::cout << "  -> [" << cur << "] \"" << _nodes[cur].name << "\"";

			if (parent >= 0)
			{
				std::cout << "  parent=[" << parent << "] \"" << _nodes[parent].name << "\"";
			}
			else
			{
				std::cout << "  parent=[-1]";
			}

			std::cout << "\n";

			cur = parent;
			step++;
		}
	}

	/** @brief _ancestor が _child の祖先か判定する（デバッグ用）
	 *  @param _nodes ノード配列
	 *  @param _ancestor 祖先候補
	 *  @param _child 子孫候補
	 *  @return 祖先なら true
	 */
	static bool IsAncestor(
		const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes,
		int _ancestor,
		int _child)
	{
		if (_ancestor < 0 || _child < 0) { return false; }
		if (_ancestor >= static_cast<int>(_nodes.size())) { return false; }
		if (_child >= static_cast<int>(_nodes.size())) { return false; }

		int cur = _child;
		int step = 0;

		while (cur >= 0 && cur < static_cast<int>(_nodes.size()) && step < 256)
		{
			if (cur == _ancestor)
			{
				return true;
			}

			cur = _nodes[cur].parentIndex;
			step++;
		}

		return false;
	}

	/** @brief 2ノードの関係を出力する（デバッグ用）
	 *  @param _nodes ノード配列
	 *  @param _a ノードA
	 *  @param _aName 表示名A
	 *  @param _b ノードB
	 *  @param _bName 表示名B
	 */
	static void PrintRelationship(
		const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes,
		int _a,
		const char* _aName,
		int _b,
		const char* _bName)
	{
		const bool aIsParentOfB = (_nodes[_b].parentIndex == _a);
		const bool bIsParentOfA = (_nodes[_a].parentIndex == _b);

		if (aIsParentOfB)
		{
			std::cout << "[Relation] \"" << _aName << "\" is PARENT of \"" << _bName << "\"\n";
			return;
		}

		if (bIsParentOfA)
		{
			std::cout << "[Relation] \"" << _bName << "\" is PARENT of \"" << _aName << "\"\n";
			return;
		}

		const int aParent = _nodes[_a].parentIndex;
		const int bParent = _nodes[_b].parentIndex;

		if (aParent >= 0 && aParent == bParent)
		{
			std::cout << "[Relation] \"" << _aName << "\" and \"" << _bName << "\" are SIBLINGS"
				<< " (same parent=\"" << _nodes[aParent].name << "\")\n";
			return;
		}

		if (IsAncestor(_nodes, _a, _b))
		{
			std::cout << "[Relation] \"" << _aName << "\" is ANCESTOR of \"" << _bName << "\" (not direct parent)\n";
			return;
		}

		if (IsAncestor(_nodes, _b, _a))
		{
			std::cout << "[Relation] \"" << _bName << "\" is ANCESTOR of \"" << _aName << "\" (not direct parent)\n";
			return;
		}

		std::cout << "[Relation] \"" << _aName << "\" and \"" << _bName << "\" are NOT parent/child/siblings (likely different branches)\n";
	}

	//-----------------------------------------------------------------------------
	// BuildNodeTree
	//-----------------------------------------------------------------------------
	/** @brief aiNode から TreeNode を再帰構築する（ModelData 用）
	 *  @param _aiNode Assimp ノード
		*  @param _outNode 出力ノード
	 */
	static void BuildNodeTreeRecursive(const aiNode* _aiNode, TreeNode<BoneNode>& _outNode)
	{
		assert(_aiNode != nullptr);

		// ノード名とローカルバインド行列を保存
		_outNode.nodedata.name = _aiNode->mName.C_Str();
		_outNode.nodedata.localBind = _aiNode->mTransformation;

		_outNode.children.clear();

		for (unsigned int childIndex = 0; childIndex < _aiNode->mNumChildren; childIndex++)
		{
			const aiNode* child = _aiNode->mChildren[childIndex];
			if (!child)
			{
				continue;
			}

			auto childNode = std::make_unique<TreeNode<BoneNode>>();
			BuildNodeTreeRecursive(child, *childNode);
			_outNode.Addchild(std::move(childNode));
		}
	}

	void ModelImporter::BuildNodeTree(const aiScene* _scene, ModelData& _modelData) const
	{
		assert(_scene != nullptr);
		assert(_scene->mRootNode != nullptr);

		// ルートから再帰的に TreeNode を生成して ModelData に格納する
		_modelData.nodeTree = TreeNode<BoneNode>();
		BuildNodeTreeRecursive(_scene->mRootNode, _modelData.nodeTree);
	}

	//-----------------------------------------------------------------------------
// SkeletonID helpers
//-----------------------------------------------------------------------------
/** @brief ノード名と親子関係から SkeletonID を作成する（決定的な 64bit）
 *  @details
 *  - Assimp が自動生成する補助ノード（"_$AssimpFbx$_" を含むもの）は無視する
 *  - 既存の BakeNodeIndices の「本体ノードのみを扱う」方針と揃える
 *  - ノード名＋親子関係だけを材料にし、行列やボーン情報は使わない
 *  - ノード順に依存しないよう、親->子の文字列表現をソートしてからハッシュ化する
 *
 *  @param _nodes SkeletonCache の nodes
 *  @return SkeletonID（同一構造なら必ず同じ値になる）
 */
	static uint64_t BuildSkeletonIdFromNodes(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	{
		//-------------------------------------------------------------------------
		// FNV-1a 64bit ハッシュの定数
		//-------------------------------------------------------------------------
		constexpr uint64_t FnvOffsetBasis = 14695981039346656037ull; // 初期値
		constexpr uint64_t FnvPrime = 1099511628211ull;        // 乗数

		//-------------------------------------------------------------------------
		// 任意のバイト列をハッシュに混ぜる関数
		//-------------------------------------------------------------------------
		auto fnv1aAppend = [](uint64_t& _hash, const void* _data, size_t _size)
			{
				const uint8_t* bytes = static_cast<const uint8_t*>(_data);
				for (size_t i = 0; i < _size; ++i)
				{
					_hash ^= static_cast<uint64_t>(bytes[i]);
					_hash *= FnvPrime;
				}
			};

		//-------------------------------------------------------------------------
		// 文字列をハッシュに混ぜる関数
		// ヌル終端を入れることで "ab"+"c" と "a"+"bc" が衝突しないようにする
		//-------------------------------------------------------------------------
		auto fnv1aAppendString = [&](uint64_t& _hash, const std::string& _s)
			{
				fnv1aAppend(_hash, _s.data(), _s.size());
				const char zero = '\0';
				fnv1aAppend(_hash, &zero, 1);
			};

		//-------------------------------------------------------------------------
		// Assimp が生成する補助ノードかどうか判定
		//-------------------------------------------------------------------------
		auto isHelperNode = [](const std::string& _name)
			{
				return (_name.find("_$AssimpFbx$_") != std::string::npos);
			};

		//-------------------------------------------------------------------------
		// 補助ノードを飛ばして「本体ノードの親」を探す
		//-------------------------------------------------------------------------
		auto findVisibleParentIndex = [&](int _nodeIndex)
			{
				int cur = _nodeIndex;
				int step = 0;

				while (cur >= 0 && cur < static_cast<int>(_nodes.size()) && step < 512)
				{
					const int parent = _nodes[cur].parentIndex;
					if (parent < 0)
					{
						// ルート扱い
						return -1;
					}

					// 親が補助ノードでなければそれを採用
					if (!isHelperNode(_nodes[parent].name))
					{
						return parent;
					}

					// 補助ノードならさらに上へ
					cur = parent;
					++step;
				}

				return -1;
			};

		//-------------------------------------------------------------------------
		// ノードが無い場合は 0 固定
		//-------------------------------------------------------------------------
		if (_nodes.empty())
		{
			return 0ull;
		}

		//-------------------------------------------------------------------------
		// 「親名->子名」のエッジ文字列を集める
		//-------------------------------------------------------------------------
		std::vector<std::string> edges;
		edges.reserve(_nodes.size());

		uint64_t visibleNodeCount = 0;

		for (size_t i = 0; i < _nodes.size(); ++i)
		{
			const auto& node = _nodes[i];

			// 補助ノードは SkeletonID の計算から除外
			if (isHelperNode(node.name))
			{
				continue;
			}

			++visibleNodeCount;

			const int parentIndex = findVisibleParentIndex(static_cast<int>(i));

			// 親ノード名（見つからなければ ROOT）
			std::string parentName = "<ROOT>";
			if (parentIndex >= 0)
			{
				parentName = _nodes[parentIndex].name;
			}

			// 親->子 形式の文字列を作成
			std::string edge;
			edge.reserve(parentName.size() + 2 + node.name.size());
			edge += parentName;
			edge += "->";
			edge += node.name;

			edges.push_back(std::move(edge));
		}

		//-------------------------------------------------------------------------
		// ノード順に依存しないよう、エッジ文字列をソート
		//-------------------------------------------------------------------------
		std::sort(edges.begin(), edges.end());

		//-------------------------------------------------------------------------
		// ハッシュ計算開始
		//-------------------------------------------------------------------------
		uint64_t hash = FnvOffsetBasis;

		// ノード数も混ぜる（構造差の検出用）
		fnv1aAppend(hash, &visibleNodeCount, sizeof(visibleNodeCount));

		// 親子関係をすべて混ぜる
		for (const auto& e : edges)
		{
			fnv1aAppendString(hash, e);
		}

		return hash;
	}

	//-----------------------------------------------------------------------------
	// BuildSkeletonCache
	//-----------------------------------------------------------------------------
	/** @brief SkeletonCache（実行時用）を構築する
	 *  @details nodeNameToIndex は内部で一時生成して捨てる
	 *  @param _scene Assimp シーン
	 *  @param _modelData 入力モデルデータ（boneDictionary を参照）
	 *  @param _outSkeletonCache 出力先スケルトンキャッシュ
	 */
	void ModelImporter::BuildSkeletonCache(const aiScene* _scene, const ModelData& _modelData, SkeletonCache& _outSkeletonCache) const
	{
		assert(_scene != nullptr);
		assert(_scene->mRootNode != nullptr);

		_outSkeletonCache.nodes.clear();
		_outSkeletonCache.order.clear();
		_outSkeletonCache.boneOffset.clear();
		_outSkeletonCache.boneIndexToNodeIndex.clear();
		_outSkeletonCache.meshRootNodeIndex = -1;
		_outSkeletonCache.globalInverse = DX::Matrix4x4::Identity;

		std::unordered_map<std::string, int> nodeNameToIndex{};
		::BuildSkeletonNodesRecursive(_scene->mRootNode, -1, _outSkeletonCache.nodes, _outSkeletonCache.order, nodeNameToIndex);

		const int nodeCount = static_cast<int>(_outSkeletonCache.nodes.size());
		if (nodeCount <= 0)
		{
			return;
		}

		// SkeletonID を構築して設定する
		_outSkeletonCache.skeletonID = BuildSkeletonIdFromNodes(_outSkeletonCache.nodes);

		// nodeNameToIndex が完成していて、nodeCount も有効な段階で関係を確認する（デバッグ）
		if (EnableSkinningDebugLog)
		{
			const auto itMeshRoot = nodeNameToIndex.find("Stick_Body");
			const auto itHips = nodeNameToIndex.find("mixamorig:Hips");

			const int stickBodyNodeIndex = (itMeshRoot != nodeNameToIndex.end()) ? itMeshRoot->second : -1;
			const int hipsNodeIndex = (itHips != nodeNameToIndex.end()) ? itHips->second : -1;

			LogHeader("[ModelImporter] Node Relationship Probe (Stick_Body vs mixamorig:Hips)");
			PrintNodeLineage(_outSkeletonCache.nodes, stickBodyNodeIndex, "[Lineage] Stick_Body");
			PrintNodeLineage(_outSkeletonCache.nodes, hipsNodeIndex, "[Lineage] mixamorig:Hips");

			if (stickBodyNodeIndex >= 0 && hipsNodeIndex >= 0)
			{
				PrintRelationship(_outSkeletonCache.nodes, stickBodyNodeIndex, "Stick_Body", hipsNodeIndex, "mixamorig:Hips");
			}
			else
			{
				std::cout << "[Relation] missing: Stick_BodyIndex=" << stickBodyNodeIndex << " hipsIndex=" << hipsNodeIndex << "\n";
			}
		}

		_outSkeletonCache.meshRootNodeIndex = ::FindMeshRootNodeIndex(_outSkeletonCache.nodes);

		// bindGlobalMatrices[nodeIndex] = bind global 行列（row vector 前提）
		std::vector<DX::Matrix4x4> bindGlobalMatrices;
		bindGlobalMatrices.resize(static_cast<size_t>(nodeCount), DX::Matrix4x4::Identity);

		for (int orderIndex = 0; orderIndex < static_cast<int>(_outSkeletonCache.order.size()); orderIndex++)
		{
			const int nodeIndex = _outSkeletonCache.order[orderIndex];
			if (nodeIndex < 0 || nodeIndex >= nodeCount)
			{
				continue;
			}

			const int parentIndex = _outSkeletonCache.nodes[nodeIndex].parentIndex;

			if (parentIndex < 0)
			{
				// ルートはローカル行列そのままをグローバルとする
				bindGlobalMatrices[nodeIndex] = _outSkeletonCache.nodes[nodeIndex].bindLocalMatrix;
			}
			else
			{
				// row vector 前提：global = local * parentGlobal
				bindGlobalMatrices[nodeIndex] =
					_outSkeletonCache.nodes[nodeIndex].bindLocalMatrix *
					bindGlobalMatrices[parentIndex];
			}
		}

		{
			const int meshRootNodeIndex = _outSkeletonCache.meshRootNodeIndex;
			if (meshRootNodeIndex >= 0 && meshRootNodeIndex < nodeCount)
			{
				// skin の基準として meshRoot の inverse を使う
				_outSkeletonCache.globalInverse = DX::InverseMatrix(bindGlobalMatrices[meshRootNodeIndex]);
			}
		}

		// boneDictionary の index は疎になる可能性があるため、最大 index から数を決める
		int maxBoneIndex = -1;
		for (const auto& kv : _modelData.boneDictionary)
		{
			maxBoneIndex = std::max(maxBoneIndex, kv.second.index);
		}

		const int boneCount = maxBoneIndex + 1;
		if (boneCount <= 0)
		{
			return;
		}

		_outSkeletonCache.boneOffset.resize(static_cast<size_t>(boneCount), DX::Matrix4x4::Identity);
		_outSkeletonCache.boneIndexToNodeIndex.resize(static_cast<size_t>(boneCount), -1);

		// boneOffset[boneIndex] = Assimp の offsetMatrix（転置変換済み）
		for (const auto& kv : _modelData.boneDictionary)
		{
			const Bone& bone = kv.second;
			if (bone.index < 0 || bone.index >= boneCount)
			{
				continue;
			}

			_outSkeletonCache.boneOffset[static_cast<size_t>(bone.index)] =
				::ConvertAiMatrixToDxMatrix_Transpose(bone.offsetMatrix);
		}

		// Minimal Log: 基準行列（meshRoot / globalInverse）
		if (EnableSkinningDebugLog)
		{
			::LogHeader("[ModelImporter] Skin Debug Base Matrices (meshRoot/globalInverse)");

			const int meshRootNodeIndex = _outSkeletonCache.meshRootNodeIndex;

			std::cout << "[SkinBase] nodeCount=" << nodeCount
				<< " boneCount=" << boneCount
				<< " meshRootNodeIndex=" << meshRootNodeIndex;

			if (meshRootNodeIndex >= 0 && meshRootNodeIndex < nodeCount)
			{
				std::cout << " meshRootName=\"" << _outSkeletonCache.nodes[meshRootNodeIndex].name << "\"";
			}

			std::cout << "\n";

			if (meshRootNodeIndex >= 0 && meshRootNodeIndex < nodeCount)
			{
				::PrintMatrix4x4("[SkinBase] bindGlobal(meshRoot)", bindGlobalMatrices[static_cast<size_t>(meshRootNodeIndex)]);
			}

			::PrintMatrix4x4("[SkinBase] globalInverse(inverse(bindGlobal(meshRoot)))", _outSkeletonCache.globalInverse);
		}

		//-----------------------------------------------------------------------------
		// boneIndexToNodeIndex 解決 + 最小ログ（しきい値超えだけ）
		//-----------------------------------------------------------------------------
		int warnedCount = 0;
		int probedCount = 0;

		for (const auto& kv : _modelData.boneDictionary)
		{
			const Bone& bone = kv.second;
			const int boneIndex = bone.index;

			if (boneIndex < 0 || boneIndex >= boneCount)
			{
				continue;
			}

			// FBX の回転補助ノードが存在するケースがあるため、候補を2つ持つ
			const std::string baseNodeName = bone.boneName;
			const std::string rotationNodeName = baseNodeName + "_$AssimpFbx$_Rotation";

			int baseNodeIndex = -1;
			int rotationNodeIndex = -1;

			const auto itBase = nodeNameToIndex.find(baseNodeName);
			if (itBase != nodeNameToIndex.end())
			{
				baseNodeIndex = itBase->second;
			}

			const auto itRot = nodeNameToIndex.find(rotationNodeName);
			if (itRot != nodeNameToIndex.end())
			{
				rotationNodeIndex = itRot->second;
			}

			struct Candidate
			{
				int nodeIndex = -1;
				float error = FLT_MAX;
			};

			Candidate baseCandidate{};
			Candidate rotationCandidate{};

			if (baseNodeIndex >= 0 && baseNodeIndex < nodeCount)
			{
				// row vector 前提：skin = offset * bindGlobal(node) * globalInverse(meshRoot)
				const DX::Matrix4x4 skin =
					_outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
					bindGlobalMatrices[static_cast<size_t>(baseNodeIndex)] *
					_outSkeletonCache.globalInverse;

				baseCandidate.nodeIndex = baseNodeIndex;
				baseCandidate.error = ::MaxAbsDiffFromIdentity(skin);
			}

			if (rotationNodeIndex >= 0 && rotationNodeIndex < nodeCount)
			{
				const DX::Matrix4x4 skin =
					_outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
					bindGlobalMatrices[static_cast<size_t>(rotationNodeIndex)] *
					_outSkeletonCache.globalInverse;

				rotationCandidate.nodeIndex = rotationNodeIndex;
				rotationCandidate.error = ::MaxAbsDiffFromIdentity(skin);
			}

			Candidate bestCandidate = baseCandidate;
			if (rotationCandidate.error < bestCandidate.error)
			{
				bestCandidate = rotationCandidate;
			}

			if (bestCandidate.nodeIndex < 0)
			{
				if (EnableSkinningDebugLog && warnedCount < MaxBindPoseWarnBones)
				{
					std::cout << "[ModelImporter][Warn] boneIndexToNodeIndex unresolved: boneName=\"" << baseNodeName << "\"\n";
					warnedCount++;
				}
				continue;
			}

			_outSkeletonCache.boneIndexToNodeIndex[static_cast<size_t>(boneIndex)] = bestCandidate.nodeIndex;

			// ノードがまだボーンを割り当てられていなければ設定する
			if (_outSkeletonCache.nodes[bestCandidate.nodeIndex].boneIndex < 0)
			{
				_outSkeletonCache.nodes[bestCandidate.nodeIndex].boneIndex = boneIndex;
			}

			// しきい値を超える誤差は警告ログを出す（必要なら詳細プローブを出力）
			if (EnableSkinningDebugLog && bestCandidate.error > BindPoseWarnThreshold)
			{
				if (warnedCount < MaxBindPoseWarnBones)
				{
					std::cout
						<< "[ModelImporter][Warn] BindPose identity check failed: boneIndex=" << boneIndex
						<< " boneName=\"" << baseNodeName
						<< "\" chosenNode=\"" << _outSkeletonCache.nodes[bestCandidate.nodeIndex].name
						<< "\" maxAbs=" << bestCandidate.error << "\n";
					warnedCount++;
				}

				if (probedCount < MaxProbePrintBones)
				{
					const DX::Matrix4x4& bindGlobalBone = bindGlobalMatrices[static_cast<size_t>(bestCandidate.nodeIndex)];
					const DX::Matrix4x4& globalInv = _outSkeletonCache.globalInverse;
					const DX::Matrix4x4& offset = _outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)];

					::DebugBindPoseIdentityTransposeProbe(boneIndex, baseNodeName, bindGlobalBone, globalInv, offset);
					probedCount++;
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Load
	//-----------------------------------------------------------------------------
	/** @brief モデルファイルを読み込み、ModelData と SkeletonCache を構築する
	 *  @param _filename モデルファイルパス
	 *  @param _textureDir テクスチャディレクトリ
	 *  @param _outModel 出力先モデルデータ
	 *  @param _outSkeletonCache 出力先スケルトンキャッシュ（実行時用・番号のみ）
	 *  @return 成功時 true
	 */
	bool ModelImporter::Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel, SkeletonCache& _outSkeletonCache)
	{
		Assimp::Importer importer;

		// FBX のピボットをAssimp側で正規化する
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		// 単位変換を有効にする（アニメーションも同じスケールで扱えるようにする）
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 100.0f);

		// 読み込み。 各プロセスフラグの意味:
		//  - aiProcess_Triangulate: ポリゴンを三角形へ分割
		//  - aiProcess_ConvertToLeftHanded: 左手系変換
		//  - aiProcess_GlobalScale: グローバルスケールを適用
		//  - aiProcess_LimitBoneWeights: ボーンウェイト数を制限
		//  - aiProcess_PopulateArmatureData: アーマチュア情報を収集
		const aiScene* scene = importer.ReadFile(_filename,
			aiProcess_Triangulate |               // 三角形化
			aiProcess_ConvertToLeftHanded |      // 左手系変換
			aiProcess_GlobalScale |               // グローバルスケール適用
			aiProcess_LimitBoneWeights |         // ボーンウェイト制限
			aiProcess_PopulateArmatureData       // アーマチュア情報収集
		);

		if (!scene)
		{
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return false;
		}
		
		//-----------------------------------------------------------------------------
		// モデルの各種データを構築
		//-----------------------------------------------------------------------------
		_outModel.vertices.clear();
		_outModel.indices.clear();
		_outModel.materials.clear();
		_outModel.diffuseTextures.clear();
		_outModel.subsets.clear();
		_outModel.boneDictionary.clear();

		BuildMaterials(scene, _outModel, _textureDir);
		BuildMeshBuffers(scene, _outModel);
		BuildSubsets(scene, _outModel, false); // 分離バッファ版
		BuildBonesAndSkinWeights(scene, _outModel);
		BuildNodeTree(scene, _outModel);

		// 読み込んだデータからスケルトンキャッシュを構築
		BuildSkeletonCache(scene, _outModel, _outSkeletonCache);

		//-----------------------------------------------------------------------------
		// Debug: ノード構造＋ボーン対応表
		//-----------------------------------------------------------------------------
		if (EnableSkinningDebugLog)
		{
			::LogHeader("[Structure Analysis] Debug: Node Structure & Bone Mapping");

			for (int nodeIndex = 0; nodeIndex < static_cast<int>(_outSkeletonCache.nodes.size()); nodeIndex++)
			{
				const auto& node = _outSkeletonCache.nodes[nodeIndex];

				// インデントで階層を表現
				std::string indent;
				int parentIndex = node.parentIndex;

				while (parentIndex >= 0)
				{
					indent += "  ";
					parentIndex = _outSkeletonCache.nodes[parentIndex].parentIndex;
				}

				// bind local から平行移動成分(T)を抽出
				const auto& localBind = node.bindLocalMatrix;
				const float translateX = localBind.m[3][0];
				const float translateY = localBind.m[3][1];
				const float translateZ = localBind.m[3][2];

				// ボーン情報があれば併記する
				std::string boneInfo;
				if (node.boneIndex >= 0)
				{
					boneInfo = " [BONE IDX: " + std::to_string(node.boneIndex) + "]";

					// offset の平行移動も併記（向き確認に使う）
					const auto& boneOffset = _outSkeletonCache.boneOffset[static_cast<size_t>(node.boneIndex)];
					boneInfo += " (Offset T: " + std::to_string(boneOffset.m[3][0]) + ", "
						+ std::to_string(boneOffset.m[3][1]) + ", "
						+ std::to_string(boneOffset.m[3][2]) + ")";
				}

				printf("%sNode[%d]: %s %s\n", indent.c_str(), nodeIndex, node.name.c_str(), boneInfo.c_str());
				printf("%s  Bind Local T: (%.4f, %.4f, %.4f)\n", indent.c_str(), translateX, translateY, translateZ);

				if (node.hasMesh)
				{
					printf("%s  *** HAS MESH ***\n", indent.c_str());
				}
			}

			::LogHeader("End of Structure Analysis");
		}

		return true;
	}
} // namespace Graphics::Import