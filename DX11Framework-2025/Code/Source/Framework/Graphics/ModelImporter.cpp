/** @file	ModelImporter.cpp
 *  @brief	Assimpを利用したモデルデータ読み込み
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
	static constexpr float BindPoseWarnThreshold = 1.0e-3f;
	static constexpr int MaxBindPoseWarnBones = 6;   // しきい値超えのボーンを最大何件ログするか
	static constexpr int MaxProbePrintBones = 6;     // Probe の詳細出力を最大何件にするか

	//-----------------------------------------------------------------------------
	// Matrix helpers
	//-----------------------------------------------------------------------------
	static DX::Matrix4x4 TransposeDxMatrix(const DX::Matrix4x4& _m)
	{
		DirectX::XMFLOAT4X4 f{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(DirectX::XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&f, &_m, sizeof(DirectX::XMFLOAT4X4));

		const DirectX::XMMATRIX xm = DirectX::XMLoadFloat4x4(&f);
		const DirectX::XMMATRIX t = DirectX::XMMatrixTranspose(xm);

		DirectX::XMFLOAT4X4 outF{};
		DirectX::XMStoreFloat4x4(&outF, t);

		DX::Matrix4x4 out{};
		std::memcpy(&out, &outF, sizeof(DirectX::XMFLOAT4X4));
		return out;
	}

	static DX::Matrix4x4 AiToDxMtx_NoTranspose(const aiMatrix4x4& _m)
	{
		return DX::Matrix4x4(
			_m.a1, _m.a2, _m.a3, _m.a4,
			_m.b1, _m.b2, _m.b3, _m.b4,
			_m.c1, _m.c2, _m.c3, _m.c4,
			_m.d1, _m.d2, _m.d3, _m.d4
		);
	}

	static DX::Matrix4x4 AiToDxMtx_Transpose(const aiMatrix4x4& _m)
	{
		return ::TransposeDxMatrix(::AiToDxMtx_NoTranspose(_m));
	}

	static DX::Matrix4x4 InverseDxMatrix(const DX::Matrix4x4& _m)
	{
		using namespace DirectX;

		XMFLOAT4X4 f{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&f, &_m, sizeof(XMFLOAT4X4));

		const XMMATRIX xm = XMLoadFloat4x4(&f);
		XMVECTOR det{};
		const XMMATRIX inv = XMMatrixInverse(&det, xm);

		XMFLOAT4X4 outF{};
		XMStoreFloat4x4(&outF, inv);

		DX::Matrix4x4 out{};
		std::memcpy(&out, &outF, sizeof(XMFLOAT4X4));
		return out;
	}

	static DX::Matrix4x4 MaybeTranspose(const DX::Matrix4x4& _m, bool _doTranspose)
	{
		if (!_doTranspose) { return _m; }
		return ::TransposeDxMatrix(_m);
	}

	static float MaxAbsDiffIdentity(const DX::Matrix4x4& _m)
	{
		const float id[16] =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		const float* p = reinterpret_cast<const float*>(&_m);

		float maxAbs = 0.0f;
		for (int i = 0; i < 16; i++)
		{
			const float d = std::fabs(p[i] - id[i]);
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
	static void LogHeader(const char* _title)
	{
		if (!EnableSkinningDebugLog) { return; }

		std::cout << "\n";
		std::cout << "============================================================\n";
		std::cout << _title << "\n";
		std::cout << "============================================================\n";
	}

	static void PrintMatrix4x4(const char* _label, const DX::Matrix4x4& _m)
	{
		if (!EnableSkinningDebugLog) { return; }

		std::cout << _label << std::endl;
		std::cout
			<< "  [r0] " << _m.m[0][0] << ", " << _m.m[0][1] << ", " << _m.m[0][2] << ", " << _m.m[0][3] << "\n"
			<< "  [r1] " << _m.m[1][0] << ", " << _m.m[1][1] << ", " << _m.m[1][2] << ", " << _m.m[1][3] << "\n"
			<< "  [r2] " << _m.m[2][0] << ", " << _m.m[2][1] << ", " << _m.m[2][2] << ", " << _m.m[2][3] << "\n"
			<< "  [r3] " << _m.m[3][0] << ", " << _m.m[3][1] << ", " << _m.m[3][2] << ", " << _m.m[3][3] << std::endl;
	}

	//-----------------------------------------------------------------------------
	// Minimal debug: bind pose identity の転置パターン総当たり
	//-----------------------------------------------------------------------------
	/** @brief bind pose の identity 成否を「転置の有無 8通り」で総当たりして、最小誤差の組み合わせを出す */
	static void DebugBindPoseIdentityTransposeProbe(
		int _boneIndex,
		const std::string& _boneName,
		const DX::Matrix4x4& _bindGlobalBone,
		const DX::Matrix4x4& _globalInverseMeshRoot,
		const DX::Matrix4x4& _boneOffset)
	{
		if (!EnableSkinningDebugLog) { return; }

		float best = FLT_MAX;
		int bestMask = 0;
		DX::Matrix4x4 bestM = DX::Matrix4x4::Identity;

		for (int mask = 0; mask < 8; ++mask)
		{
			const bool tBindGlobal = ((mask & 1) != 0);
			const bool tGlobalInv = ((mask & 2) != 0);
			const bool tOffset = ((mask & 4) != 0);

			const DX::Matrix4x4 bg = MaybeTranspose(_bindGlobalBone, tBindGlobal);
			const DX::Matrix4x4 gi = MaybeTranspose(_globalInverseMeshRoot, tGlobalInv);
			const DX::Matrix4x4 of = MaybeTranspose(_boneOffset, tOffset);

			// row vector 前提：skin = offset * bindGlobal(bone) * globalInverse(meshRoot)
			const DX::Matrix4x4 skin = of * bg * gi;

			const float err = MaxAbsDiffIdentity(skin);
			if (err < best)
			{
				best = err;
				bestMask = mask;
				bestM = skin;
			}
		}

		const bool bestTBindGlobal = ((bestMask & 1) != 0);
		const bool bestTGlobalInv = ((bestMask & 2) != 0);
		const bool bestTOffset = ((bestMask & 4) != 0);

		std::cout
			<< "[BindPoseTransposeProbe] boneIndex=" << _boneIndex
			<< " boneName=\"" << _boneName << "\""
			<< " bestMaxAbs=" << best
			<< " bestTranspose={ bindGlobal:" << (bestTBindGlobal ? "T" : "N")
			<< ", globalInv:" << (bestTGlobalInv ? "T" : "N")
			<< ", offset:" << (bestTOffset ? "T" : "N")
			<< " }"
			<< std::endl;

		if (best > 1e-3f)
		{
			::PrintMatrix4x4("  bestSkin", bestM);
		}
	}

	//-----------------------------------------------------------------------------
	// Material helpers
	//-----------------------------------------------------------------------------
	static std::string MakeTextureFullPath(const std::string& _textureDir, const std::string& _texPath)
	{
		if (_texPath.empty())
		{
			return "";
		}

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

	static void TryGetColor(
		const aiMaterial* _mat,
		const char* _key,
		unsigned int _type,
		unsigned int _index,
		aiColor4D& _ioColor)
	{
		if (!_mat) { return; }

		aiColor4D c;
		if (aiGetMaterialColor(_mat, _key, _type, _index, &c) == AI_SUCCESS)
		{
			_ioColor = c;
		}
	}

	static void TryGetColor(const aiMaterial* _mat, const char* _keyColor, aiColor4D& _ioColor)
	{
		::TryGetColor(_mat, _keyColor, 0, 0, _ioColor);
	}

	static void TryGetShininess(const aiMaterial* _mat, float& _ioShininess)
	{
		if (!_mat) { return; }

		float s = 0.0f;
		unsigned int max = 1;
		if (aiGetMaterialFloatArray(_mat, AI_MATKEY_SHININESS, &s, &max) == AI_SUCCESS)
		{
			_ioShininess = s;
		}
	}

	static std::string GetDiffuseTexturePath(const aiMaterial* _mat)
	{
		if (!_mat) { return ""; }

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

		const int myIndex = static_cast<int>(_outNodes.size());

		Graphics::Import::SkeletonNodeCache n{};
		n.name = _node->mName.C_Str();
		n.parentIndex = _parentIndex;
		n.bindLocalMatrix = AiToDxMtx_Transpose(_node->mTransformation);
		n.hasMesh = (_node->mNumMeshes > 0);
		n.boneIndex = -1;

		_outNodes.push_back(n);
		_outOrder.push_back(myIndex);

		_outNodeNameToIndex.emplace(n.name, myIndex);

		for (unsigned int i = 0; i < _node->mNumChildren; i++)
		{
			BuildSkeletonNodesRecursive(_node->mChildren[i], myIndex, _outNodes, _outOrder, _outNodeNameToIndex);
		}
	}

	static int FindMeshRootNodeIndex(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	{
		for (int i = 0; i < static_cast<int>(_nodes.size()); i++)
		{
			if (_nodes[i].hasMesh)
			{
				return i;
			}
		}
		return 0;
	}

	static int FindSceneRootNodeIndex(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	{
		for (int i = 0; i < static_cast<int>(_nodes.size()); i++)
		{
			if (_nodes[i].parentIndex < 0)
			{
				return i;
			}
		}
		return 0;
	}

	//-----------------------------------------------------------------------------
	// Vertex influence temp struct
	//-----------------------------------------------------------------------------
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
	void ModelImporter::BuildMaterials(const aiScene* _scene, ModelData& _modelData, const std::string& _textureDir) const
	{
		assert(_scene != nullptr);
		assert(textureLoader != nullptr);

		const unsigned int materialCount = _scene->mNumMaterials;

		_modelData.materials.clear();
		_modelData.diffuseTextures.clear();
		_modelData.materials.resize(materialCount);
		_modelData.diffuseTextures.resize(materialCount);

		for (unsigned int i = 0; i < materialCount; i++)
		{
			const aiMaterial* mat = _scene->mMaterials[i];

			Material out{};
			out.materialName = ::GetMaterialName(mat);

			out.ambient = aiColor4D(0, 0, 0, 1);
			out.diffuse = aiColor4D(1, 1, 1, 1);
			out.specular = aiColor4D(1, 1, 1, 1);
			out.emission = aiColor4D(0, 0, 0, 1);
			out.shiness = 0.0f;

			::TryGetColor(mat, AI_MATKEY_COLOR_AMBIENT, out.ambient);
			::TryGetColor(mat, AI_MATKEY_COLOR_DIFFUSE, out.diffuse);
			::TryGetColor(mat, AI_MATKEY_COLOR_SPECULAR, out.specular);
			::TryGetColor(mat, AI_MATKEY_COLOR_EMISSIVE, out.emission);
			::TryGetShininess(mat, out.shiness);

			out.diffuseTextureName = ::GetDiffuseTexturePath(mat);
			if (!out.diffuseTextureName.empty())
			{
				const std::string fullPath = ::MakeTextureFullPath(_textureDir, out.diffuseTextureName);
				_modelData.diffuseTextures[i] = textureLoader->FromFile(fullPath);
			}
			else
			{
				_modelData.diffuseTextures[i] = nullptr;
			}

			_modelData.materials[i] = std::move(out);
		}
	}

	//-----------------------------------------------------------------------------
	// BuildMeshBuffers（頂点バッファ・インデックスバッファ構築）
	//-----------------------------------------------------------------------------
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
			const unsigned int vCount = mesh->mNumVertices;

			auto& outVerts = _modelData.vertices[meshIndex];
			outVerts.clear();
			outVerts.resize(vCount);

			const bool hasNormals = (mesh->mNormals != nullptr);
			const bool hasColors0 = (mesh->mColors[0] != nullptr);
			const bool hasTex0 = (mesh->mTextureCoords[0] != nullptr);

			const int materialIndex = static_cast<int>(mesh->mMaterialIndex);
			std::string materialName = "";
			if (materialIndex >= 0 && static_cast<size_t>(materialIndex) < _modelData.materials.size())
			{
				materialName = _modelData.materials[materialIndex].materialName;
			}

			for (unsigned int v = 0; v < vCount; v++)
			{
				Vertex vert{};
				vert.meshName = meshName;

				vert.pos = mesh->mVertices[v];

				if (hasNormals)
				{
					vert.normal = mesh->mNormals[v];
				}
				else
				{
					vert.normal = aiVector3D(0.0f, 1.0f, 0.0f);
				}

				if (hasColors0)
				{
					vert.color = mesh->mColors[0][v];
				}
				else
				{
					vert.color = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
				}

				if (hasTex0)
				{
					const aiVector3D uvw = mesh->mTextureCoords[0][v];
					vert.texCoord = aiVector3D(uvw.x, uvw.y, 0.0f);
				}
				else
				{
					vert.texCoord = aiVector3D(0.0f, 0.0f, 0.0f);
				}

				vert.materialIndex = materialIndex;
				vert.materialName = materialName;

				outVerts[v] = vert;
			}

			auto& outIdx = _modelData.indices[meshIndex];
			outIdx.clear();
			outIdx.reserve(mesh->mNumFaces * 3);

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace& face = mesh->mFaces[f];

				if (face.mNumIndices < 3)
				{
					continue;
				}

				outIdx.push_back(static_cast<unsigned int>(face.mIndices[0]));
				outIdx.push_back(static_cast<unsigned int>(face.mIndices[1]));
				outIdx.push_back(static_cast<unsigned int>(face.mIndices[2]));

				if (face.mNumIndices == 4)
				{
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[0]));
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[2]));
					outIdx.push_back(static_cast<unsigned int>(face.mIndices[3]));
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// BuildSubsets
	//-----------------------------------------------------------------------------
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

			if (meshIndex < _modelData.vertices.size())
			{
				subset.vertexNum = static_cast<unsigned int>(_modelData.vertices[meshIndex].size());
			}
			else
			{
				subset.vertexNum = 0;
			}

			if (meshIndex < _modelData.indices.size())
			{
				subset.indexNum = static_cast<unsigned int>(_modelData.indices[meshIndex].size());
			}
			else
			{
				subset.indexNum = 0;
			}

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
	void ModelImporter::BuildBonesAndSkinWeights(const aiScene* _scene, ModelData& _modelData) const
	{
		assert(_scene != nullptr);

		const unsigned int meshCount = _scene->mNumMeshes;
		_modelData.boneDictionary.clear();

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

		std::vector<std::vector<std::vector<::VertexInfluence>>> influences;
		influences.resize(meshCount);
		for (unsigned int m = 0; m < meshCount; m++)
		{
			const aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh)
			{
				continue;
			}

			const size_t vCount = (m < _modelData.vertices.size()) ? _modelData.vertices[m].size() : 0;
			influences[m].resize(vCount);
		}

		int nextBoneIndex = 0;

		for (unsigned int m = 0; m < meshCount; m++)
		{
			const aiMesh* mesh = _scene->mMeshes[m];
			if (!mesh)
			{
				continue;
			}

			const std::string meshName = mesh->mName.C_Str();

			if (m >= _modelData.vertices.size())
			{
				continue;
			}

			for (unsigned int b = 0; b < mesh->mNumBones; b++)
			{
				const aiBone* aiBonePtr = mesh->mBones[b];
				if (!aiBonePtr)
				{
					continue;
				}

				const std::string boneName = aiBonePtr->mName.C_Str();

				auto it = _modelData.boneDictionary.find(boneName);
				if (it == _modelData.boneDictionary.end())
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
					it = inserted.first;
				}

				Bone& bone = it->second;

				for (unsigned int w = 0; w < aiBonePtr->mNumWeights; w++)
				{
					const aiVertexWeight& vw = aiBonePtr->mWeights[w];

					const int vertexIndex = static_cast<int>(vw.mVertexId);
					const float weight = static_cast<float>(vw.mWeight);

					if (weight <= 0.0f)
					{
						continue;
					}
					if (vertexIndex < 0 || static_cast<size_t>(vertexIndex) >= _modelData.vertices[m].size())
					{
						continue;
					}

					Weight outW{};
					outW.boneName = boneName;
					outW.meshName = meshName;
					outW.weight = weight;
					outW.vertexIndex = vertexIndex;
					outW.meshIndex = static_cast<int>(m);
					bone.weights.push_back(outW);

					::VertexInfluence inf{};
					inf.boneIndex = bone.index;
					inf.weight = weight;
					inf.boneName = boneName;

					influences[m][static_cast<size_t>(vertexIndex)].push_back(std::move(inf));
				}
			}
		}

		for (unsigned int m = 0; m < meshCount && m < _modelData.vertices.size(); m++)
		{
			auto& meshVerts = _modelData.vertices[m];
			for (size_t v = 0; v < meshVerts.size(); v++)
			{
				auto& list = influences[m][v];
				if (list.empty())
				{
					continue;
				}

				std::sort(
					list.begin(),
					list.end(),
					[](const ::VertexInfluence& a, const ::VertexInfluence& b)
					{
						return a.weight > b.weight;
					});

				const size_t count = std::min<size_t>(4, list.size());

				float sum = 0.0f;
				for (size_t i = 0; i < count; i++)
				{
					sum += list[i].weight;
				}

				if (sum <= 0.0f)
				{
					continue;
				}

				Vertex& outV = meshVerts[v];
				for (size_t i = 0; i < count; i++)
				{
					outV.boneIndex[i] = static_cast<UINT>(list[i].boneIndex);
					outV.boneWeight[i] = list[i].weight / sum;
					outV.boneName[i] = list[i].boneName;
				}

				outV.boneCount = static_cast<int>(count);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Debug: Stick_Body(meshRoot) と mixamorig:Hips の関係を出力
	// BuildSkeletonCache の nodeNameToIndex を作った後、nodeCount を確認した後に呼ぶ
	//-----------------------------------------------------------------------------
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
			if (cur == _ancestor) { return true; }
			cur = _nodes[cur].parentIndex;
			step++;
		}
		return false;
	}

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
	static void BuildNodeTreeRecursive(const aiNode* _aiNode, TreeNode<BoneNode>& _outNode)
	{
		assert(_aiNode != nullptr);

		_outNode.nodedata.name = _aiNode->mName.C_Str();
		_outNode.nodedata.localBind = _aiNode->mTransformation;

		_outNode.children.clear();

		for (unsigned int i = 0; i < _aiNode->mNumChildren; i++)
		{
			const aiNode* child = _aiNode->mChildren[i];
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

		_modelData.nodeTree = TreeNode<BoneNode>();
		BuildNodeTreeRecursive(_scene->mRootNode, _modelData.nodeTree);
	}

	//-----------------------------------------------------------------------------
	// BuildSkeletonCache
	//-----------------------------------------------------------------------------
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


		// ここに入れる（nodeNameToIndex が完成していて、nodeCount も有効な段階）
		if (EnableSkinningDebugLog)
		{
			auto itMeshRoot = nodeNameToIndex.find("Stick_Body");
			auto itHips = nodeNameToIndex.find("mixamorig:Hips");


			const int stickBodyIndex = (itMeshRoot != nodeNameToIndex.end()) ? itMeshRoot->second : -1;
			const int hipsIndex = (itHips != nodeNameToIndex.end()) ? itHips->second : -1;


			LogHeader("[ModelImporter] Node Relationship Probe (Stick_Body vs mixamorig:Hips)");
			PrintNodeLineage(_outSkeletonCache.nodes, stickBodyIndex, "[Lineage] Stick_Body");
			PrintNodeLineage(_outSkeletonCache.nodes, hipsIndex, "[Lineage] mixamorig:Hips");


			if (stickBodyIndex >= 0 && hipsIndex >= 0)
			{
				PrintRelationship(_outSkeletonCache.nodes, stickBodyIndex, "Stick_Body", hipsIndex, "mixamorig:Hips");
			}
			else
			{
				std::cout << "[Relation] missing: Stick_BodyIndex=" << stickBodyIndex << " hipsIndex=" << hipsIndex << "\n";
			}
		}

		_outSkeletonCache.meshRootNodeIndex = ::FindMeshRootNodeIndex(_outSkeletonCache.nodes);
		std::vector<DX::Matrix4x4> bindGlobal{};
		bindGlobal.resize(static_cast<size_t>(nodeCount), DX::Matrix4x4::Identity);

		for (int oi = 0; oi < static_cast<int>(_outSkeletonCache.order.size()); oi++)
		{
			const int nodeIndex = _outSkeletonCache.order[oi];
			if (nodeIndex < 0 || nodeIndex >= nodeCount)
			{
				continue;
			}

			const int parentIndex = _outSkeletonCache.nodes[nodeIndex].parentIndex;

			if (parentIndex < 0)
			{
				bindGlobal[nodeIndex] = _outSkeletonCache.nodes[nodeIndex].bindLocalMatrix;
			}
			else
			{
				// row vector 前提：global = local * parentGlobal
				bindGlobal[nodeIndex] =
					_outSkeletonCache.nodes[nodeIndex].bindLocalMatrix *
					bindGlobal[parentIndex];
			}
		}

		{
			const int root = _outSkeletonCache.meshRootNodeIndex;
			if (root >= 0 && root < nodeCount)
			{
				_outSkeletonCache.globalInverse = ::InverseDxMatrix(bindGlobal[root]);
			}
		}

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

		for (const auto& kv : _modelData.boneDictionary)
		{
			const Bone& b = kv.second;
			if (b.index < 0 || b.index >= boneCount)
			{
				continue;
			}

			_outSkeletonCache.boneOffset[static_cast<size_t>(b.index)] = ::AiToDxMtx_Transpose(b.offsetMatrix);
		}

		//-----------------------------------------------------------------------------
		// Minimal Log: 基準行列（meshRoot / globalInverse）
		//-----------------------------------------------------------------------------
		if (EnableSkinningDebugLog)
		{
			::LogHeader("[ModelImporter] Skin Debug Base Matrices (meshRoot/globalInverse)");

			const int meshRootIndex = _outSkeletonCache.meshRootNodeIndex;
			std::cout << "[SkinBase] nodeCount=" << nodeCount
				<< " boneCount=" << boneCount
				<< " meshRootNodeIndex=" << meshRootIndex;

			if (meshRootIndex >= 0 && meshRootIndex < nodeCount)
			{
				std::cout << " meshRootName=\"" << _outSkeletonCache.nodes[meshRootIndex].name << "\"";
			}
			std::cout << "\n";

			if (meshRootIndex >= 0 && meshRootIndex < nodeCount)
			{
				::PrintMatrix4x4("[SkinBase] bindGlobal(meshRoot)", bindGlobal[static_cast<size_t>(meshRootIndex)]);
			}
			::PrintMatrix4x4("[SkinBase] globalInverse(inverse(bindGlobal(meshRoot)))", _outSkeletonCache.globalInverse);
		}

		//-----------------------------------------------------------------------------
		// boneIndexToNodeIndex 解決 + 最小ログ（しきい値超えだけ）
		//-----------------------------------------------------------------------------
		int warned = 0;
		int probed = 0;

		for (const auto& kv : _modelData.boneDictionary)
		{
			const Bone& b = kv.second;
			const int boneIndex = b.index;

			if (boneIndex < 0 || boneIndex >= boneCount)
			{
				continue;
			}

			const std::string baseName = b.boneName;
			const std::string rotName = baseName + "_$AssimpFbx$_Rotation";

			int baseNodeIndex = -1;
			int rotNodeIndex = -1;

			auto itBase = nodeNameToIndex.find(baseName);
			if (itBase != nodeNameToIndex.end())
			{
				baseNodeIndex = itBase->second;
			}

			auto itRot = nodeNameToIndex.find(rotName);
			if (itRot != nodeNameToIndex.end())
			{
				rotNodeIndex = itRot->second;
			}

			struct Candidate
			{
				int nodeIndex = -1;
				float error = FLT_MAX;
			};

			Candidate c0{};
			Candidate c1{};

			if (baseNodeIndex >= 0 && baseNodeIndex < nodeCount)
			{
				const DX::Matrix4x4 skin =
					_outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
					bindGlobal[static_cast<size_t>(baseNodeIndex)] *
					_outSkeletonCache.globalInverse;

				c0.nodeIndex = baseNodeIndex;
				c0.error = ::MaxAbsDiffIdentity(skin);
			}

			if (rotNodeIndex >= 0 && rotNodeIndex < nodeCount)
			{
				const DX::Matrix4x4 skin =
					_outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
					bindGlobal[static_cast<size_t>(rotNodeIndex)] *
					_outSkeletonCache.globalInverse;

				c1.nodeIndex = rotNodeIndex;
				c1.error = ::MaxAbsDiffIdentity(skin);
			}

			Candidate best = c0;
			if (c1.error < best.error)
			{
				best = c1;
			}

			if (best.nodeIndex < 0)
			{
				if (EnableSkinningDebugLog && warned < MaxBindPoseWarnBones)
				{
					std::cout << "[ModelImporter][Warn] boneIndexToNodeIndex unresolved: boneName=\"" << baseName << "\"\n";
					warned++;
				}
				continue;
			}

			_outSkeletonCache.boneIndexToNodeIndex[static_cast<size_t>(boneIndex)] = best.nodeIndex;

			if (_outSkeletonCache.nodes[best.nodeIndex].boneIndex < 0)
			{
				_outSkeletonCache.nodes[best.nodeIndex].boneIndex = boneIndex;
			}

			if (EnableSkinningDebugLog && best.error > BindPoseWarnThreshold)
			{
				if (warned < MaxBindPoseWarnBones)
				{
					std::cout
						<< "[ModelImporter][Warn] BindPose identity check failed: boneIndex=" << boneIndex
						<< " boneName=\"" << baseName
						<< "\" chosenNode=\"" << _outSkeletonCache.nodes[best.nodeIndex].name
						<< "\" maxAbs=" << best.error << "\n";
					warned++;
				}

				if (probed < MaxProbePrintBones)
				{
					const DX::Matrix4x4& bindGlobalBone = bindGlobal[static_cast<size_t>(best.nodeIndex)];
					const DX::Matrix4x4& globalInv = _outSkeletonCache.globalInverse;
					const DX::Matrix4x4& offset = _outSkeletonCache.boneOffset[static_cast<size_t>(boneIndex)];

					::DebugBindPoseIdentityTransposeProbe(boneIndex, baseName, bindGlobalBone, globalInv, offset);
					probed++;
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Load
	//-----------------------------------------------------------------------------
	bool ModelImporter::Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel, SkeletonCache& _outSkeletonCache)
	{
		Assimp::Importer importer;

		// 1. FBXの面倒なピボットをAssimpに正規化させる
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		// 2. 単位変換を有効にする (アニメーションも一緒にリサイズされるようになります)
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 100.0f);

		// 3. 読み込みフラグに aiProcess_GlobalScale を追加
		const aiScene* scene = importer.ReadFile(_filename,
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GlobalScale |      // ← これを追加！
			aiProcess_LimitBoneWeights |
			aiProcess_PopulateArmatureData // 骨の構造を整理
		);

		if (!scene)
		{
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return false;
		}

		_outModel.vertices.clear();
		_outModel.indices.clear();
		_outModel.materials.clear();
		_outModel.diffuseTextures.clear();
		_outModel.subsets.clear();
		_outModel.boneDictionary.clear();

		BuildMaterials(scene, _outModel, _textureDir);
		BuildMeshBuffers(scene, _outModel);
		BuildSubsets(scene, _outModel, false);
		BuildBonesAndSkinWeights(scene, _outModel);
		BuildNodeTree(scene, _outModel);

		BuildSkeletonCache(scene, _outModel, _outSkeletonCache);

		// --- ModelImporter.cpp BuildSkeletonCache 関数の最後に貼り付け ---

		if (EnableSkinningDebugLog) {
			::LogHeader("[Structure Analysis] Full Node Tree & Bone Info");

			for (int i = 0; i < static_cast<int>(_outSkeletonCache.nodes.size()); ++i) {
				const auto& node = _outSkeletonCache.nodes[i];

				// インデントで階層を表現
				std::string indent = "";
				int parent = node.parentIndex;
				while (parent >= 0) {
					indent += "  ";
					parent = _outSkeletonCache.nodes[parent].parentIndex;
				}

				// 行列から平行移動成分(T)を抽出
				const auto& m = node.bindLocalMatrix;
				float tx = m.m[3][0];
				float ty = m.m[3][1];
				float tz = m.m[3][2];

				// ボーン情報があるか確認
				std::string boneInfo = "";
				if (node.boneIndex >= 0) {
					boneInfo = " [BONE IDX: " + std::to_string(node.boneIndex) + "]";

					// Offset行列のTも確認
					const auto& off = _outSkeletonCache.boneOffset[node.boneIndex];
					boneInfo += " (Offset T: " + std::to_string(off.m[3][0]) + ", "
						+ std::to_string(off.m[3][1]) + ", "
						+ std::to_string(off.m[3][2]) + ")";
				}

				printf("%sNode[%d]: %s %s\n", indent.c_str(), i, node.name.c_str(), boneInfo.c_str());
				printf("%s  Bind Local T: (%.4f, %.4f, %.4f)\n", indent.c_str(), tx, ty, tz);

				if (node.hasMesh) {
					printf("%s  *** HAS MESH ***\n", indent.c_str());
				}
			}
			::LogHeader("End of Structure Analysis");
		}

		return true;
	}
} // namespace Graphics::Import