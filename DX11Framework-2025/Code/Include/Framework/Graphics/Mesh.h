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

        /**@brief メッシュをバインド
         * @param _context
         */
        void Bind(ID3D11DeviceContext& _context) const;

        /**@brief 各サブセット情報の取得
         * @return
         */
        const std::vector<MeshSubset>& GetSubsets() const { return this->subsets; }

		/**@brief インデックスバッファの取得
         * @return 
         */
        const  IndexBuffer& GetIndex() const { return *this->indexBuffer.get(); }

        /**@brief 頂点バッファの設定
         * @param _vb 
         */
        void SetVertexBuffer(std::unique_ptr<VertexBuffer> _vb) { this->vertexBuffer = std::move(_vb); }
        
        /**@brief インデックスバッファの設定
         * @param _ib 
         */
        void SetIndexBuffer(std::unique_ptr<IndexBuffer> _ib) { this->indexBuffer = std::move(_ib); }

		/**@brief サブセット情報の設定
         * @param _subsets 
         */
        void SetSubsets(std::vector<MeshSubset>&& _subsets) { this->subsets = std::move(_subsets); }

    private:
        std::unique_ptr<VertexBuffer> vertexBuffer;         ///< 頂点バッファ
        std::unique_ptr<IndexBuffer> indexBuffer;           ///< インデックスバッファ
        std::vector<MeshSubset> subsets;                    ///< サブセット情報
    };
}// namespace Graphics