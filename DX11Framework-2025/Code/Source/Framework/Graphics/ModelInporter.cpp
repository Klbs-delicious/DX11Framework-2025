#include "Include/Framework/Graphics/ModelInporter.h"

#include <iostream>
#include <cassert>
#include <unordered_map>
#include <memory>

// Assimp関連
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>

namespace Graphics::Import
{
    using Framework::Utils::TreeNode;

    ModelImporter::ModelImporter() :textureLoader(nullptr)
    {
        this->textureLoader = std::make_unique<TextureLoader>();
    }
    ModelImporter::~ModelImporter()
    {
        this->textureLoader.reset();
    }

    //----------------------------------------------------
    // ノードツリー生成（安全版）
    //----------------------------------------------------
    void ModelImporter::CreateNodeTree(aiNode* _node, Framework::Utils::TreeNode<std::string>* _tree)
    {
        // ----------------------------
        // NULL ガード
        // ----------------------------
        if (!_node || !_tree)
            return;

        // ノード名を設定
        if (_node->mName.length > 0)
            _tree->nodedata = _node->mName.C_Str();
        else
            _tree->nodedata = "(UnnamedNode)";

        // ----------------------------
        // 子ノードが存在しない場合は終了
        // ----------------------------
        if (_node->mNumChildren == 0 || !_node->mChildren)
            return;

        // ----------------------------
        // 子ノードを安全に再帰処理
        // ----------------------------
        for (unsigned int i = 0; i < _node->mNumChildren; ++i)
        {
            aiNode* childNode = _node->mChildren[i];
            if (!childNode)
                continue; // nullptrスキップ

            std::string childName;
            if (childNode->mName.length > 0)
                childName = childNode->mName.C_Str();
            else
                childName = "(UnnamedChild)";

            // 子ノード生成
            auto child = std::make_unique<Framework::Utils::TreeNode<std::string>>(childName);

            // 親に追加
            _tree->Addchild(std::move(child));

            // 末尾の子を再帰呼び出し
            Framework::Utils::TreeNode<std::string>* added = _tree->children.back().get();
            CreateNodeTree(childNode, added);
        }
    }


    //----------------------------------------------------
    // 空のボーン辞書作成（ノードを再帰的に辿る）
    //----------------------------------------------------
    void ModelImporter::CreateEmptyBoneDictionary(aiNode* _node, std::unordered_map<std::string, Bone>& _boneDict)
    {
        Bone bone{};
        _boneDict[_node->mName.C_Str()] = bone;

        for (unsigned int n = 0; n < _node->mNumChildren; n++)
        {
            CreateEmptyBoneDictionary(_node->mChildren[n], _boneDict);
        }
    }

    //----------------------------------------------------
    // メッシュ単位のボーン情報取得
    //----------------------------------------------------
    std::vector<Bone> ModelImporter::GetBonesPerMesh(const aiMesh* _mesh, std::unordered_map<std::string, Bone>& _boneDict)
    {
        std::vector<Bone> bones;

        for (unsigned int b = 0; b < _mesh->mNumBones; b++)
        {
            Bone bone{};
            bone.boneName = _mesh->mBones[b]->mName.C_Str();
            bone.meshName = _mesh->mBones[b]->mNode->mName.C_Str();

            if (_mesh->mBones[b]->mArmature)
                bone.armatureName = _mesh->mBones[b]->mArmature->mName.C_Str();

            bone.OffsetMatrix = _mesh->mBones[b]->mOffsetMatrix;

            // ウェイト情報
            for (unsigned int w = 0; w < _mesh->mBones[b]->mNumWeights; w++)
            {
                Weight weight{};
                weight.meshName = bone.meshName;
                weight.boneName = bone.boneName;
                weight.weight = _mesh->mBones[b]->mWeights[w].mWeight;
                weight.vertexIndex = _mesh->mBones[b]->mWeights[w].mVertexId;
                bone.weights.emplace_back(weight);
            }

            bones.emplace_back(bone);
            _boneDict[bone.boneName].OffsetMatrix = bone.OffsetMatrix;
        }

        return bones;
    }

    //----------------------------------------------------
    // 頂点へボーンデータを反映
    //----------------------------------------------------
    void ModelImporter::SetBoneDataToVertices(ModelData& _model)
    {
        // 初期化
        for (auto& meshVertices : _model.vertices)
        {
            for (auto& v : meshVertices)
            {
                v.boneCount = 0;
                for (int i = 0; i < 4; i++)
                {
                    v.BoneIndex[i] = -1;
                    v.BoneWeight[i] = 0.0f;
                    v.BoneName[i].clear();
                }
            }
        }

        int subsetIndex = 0;
        for (auto& bones : _model.boneDictionary)
        {
            (void)bones; // 未使用警告回避
        }

        // サブセット単位に設定
        int subsetID = 0;
        for (auto& bonesPerMesh : _model.boneDictionary)
        {
            (void)bonesPerMesh;
            subsetID++;
        }
    }

