/**	@file	ModelImporter.h
*	@date	2025/10/26
*/
#pragma once
#include "Include/Framework/Utils/TreeNode.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/TextureLoader.h"

#include <assimp/scene.h>
#include <memory>

namespace Graphics::Import
{
    class ModelImporter
    {
        using TreeNode_t = Framework::Utils::TreeNode<std::string>;

    public:
        ModelImporter();
        ~ModelImporter();
        bool Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel);

    private:
        void CreateNodeTree(aiNode* _node, TreeNode_t* _tree);
        void CreateEmptyBoneDictionary(aiNode* _node, std::unordered_map<std::string, Bone>& _boneDict);
        std::vector<Bone> GetBonesPerMesh(const aiMesh* _mesh, std::unordered_map<std::string, Bone>& _boneDict);
        void SetBoneDataToVertices(ModelData& _model);
        void GetBone(const aiScene* _scene, ModelData& _model);
        void GetMaterialData(const aiScene* _scene, const std::string& _textureDir, ModelData& _model);

        std::unique_ptr<TextureLoader> textureLoader;   ///< テクスチャ読み込み
    };
}