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
    struct Vertex
    {
        std::string meshname;
        aiVector3D pos;
        aiVector3D normal;
        aiColor4D  color;
        aiVector3D texcoord;
        int materialIndex;
        std::string materialName;

        int BoneIndex[4]{};
        float BoneWeight[4]{};
        std::string BoneName[4];
        int boneCount = 0;
    };

    struct Subset
    {
        std::string meshname;
        int materialIndex;
        unsigned int vertexBase;
        unsigned int vertexCount;
        unsigned int indexBase;
        unsigned int indexCount;
        std::string materialName;
    };

    struct Material
    {
        std::string name;
        aiColor4D Ambient;
        aiColor4D Diffuse;
        aiColor4D Specular;
        aiColor4D Emission;
        float Shininess = 0.0f;
        std::string diffuseTextureName;
    };

    struct Weight
    {
        std::string boneName;
        std::string meshName;
        float weight;
        int vertexIndex;
    };

    struct Bone
    {
        std::string boneName;
        std::string meshName;
        std::string armatureName;
        aiMatrix4x4 Matrix{};
        aiMatrix4x4 AnimationMatrix{};
        aiMatrix4x4 OffsetMatrix{};
        int index = 0;
        std::vector<Weight> weights;
    };

    struct ModelData
    {
        std::vector<std::vector<Vertex>> vertices;
        std::vector<std::vector<unsigned int>> indices;
        std::vector<Subset> subsets;
        std::vector<Material> materials;
        std::vector<std::unique_ptr<TextureResource>> diffuseTextures;
        std::unordered_map<std::string, Bone> boneDictionary;
        Framework::Utils::TreeNode<std::string> boneTree;
    };
}