/** @file	ModelImporter.h
 *  @date	2025/10/26
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/TextureLoader.h"

#include <assimp/scene.h>

#include <memory>
#include <string>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	/** @class ModelImporter
	 *  @brief モデルデータをAssimpで読み込み ModelData と SkeletonCache に変換するクラス
	 */
	class ModelImporter
	{
	public:
		/// @brief コンストラクタ
		ModelImporter();

		/// @brief デストラクタ
		~ModelImporter();

		/** @brief モデルを読み込み ModelData と SkeletonCache（実行時用）を構築する
		 *  @param _filename モデルファイルパス
		 *  @param _textureDir テクスチャディレクトリ
		 *  @param _outModel 出力先モデルデータ
		 *  @param _outSkeletonCache 出力先スケルトンキャッシュ（実行時用・番号のみ）
		 *  @return 成功時 true
		 */
		bool Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel, SkeletonCache& _outSkeletonCache);

	private:
		/** @brief Assimp シーンから Material / DiffuseTexture を構築する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先
		 *  @param _textureDir テクスチャディレクトリ
		 */
		void BuildMaterials(const aiScene* _scene, ModelData& _modelData, const std::string& _textureDir) const;

		/** @brief Assimp のメッシュ群から頂点配列とインデックス配列を構築する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先モデルデータ
		 */
		void BuildMeshBuffers(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief メッシュ単位の Subset 情報を構築する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先モデルデータ
		 *  @param _useUnifiedBuffers 1本のVB/IBにまとめる前提で base を詰めるか
		 */
		void BuildSubsets(const aiScene* _scene, ModelData& _modelData, bool _useUnifiedBuffers = false) const;

		/** @brief ボーン辞書（boneDictionary）と頂点の boneIndex/boneWeight を構築する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先モデルデータ
		 */
		void BuildBonesAndSkinWeights(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief Assimp シーンからノードツリーを構築して ModelData に格納する
		 *  @param _scene Assimp シーン
		 *  @param _modelData 出力先モデルデータ
		 */
		void BuildNodeTree(const aiScene* _scene, ModelData& _modelData) const;

		/** @brief SkeletonCache（実行時用）を構築する
		 *  @details nodeNameToIndex は内部で一時生成して捨てる
		 *  @param _scene Assimp シーン
		 *  @param _modelData 入力モデルデータ（boneDictionary を参照）
		 *  @param _outSkeletonCache 出力先スケルトンキャッシュ
		 */
		void BuildSkeletonCache(const aiScene* _scene, const ModelData& _modelData, SkeletonCache& _outSkeletonCache) const;



	private:
		std::unique_ptr<TextureLoader> textureLoader;	///< テクスチャ読み込み
	};
} // namespace Graphics::Import