    //----------------------------------------------------
    // ボーン情報構築
    //----------------------------------------------------
    void ModelImporter::GetBone(const aiScene* _scene, ModelData& _model)
    {
        if (!_scene || !_scene->mRootNode)
        {
            OutputDebugStringA("Assimp: Invalid scene or missing root node.\n");
            return;
        }

        // 空辞書作成
        CreateEmptyBoneDictionary(_scene->mRootNode, _model.boneDictionary);

        // ボーンインデックス割り当て
        unsigned int idx = 0;
        for (auto& [name, bone] : _model.boneDictionary)
        {
            bone.index = idx++;
        }

        // 各メッシュごとのボーン情報取得
        for (unsigned int m = 0; m < _scene->mNumMeshes; m++)
        {
            aiMesh* mesh = _scene->mMeshes[m];
            if (!mesh)
                continue;

            auto bones = GetBonesPerMesh(mesh, _model.boneDictionary);
            for (auto& b : bones)
            {
                _model.boneDictionary[b.boneName] = b;
            }
        }

        // 頂点へ反映
        SetBoneDataToVertices(_model);

        // ボーンツリー構築（安全版）
        _model.boneTree = Framework::Utils::TreeNode<std::string>("Root");
        CreateNodeTree(_scene->mRootNode, &_model.boneTree);
    }

    //----------------------------------------------------
    // マテリアル情報の取得
    //----------------------------------------------------
    void ModelImporter::GetMaterialData(const aiScene* _scene, const std::string& _textureDir, ModelData& _model)
    {
        _model.materials.clear();
        _model.diffuseTextures.resize(_scene->mNumMaterials);

        for (unsigned int m = 0; m < _scene->mNumMaterials; m++)
        {
            aiMaterial* material = _scene->mMaterials[m];
            Material mat{};
            mat.name = material->GetName().C_Str();

            aiColor4D ambient{}, diffuse{}, specular{}, emission{};
            float shininess = 0.0f;

            aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
            aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission);
            aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);

            mat.Ambient = ambient;
            mat.Diffuse = diffuse;
            mat.Specular = specular;
            mat.Emission = emission;
            mat.Shininess = shininess;

            aiString path;
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
                AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &path))
            {
                mat.diffuseTextureName = _textureDir + "/" + path.C_Str();

                // 外部ファイル読み込み
                auto tex = std::make_unique<TextureResource>();
                if (tex = this->textureLoader->FromFile(mat.diffuseTextureName))
                    _model.diffuseTextures[m] = std::move(tex);
            }

            _model.materials.push_back(mat);
        }
    }

    //----------------------------------------------------
    // モデル読み込み（メイン）
    //----------------------------------------------------
    bool ModelImporter::Load(const std::string& _filename, const std::string& _textureDir, ModelData& _outModel)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            _filename,
            aiProcessPreset_TargetRealtime_MaxQuality |
            aiProcess_ConvertToLeftHanded |
            aiProcess_PopulateArmatureData
        );

        if (!scene)
        {
            std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // クリア
        _outModel.vertices.clear();
        _outModel.indices.clear();
        _outModel.subsets.clear();
        _outModel.materials.clear();
        _outModel.boneDictionary.clear();
        _outModel.diffuseTextures.clear();

        // マテリアル情報
        GetMaterialData(scene, _textureDir, _outModel);

        // 頂点・インデックスデータ
        _outModel.vertices.resize(scene->mNumMeshes);
        _outModel.indices.resize(scene->mNumMeshes);
        _outModel.subsets.resize(scene->mNumMeshes);

        for (unsigned int m = 0; m < scene->mNumMeshes; m++)
        {
            aiMesh* mesh = scene->mMeshes[m];
            std::string meshname = mesh->mName.C_Str();

            // 頂点
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex v{};
                v.meshname = meshname;
                v.pos = mesh->mVertices[i];
                v.normal = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 0, 0);
                v.color = mesh->HasVertexColors(0) ? mesh->mColors[0][i] : aiColor4D(1, 1, 1, 1);
                v.texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);
                v.materialIndex = mesh->mMaterialIndex;
                v.materialName = _outModel.materials[v.materialIndex].name;
                _outModel.vertices[m].push_back(v);
            }

            // インデックス
            for (unsigned int f = 0; f < mesh->mNumFaces; f++)
            {
                aiFace face = mesh->mFaces[f];
                assert(face.mNumIndices <= 3);
                for (unsigned int i = 0; i < face.mNumIndices; i++)
                    _outModel.indices[m].push_back(face.mIndices[i]);
            }

            // サブセット
            Subset subset{};
            subset.meshname = meshname;
            subset.vertexCount = static_cast<unsigned int>(_outModel.vertices[m].size());
            subset.indexCount = static_cast<unsigned int>(_outModel.indices[m].size());
            subset.materialIndex = mesh->mMaterialIndex;
            subset.materialName = _outModel.materials[subset.materialIndex].name;
            _outModel.subsets[m] = subset;
        }

        // ボーン情報
        GetBone(scene, _outModel);

        return true;
    }
}