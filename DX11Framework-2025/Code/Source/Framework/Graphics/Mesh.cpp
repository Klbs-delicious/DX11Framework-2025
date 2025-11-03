/** @file   Mesh.cpp
 *  @date   2025/11/01
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/Mesh.h"

//-----------------------------------------------------------------------------
// Mesh Class
//-----------------------------------------------------------------------------
namespace Graphics
{
    /**@brief メッシュを生成する（頂点・インデックス・サブセットを統合）
     * @param _device
     * @param _modelData 読み込んだモデル情報
     */
    void Mesh::Create(
        ID3D11Device* _device,
        ID3D11DeviceContext* _context,
        ShaderManager* _shaderManager,
        const Graphics::Import::ModelData& _modelData)
    {
        // --- サブセット構築 ---
        this->subsets.clear();
        this->subsets.reserve(_modelData.subsets.size());
        for (const auto& subset : _modelData.subsets)
        {
            // NOTE: Assimp のメッシュが独立した頂点を持つ場合は vertexBase による補正が必要
            MeshSubset meshSubset{};
            meshSubset.indexStart = subset.indexBase;
            meshSubset.indexCount = subset.indexCount;
            meshSubset.vertexBase = subset.vertexBase;
            meshSubset.vertexCount = subset.vertexCount;
            meshSubset.materialIndex = subset.materialIndex;
            this->subsets.push_back(meshSubset);
        }

        // --- 頂点統合 ---
        // 総頂点数を事前にカウントして実際のサイズで確保する
        size_t totalVertexCount = 0;
        for (const auto& meshVertices : _modelData.vertices)
        {
            totalVertexCount += meshVertices.size();
        }
        std::vector<ModelVertexGPU> vertexData;
        vertexData.reserve(totalVertexCount);

        for (const auto& meshVertices : _modelData.vertices)
        {
            for (const auto& vertex : meshVertices)
            {
                ModelVertexGPU vtx{};
                vtx.position = { vertex.pos.x, vertex.pos.y, vertex.pos.z };
                vtx.normal = { vertex.normal.x, vertex.normal.y, vertex.normal.z };
                vtx.texcoord = { vertex.texcoord.x, vertex.texcoord.y };
                vertexData.push_back(vtx);
            }
        }

        // --- 頂点バッファ作成 ---
        this->vertexBuffer = std::make_unique<VertexBuffer>();
        this->vertexBuffer->Create(_device, vertexData.data(), sizeof(ModelVertexGPU),
            static_cast<UINT>(vertexData.size()), false);

        // --- インデックス統合 ---
        size_t totalIndexCount = 0;
        for (const auto& meshIndices : _modelData.indices)
        {
            totalIndexCount += meshIndices.size();
        }

        std::vector<UINT> indexData;
        indexData.reserve(totalIndexCount);

        for (const auto& meshIndices : _modelData.indices)
        {
            indexData.insert(indexData.end(), meshIndices.begin(), meshIndices.end());
        }

        // --- インデックスバッファ作成 ---
        this->indexBuffer = std::make_unique<IndexBuffer>();
        this->indexBuffer->Create(_device, indexData.data(),
            static_cast<UINT>(indexData.size()), false);

        // --- マテリアル作成 ---
        this->materials.reserve(_modelData.materials.size());
        for (size_t i = 0; i < _modelData.materials.size(); ++i)
        {
            const auto& srcMat = _modelData.materials[i];
            auto mat = std::make_unique<Material>();

            mat->shaders = _shaderManager->GetShaderProgram("ModelTest"); // 統一命名推奨

            MaterialParams params{};

            if (i < _modelData.diffuseTextures.size() && _modelData.diffuseTextures[i])
            {
                mat->albedoMap = _modelData.diffuseTextures[i].get();
                params.TextureEnable = TRUE;
            }
            else
            {
                mat->albedoMap = nullptr;
                params.TextureEnable = FALSE;
            }

            params.Ambient = DX::Color(srcMat.Ambient.r, srcMat.Ambient.g, srcMat.Ambient.b, 1.0f);
            params.Diffuse = DX::Color(srcMat.Diffuse.r, srcMat.Diffuse.g, srcMat.Diffuse.b, 1.0f);
            params.Specular = DX::Color(srcMat.Specular.r, srcMat.Specular.g, srcMat.Specular.b, 1.0f);
            params.Emission = DX::Color(srcMat.Emission.r, srcMat.Emission.g, srcMat.Emission.b, 1.0f);
            params.Shiness = srcMat.Shininess;

            mat->materialBuffer->Update(_context,params);
            this->materials.emplace_back(std::move(mat));
        }
    }

    /**@brief メッシュをバインド
     * @param _context
     */
    void Mesh::Bind(ID3D11DeviceContext& _context) const
    {
        // 頂点・インデックスを IA ステージに設定
        if (this->vertexBuffer)
        {
            this->vertexBuffer->Bind(&_context);
		}
        if (this->indexBuffer)
        {
            this->indexBuffer->Bind(&_context);
        }
    }
}// namespace Graphics
