/** @file	ModelImporter.h
 *  @date	2025/10/26
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Utils/TreeNode.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/TextureLoader.h"

#include <assimp/scene.h>
#include <memory>
#include <vector>
#include <string>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	/** @class ModelImporter
	 *  @brief モデルデータをAssimpで読み込み ModelData に変換するクラス
	 */
	class ModelImporter
	{
	public:
		using TreeNode_t = Utils::TreeNode<BoneNode>;

		/// @brief コンストラクタ
		ModelImporter();

		/// @brief デストラクタ
		~ModelImporter();

		/** @brief モデルを読み込み ModelData に変換
		 *  @param _filename モデルファイルパス
		 *  @param _textureDir テクスチャディレクトリ
		 *  @param _outModel 出力先モデルデータ
		 *  @return 成功時 true
		 */
		bool Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel);

	private:
		/** @brief Assimp シーンから Material / DiffuseTexture を構築する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先
		 *  @param _textureDir テクスチャディレクトリ
		 */
		void BuildMaterials(const aiScene* _scene, ModelData& _modelData, const std::string& _textureDir) const;

		/** @brief Assimp のメッシュ群から頂点配列とインデックス配列を構築する
		 *  @param _scene		Assimp シーン
		 *  @param _modelData	出力先モデルデータ
		 */
		void BuildMeshBuffers(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief メッシュ単位の Subset 情報を構築する
		 *  @param _scene				Assimp シーン
		 *  @param _modelData			出力先モデルデータ
		 *  @param _useUnifiedBuffers	1本のVB/IBにまとめる前提で base を詰めるか
		 */
		void BuildSubsets(const aiScene* _scene, ModelData& _modelData, bool _useUnifiedBuffers = false) const;

		/** @brief ボーン辞書（boneDictionary）と頂点の boneIndex/boneWeight を構築する
		 *  @param _scene		Assimp シーン
		 *  @param _modelData	出力先モデルデータ
		 */
		void BuildBonesAndSkinWeights(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief Assimp シーンからノードツリーを構築して ModelData に格納する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先モデルデータ
		 */
		void BuildNodeTree(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief aiNode を再帰的に走査して TreeNode を出力先へ構築する
		 *  @param _aiNode Assimp ノード
		 *  @param _outNode 出力先ノード
		 */
		void BuildNodeTreeRecursive(const aiNode* _aiNode, TreeNode_t& _outNode) const;

	private:
		std::unique_ptr<TextureLoader> textureLoader;	///< テクスチャ読み込み
	};
} // namespace Graphics::Import
