/** @file   ModelData.h
 *  @date   2025/10/26
 */
#pragma once
#include "Include/Framework/Graphics/TextureResource.h"
#include "Include/Framework/Utils/TreeNode.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <assimp/matrix4x4.h>
#include <assimp/color4.h>

 // 前方宣言
struct Material;
namespace Graphics { class Mesh; }

namespace Graphics::Import
{
    /** @struct Vertex
     *  @brief モデルの頂点情報
     */
    struct Vertex
    {
        std::string meshName = "";          ///< メッシュ名
        aiVector3D pos = {};                ///< 位置
        aiVector3D normal = {};             ///< 法線
        aiColor4D color = { 1, 1, 1, 1 };   ///< 頂点カラー（デフォルト白）
        aiVector3D texCoord = {};           ///< テクスチャ座標
        int materialIndex = -1;             ///< マテリアルインデックス
        std::string materialName = "";      ///< マテリアル名

        int boneIndex[4] = { -1, -1, -1, -1 };              ///< ボーンインデックス
        float boneWeight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };   ///< ボーンウェイト
        std::string boneName[4] = { "", "", "", "" };       ///< ボーン名
        int boneCount = 0;                                  ///< ボーン数
    };

    /** @struct Subset
     *  @brief メッシュごとの描画サブセット情報
     */
    struct Subset
    {
        std::string meshName = "";      ///< メッシュ名
        int materialIndex = -1;         ///< マテリアルインデックス
        unsigned int vertexBase = 0;    ///< 頂点バッファのベース
        unsigned int vertexNum = 0;     ///< 頂点数
        unsigned int indexBase = 0;     ///< インデックスバッファのベース
        unsigned int indexNum = 0;      ///< インデックス数
        std::string materialName = "";  ///< マテリアル名
    };

    /** @struct Material
     *  @brief マテリアル（マテリアル色・テクスチャ名）
     */
    struct Material
    {
        std::string materialName = "";          ///< マテリアル名
        aiColor4D ambient = { 0, 0, 0, 1 };     ///< アンビエント
        aiColor4D diffuse = { 1, 1, 1, 1 };     ///< ディフューズ
        aiColor4D specular = { 1, 1, 1, 1 };    ///< スペキュラ
        aiColor4D emission = { 0, 0, 0, 1 };    ///< エミッション
        float shiness = 0.0f;                   ///< シャイネス
        std::string diffuseTextureName = "";    ///< ディフューズテクスチャ名
    };

    /** @struct Weight
     *  @brief 頂点ごとのボーンウェイト情報
     */
    struct Weight
    {
        std::string boneName = "";      ///< ボーン名
        std::string meshName = "";      ///< メッシュ名
        float weight = 0.0f;            ///< ウェイト値
        int vertexIndex = -1;           ///< 頂点インデックス
    };

    /** @struct BoneNode
     *  @brief ボーンノード情報（名前・ローカル行列）
	 */
    struct BoneNode
    {
        std::string name = "";
        aiMatrix4x4 localBind{};   ///< aiNode::mTransformation（初期ローカル行列）
    };

    /** @struct Bone
     *  @brief ボーン情報（階層・オフセット・ウェイト）
     */
    struct Bone
    {
        std::string boneName = "";      ///< ボーン名
        std::string meshName = "";      ///< メッシュ名
        std::string armatureName = "";  ///< アーマチュア名

        aiMatrix4x4 localBind{};        ///< aiNode::mTransformation
        aiMatrix4x4 globalBind{};       ///< 親から合成した初期行列

        aiMatrix4x4 animationLocal{};   ///< アニメーションから来るローカル
        aiMatrix4x4 offsetMatrix{};     ///< AssimpのOffset

        int index = -1;                 ///< 配列の何番目か
        std::vector<Weight> weights{};  ///< このボーンが影響を与える頂点情報
    };

    /** @struct ModelData
     *  @brief モデル全体の統合データ構造
     */
    struct ModelData
    {
        std::vector<std::vector<Vertex>> vertices{};                        ///< 頂点配列
        std::vector<std::vector<unsigned int>> indices{};                   ///< インデックス配列
        std::vector<Subset> subsets{};                                      ///< サブセット配列
        std::vector<Material> materials{};                                  ///< マテリアル配列
        std::vector<std::unique_ptr<TextureResource>> diffuseTextures{};    ///< テクスチャ配列
        std::unordered_map<std::string, Bone> boneDictionary{};             ///< ボーン辞書
		Utils::TreeNode<BoneNode> boneTree{};                               ///< 名前+初期ローカル行列のボーン階層ツリー
    };
} // namespace Graphics::Import

namespace Graphics
{    
    /** @brief モデルで使用するデータのエントリ
    */
    struct ModelEntry
    {
    public:
        Graphics::Mesh* mesh = nullptr;
        Material* material = nullptr;

        void SetModelData(std::unique_ptr<Graphics::Import::ModelData> _modelData)
        {
            this->modelData = std::move(_modelData);
        }

        Graphics::Import::ModelData* GetModelData() const
        {
            return this->modelData.get();
        }

    private:
        std::unique_ptr<Graphics::Import::ModelData> modelData = nullptr;
    };

    /** @brief モデル情報
    */
    struct ModelInfo
    {
        const std::string filename;     ///< モデルファイル名
        const std::string textureDir;   ///< テクスチャディレクトリ
    };
}