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

namespace Graphics::Import
{
    struct Vertex {
        std::string meshName;           // メッシュ名
        aiVector3D pos;                 // 位置
        aiVector3D normal;              // 法線
        aiColor4D color;                // 頂点カラー
        aiVector3D texCoord;            // テクスチャ座標
        int materialIndex;              // マテリアルインデックス
        std::string materialName;       // マテリアル名

        int boneIndex[4];               // ボーンインデックス
        float boneWeight[4];            // ボーンウェイト
        std::string boneName[4];        // ボーン名
        int boneCount = 0;              // ボーン数
    };

    struct Subset {
        std::string meshName;           // メッシュ名
        int materialIndex;              // マテリアルインデックス
        unsigned int vertexBase;        // 頂点バッファのベース
        unsigned int vertexNum;         // 頂点数
        unsigned int indexBase;         // インデックスバッファのベース
        unsigned int indexNum;          // インデックス数
        std::string materialName;       // マテリアル名
    };

    struct Material {
        std::string materialName;       // マテリアル名
        aiColor4D ambient;              // アンビエント
        aiColor4D diffuse;              // ディフューズ
        aiColor4D specular;             // スペキュラ
        aiColor4D emission;             // エミッション
        float shiness;                  // シャイネス
        std::string diffuseTextureName; // ディフューズテクスチャ名
    };

    // ウェイト情報
    struct Weight {
        std::string boneName;           // ボーン名
        std::string meshName;           // メッシュ名
        float weight;                   // ウェイト値
        int vertexIndex;                // 頂点インデックス
    };

    // ボーン構造体
    struct Bone {
        std::string boneName;           // ボーン名
        std::string meshName;           // メッシュ名
        std::string armatureName;       // アーマチュア名
        aiMatrix4x4 matrix{};           // 親子関係を考慮した行列
        aiMatrix4x4 animationMatrix{};  // 自分の事だけを考えた行列
        aiMatrix4x4 offsetMatrix{};     // ボーンオフセット行列
        int index;                      // 配列の何番目か
        std::vector<Weight> weights;    // このボーンが影響を与える頂点インデックス・ウェイト値
    };

    struct ModelData
    {
        std::vector<std::vector<Vertex>> vertices;
        std::vector<std::vector<unsigned int>> indices;
        std::vector<Subset> subsets;
        std::vector<Material> materials;
        std::vector<std::unique_ptr<TextureResource>> diffuseTextures;
        std::unordered_map<std::string, Bone> boneDictionary;
        Utils::TreeNode<std::string> boneTree;
    };
}