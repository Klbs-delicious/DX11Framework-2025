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
#include <unordered_map>
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
		using TreeNode_t = Utils::TreeNode<std::string>;

	public:
		/// @brief コンストラクタ
		ModelImporter();

		/// @brief デストラクタ
		~ModelImporter();

		/** @brief モデルを読み込み ModelData に変換
		 *  @param const std::string& _filename モデルファイルパス
		 *  @param const std::string& _textureDir テクスチャディレクトリ
		 *  @param ModelData& _outModel 出力先モデルデータ
		 *  @return 成功時 true
		 */
		bool Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel);

	private:
		/** @brief ノードツリーを作成
		 *  @param aiNode* _node Assimpノード
		 *  @param TreeNode_t* _tree 生成先ツリーノード
		 */
		void CreateNodeTree(aiNode* _node, TreeNode_t* _tree);

		/** @brief 空のボーン辞書を作成
		 *  @param aiNode* _node Assimpノード
		 *  @param std::unordered_map<std::string, Bone>& _boneDict ボーン辞書
		 */
		void CreateEmptyBoneDictionary(aiNode* _node, std::unordered_map<std::string, Bone>& _boneDict);

		/** @brief メッシュのボーン情報を取得
		 *  @param const aiMesh* _mesh メッシュデータ
		 *  @param std::unordered_map<std::string, Bone>& _boneDict ボーン辞書
		 *  @return std::vector<Bone> ボーン配列
		 */
		std::vector<Bone> GetBonesPerMesh(const aiMesh* _mesh, std::unordered_map<std::string, Bone>& _boneDict);

		/** @brief 頂点にボーンデータを設定
		 *  @param ModelData& _model 対象モデル
		 */
		void SetBoneDataToVertices(ModelData& _model);

		/** @brief シーンからボーン情報を取得
		 *  @param const aiScene* _scene シーンデータ
		 *  @param ModelData& _model モデルデータ
		 */
		void GetBone(const aiScene* _scene, ModelData& _model);

		/** @brief マテリアルとテクスチャを取得
		 *  @param const aiScene* _scene Assimpシーン
		 *  @param const std::string& _textureDir テクスチャディレクトリ
		 *  @param ModelData& _model モデルデータ
		 */
		void GetMaterialData(const aiScene* _scene, const std::string& _textureDir, ModelData& _model);

	private:
		std::unique_ptr<TextureLoader> textureLoader;	///< テクスチャ読み込み
	};
}
