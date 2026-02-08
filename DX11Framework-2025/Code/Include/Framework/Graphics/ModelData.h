/** @file   ModelData.h
 *  @date   2025/10/26
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/TextureResource.h"
#include "Include/Framework/Utils/TreeNode.h"
#include "Include/Framework/Shaders/ShaderCommon.h"
#include "Include/Tests/SkinningDebug.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <assimp/matrix4x4.h>
#include <assimp/color4.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>

//-----------------------------------------------------------------------------
// 前方宣言
//-----------------------------------------------------------------------------
struct Material;
namespace Graphics { class Mesh; }
namespace Graphics::Import { struct NodeTrack; }

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
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

        UINT boneIndex[4] = { 0, 0, 0, 0 };                  ///< ボーンインデックス（未使用は 0）
        float boneWeight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };    ///< ボーンウェイト（未使用は 0）
        std::string boneName[4] = { "", "", "", "" };        ///< ボーン名（デバッグ用）
        int boneCount = 0;                                   ///< 有効ボーン数
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
        int meshIndex = -1;             ///< Assimp のメッシュ順（m）
    };

    /** @brief 頂点へのボーン影響情報（一時保存用）
     */
    struct VertexInfluence
    {
        int boneIndex = -1;
        float weight = 0.0f;
        std::string boneName = "";
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

        std::unordered_map<std::string, Bone> boneDictionary{};             ///< ボーン辞書（スキニング用）
		Utils::TreeNode<BoneNode> nodeTree{};                               ///< ノードツリー（骨格構造用）
    };

    /** @struct SkeletonNodeCache
     *  @brief スケルトンノードキャッシュ情報
	 */
    struct SkeletonNodeCache
    {
        std::string name = ""; ///< ノード名（デバッグ用）
        int parentIndex = -1; ///< 親ノード（親なしは -1）
        DX::Matrix4x4 bindLocalMatrix = DX::Matrix4x4::Identity; ///< バインドローカル

        bool hasMesh = false; ///< このノードにメッシュが付くか
        int boneIndex = -1; ///< このノードがボーンなら index、なければ -1
    };

    /** @struct SkeletonCache
     *  @brief スケルトンキャッシュ情報（モデルが変わるまで固定）
     */
    struct SkeletonCache
    {
		uint64_t skeletonID = 0;                    ///< スケルトンキャッシュID（不変）

        std::vector<SkeletonNodeCache> nodes{};     ///< ノード配列（不変）
        std::vector<int> order{};                   ///< 親が先に来る順序（不変）

        std::vector<DX::Matrix4x4> boneOffset{};    ///< boneIndex -> offset（不変）
        std::vector<int> boneIndexToNodeIndex{};    ///< boneIndex -> nodeIndex（不変）

        int meshRootNodeIndex = -1;                             ///< メッシュ基準ノード（不変）
        DX::Matrix4x4 globalInverse = DX::Matrix4x4::Identity;  ///< inverse(bindGlobal(meshRoot))（不変）
    };

    /** @struct Pose
     *  @brief ポーズ情報（毎フレーム更新）
     */
    struct Pose
    {
        std::vector<DX::Matrix4x4> localMatrices{};           ///< ローカル行列（ノード数分）
        std::vector<DX::Matrix4x4> globalMatrices{};          ///< グローバル行列（ノード数分）
        std::vector<DX::Matrix4x4> skinMatrices{};            ///< スキニング行列（ノード数分）

        std::array<DX::Matrix4x4, ShaderCommon::MaxBones> cpuBoneMatrices{};  ///< GPUへ詰める最終配列

        /** @brief ポーズ情報をリセットする
         *  @param _skeletonCache スケルトンキャッシュ
         */
        void Reset(const SkeletonCache& _skeletonCache)
        {
            this->localMatrices.clear();
            this->globalMatrices.clear();
            this->skinMatrices.clear();

            const size_t nodeCount = _skeletonCache.nodes.size();
            this->localMatrices.resize(nodeCount, DX::Matrix4x4::Identity);
            this->globalMatrices.resize(nodeCount, DX::Matrix4x4::Identity);
            this->skinMatrices.resize(nodeCount, DX::Matrix4x4::Identity);

            // GPU用配列もリセット
            for (size_t i = 0; i < ShaderCommon::MaxBones; i++)
            {
                this->cpuBoneMatrices[i] = DX::Matrix4x4::Identity;
            }

            // local を bindLocal で埋める
            for (size_t i = 0; i < nodeCount; i++)
            {
                this->localMatrices[i] = _skeletonCache.nodes[i].bindLocalMatrix;
            }

            // global を親子合成で埋める（order は親が必ず先）
            // 行ベクトル（mul(v, M)）運用：global = local * parentGlobal
            for (size_t oi = 0; oi < _skeletonCache.order.size(); oi++)
            {
                const int nodeIndex = _skeletonCache.order[oi];
                if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodeCount)
                {
                    continue;
                }

                const int parentIndex = _skeletonCache.nodes[nodeIndex].parentIndex;

                if (parentIndex < 0)
                {
                    this->globalMatrices[nodeIndex] = this->localMatrices[nodeIndex];
                }
                else
                {
                    this->globalMatrices[nodeIndex] =
                        this->localMatrices[nodeIndex] *
                        this->globalMatrices[parentIndex];
                }
            }

            //----------------------------------------------
            // スキン行列＆GPU配列初期化
            //----------------------------------------------
            // boneIndex を持つノードだけ skinMatrices と gpuBoneMatrices を埋める
            // ここでは「バインド姿勢」のスキンを作るため animationLocal は使わない
            for (size_t i = 0; i < nodeCount; i++)
            {
                const int boneIndex = _skeletonCache.nodes[i].boneIndex;
                if (boneIndex < 0)
                {
                    continue;
                }

                if (static_cast<size_t>(boneIndex) >= _skeletonCache.boneOffset.size())
                {
                    continue;
                }

                // 行ベクトル運用：
                const DX::Matrix4x4 skin =
                    _skeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
                    this->globalMatrices[i] *
                    _skeletonCache.globalInverse;

                this->skinMatrices[i] = skin;

                // cpuBoneMatrices は「転置なしのCPU行列」を入れる
                if (boneIndex < static_cast<int>(ShaderCommon::MaxBones))
                {
                    this->cpuBoneMatrices[static_cast<size_t>(boneIndex)] = skin;
                    //this->cpuBoneMatrices[static_cast<size_t>(boneIndex)] = DX::Matrix4x4::Identity;
                }
            }

			// デバッグ出力
            Graphics::Debug::Output::DumpBindPoseGlobalCheckOnce(_skeletonCache, *this);
            Graphics::Debug::Output::DumpBindPoseSkinCheckOnce(_skeletonCache, *this);
        }
    };

    /** @struct BindTRS
     *  @brief バインド姿勢のTRS情報
     */
    struct BindTRS
    {
        aiVector3D translation = { 0.0f, 0.0f, 0.0f };
        aiQuaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
        aiVector3D scale = { 1.0f, 1.0f, 1.0f };
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

        void SetSkeletonCache(std::unique_ptr<Graphics::Import::SkeletonCache> _cache)
        {
            this->skeletonCache = std::move(_cache);
        }

        Graphics::Import::SkeletonCache* GetSkeletonCache() const
        {
            return this->skeletonCache.get();
        }

    private:
        std::unique_ptr<Graphics::Import::ModelData> modelData = nullptr;
        std::unique_ptr<Graphics::Import::SkeletonCache> skeletonCache = nullptr;
    };

    /** @brief モデル情報
    */
    struct ModelInfo
    {
        const std::string filename;     ///< モデルファイル名
        const std::string textureDir;   ///< テクスチャディレクトリ
    };
}