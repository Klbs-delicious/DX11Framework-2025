/** @file   VertexBuffer.h
 *  @date   2025/10/18
 */
#pragma once
#include "Include/Framework/Graphics/BufferBase.h"

 /** @class VertexBuffer
  *  @brief DirectX11 の頂点バッファクラス
  *  @details
  *  - 頂点データをGPUへ転送し、InputAssemblerステージにバインドする
  *  - BufferBaseを継承し、共通の生成・更新処理を利用する
  */
class VertexBuffer : public BufferBase
{
public:
    VertexBuffer();
    ~VertexBuffer() override = default;

    /** @brief 頂点バッファを作成する
     *  @param ID3D11Device* _device	D3D11デバイス
     *  @param const void* _vertexData	頂点配列データ
     *  @param UINT _vertexSize			1頂点あたりのサイズ（stride）
     *  @param UINT _vertexCount		頂点数
     *  @param bool _dynamic			trueならDynamicバッファとして生成
     *  @return bool					生成に成功したらtrue
     */
    bool Create(
        ID3D11Device* _device,
        const void* _vertexData,
        UINT _vertexSize,
        UINT _vertexCount,
        bool _dynamic = false
    );

    /** @brief 頂点バッファをIAステージにバインドする
     *  @param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
     *  @param UINT _slot						スロット番号（通常0）
     */
    void Bind(ID3D11DeviceContext* _context, UINT _slot = 0) const;

    /// @brief 頂点数を取得する
    UINT GetVertexCount() const { return this->vertexCount; }

private:
    UINT stride;        ///< 頂点1つ分のサイズ
    UINT offset;        ///< オフセット（通常0）
    UINT vertexCount;   ///< 頂点数
};
