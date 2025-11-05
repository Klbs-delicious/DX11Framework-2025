/** @file   Mesh.h
 *  @date   2025/11/01
 */
#pragma once
#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/IndexBuffer.h"
#include "Include/Framework/Graphics/Material.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Shaders/ShaderManager.h"

#include <memory>
#include <vector>

namespace Graphics
{
    struct ModelVertexGPU
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texcoord;

        UINT  boneIndex[4] = { 0,0,0,0 };
        float boneWeight[4] = { 0,0,0,0 };
    };

    /** @struct MeshSubset
     *  @brief サブセット情報（1つのマテリアルに対応）
     */
    struct MeshSubset
    {
        UINT indexStart = 0;    ///< 描画開始インデックス
        UINT indexCount = 0;    ///< 描画インデックス数
        UINT vertexBase = 0;    ///< 頂点開始位置
        UINT vertexCount = 0;   ///< 頂点数
        int materialIndex = -1; ///< 使用マテリアルインデックス
    };

    /** @class Mesh
     *  @brief モデルデータをGPUで扱うための構造
     */
    class Mesh
    {
    public:
        ~Mesh() = default;

        /**@brief メッシュ生成（頂点・インデックス・サブセットを統合）
         * @param _device
         * @param _modelData 読み込んだモデル情報
         */
        void Create(ID3D11Device* _device, ID3D11DeviceContext* _context, ShaderManager* _shaderManager, const Graphics::Import::ModelData& _modelData);

        /**@brief メッシュをバインド
         * @param _context
         */
        void Bind(ID3D11DeviceContext& _context) const;

        /**@brief 各サブセット情報の取得
         * @return
         */
        const std::vector<MeshSubset>& GetSubsets() const { return this->subsets; }

        /**@brief 特定のマテリアルを取得する
         * @param _index マテリアルのインデックス
         * @return Material* マテリアルが存在しなければ nullptr を返す
         */
        Material* GetMaterial(size_t _index) const
        {
            return _index < this->materials.size() ? this->materials[_index].get() : nullptr;
        }

        /**@brief マテリアルを追加する
         * @param _mat マテリアル情報
         */
        void AddMaterial(std::unique_ptr<Material> _mat)
        {
            this->materials.emplace_back(std::move(_mat));
        }

    private:
        std::unique_ptr<VertexBuffer> vertexBuffer;         ///< 頂点バッファ
        std::unique_ptr<IndexBuffer> indexBuffer;           ///< インデックスバッファ
        std::vector<MeshSubset> subsets;                    ///< サブセット情報
        std::vector<std::unique_ptr<Material>> materials;   ///< マテリアル群
    };
}// namespace Graphics
