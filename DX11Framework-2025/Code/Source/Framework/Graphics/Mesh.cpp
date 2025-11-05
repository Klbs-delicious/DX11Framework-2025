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
            MeshSubset meshSubset{};
            meshSubset.indexStart = subset.indexBase;
            meshSubset.indexCount = subset.indexNum;
            meshSubset.vertexBase = subset.vertexBase;
            meshSubset.vertexCount = subset.vertexNum;
            meshSubset.materialIndex = subset.materialIndex;
            this->subsets.push_back(meshSubset);
        }

        // --- 頂点・インデックス統合 ---
        std::vector<ModelVertexGPU> vertexData;
        std::vector<UINT> indexData;

        vertexData.reserve(100000); // 適宜
        indexData.reserve(300000);

        unsigned int vertexOffset = 0;

        for (size_t meshIndex = 0; meshIndex < _modelData.vertices.size(); ++meshIndex)
        {
            const auto& meshVertices = _modelData.vertices[meshIndex];
            const auto& meshIndices = _modelData.indices[meshIndex];

            // 頂点統合
            for (const auto& v : meshVertices)
            {
                ModelVertexGPU gpu{};
                gpu.position = { v.pos.x, v.pos.y, v.pos.z };
                gpu.normal = { v.normal.x, v.normal.y, v.normal.z };
                gpu.texcoord = { v.texCoord.x, v.texCoord.y };
                vertexData.push_back(gpu);
            }

            // インデックス統合（頂点オフセットを考慮）
            for (const auto& i : meshIndices)
                indexData.push_back(i + vertexOffset);

            vertexOffset += static_cast<unsigned int>(meshVertices.size());
        }

        // --- 頂点バッファ生成 ---
        this->vertexBuffer = std::make_unique<VertexBuffer>();
        this->vertexBuffer->Create(
            _device,
            vertexData.data(),
            sizeof(ModelVertexGPU),
            static_cast<UINT>(vertexData.size()),
            false);

        // --- インデックスバッファ生成 ---
        this->indexBuffer = std::make_unique<IndexBuffer>();
        this->indexBuffer->Create(
            _device,
            indexData.data(),
            sizeof(UINT),
            static_cast<UINT>(indexData.size()));

        // --- マテリアル生成 ---
        this->materials.reserve(_modelData.materials.size());
        for (size_t i = 0; i < _modelData.materials.size(); ++i)
        {
            const auto& srcMat = _modelData.materials[i];
            auto mat = std::make_unique<Material>();

            mat->shaders = _shaderManager->GetShaderProgram("ModelBasic");

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

            params.Ambient = DX::Color(srcMat.ambient.r, srcMat.ambient.g, srcMat.ambient.b, 1.0f);
            params.Diffuse = DX::Color(srcMat.diffuse.r, srcMat.diffuse.g, srcMat.diffuse.b, 1.0f);
            params.Specular = DX::Color(srcMat.specular.r, srcMat.specular.g, srcMat.specular.b, 1.0f);
            params.Emission = DX::Color(srcMat.emission.r, srcMat.emission.g, srcMat.emission.b, 1.0f);
            params.Shiness = srcMat.shiness;

            mat->materialBuffer->Create(_device);
            mat->materialBuffer->Update(_context, params);
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
