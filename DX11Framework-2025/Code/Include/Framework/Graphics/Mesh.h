/** @file   Mesh.h
 *  @date   2025/11/01
 */
#pragma once
#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/IndexBuffer.h"
#include "Include/Framework/Graphics/Material.h"
#include "Include/Framework/Graphics/VertexTypes.h"

#include "Include/Framework/Shaders/ShaderManager.h"

#include <memory>
#include <vector>

namespace Graphics
{
	struct MeshSubset
	{
		UINT indexStart = 0;
		UINT indexCount = 0;
		UINT vertexBase = 0;
		UINT vertexCount = 0;
		int materialIndex = -1;
	};

	class Mesh
	{
	public:
		~Mesh() = default;

		void Bind(ID3D11DeviceContext& _context) const;

		const std::vector<MeshSubset>& GetSubsets() const { return this->subsets; }
		const IndexBuffer& GetIndex() const { return *this->indexBuffer.get(); }

		void SetVertexBuffer(std::unique_ptr<VertexBuffer> _vb) { this->vertexBuffer = std::move(_vb); }
		void SetIndexBuffer(std::unique_ptr<IndexBuffer> _ib) { this->indexBuffer = std::move(_ib); }
		void SetSubsets(std::vector<MeshSubset>&& _subsets) { this->subsets = std::move(_subsets); }

		//-----------------------------------------------------------------------------
		// Debug / CPU cache (for diagnostics)
		//-----------------------------------------------------------------------------

		/**@brief デバッグ用：CPU側頂点配列を設定（boneIndex の範囲チェック等に使用）
		 * @param _vertices CPU頂点配列
		 */
		void SetCpuVertices(std::vector<ModelVertexGPU>&& _vertices) { this->cpuVertices = std::move(_vertices); }

		/**@brief デバッグ用：CPU側頂点配列を取得
		 * @return CPU頂点配列
		 */
		const std::vector<ModelVertexGPU>& GetCpuVertices() const { return this->cpuVertices; }

	private:
		std::unique_ptr<VertexBuffer> vertexBuffer;
		std::unique_ptr<IndexBuffer> indexBuffer;
		std::vector<MeshSubset> subsets;

		// デバッグ用：読み込み時に保持しておく CPU頂点
		std::vector<ModelVertexGPU> cpuVertices;
	};
}