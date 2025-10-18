/** @file   IndexBuffer.h
 *  @date   2025/10/18
 */
#pragma once
#include "Include/Framework/Graphics/BufferBase.h"

 /** @class IndexBuffer
  *  @brief DirectX11 のインデックスバッファクラス
  *  @details
  *     - 頂点の参照順序（インデックス）をGPUへ転送する
  *     - BufferBaseを継承し、IASetIndexBuffer() でバインドする
  */
class IndexBuffer : public BufferBase
{
public:
    IndexBuffer();
    ~IndexBuffer() override = default;

    /** @brief インデックスバッファを作成する
     *  @param ID3D11Device* _device	D3D11デバイス
     *  @param const void* _indexData	インデックスデータ配列
     *  @param UINT _indexSize			1インデックスあたりのバイト数 (2 or 4)
     *  @param UINT _indexCount			インデックス数
     *  @return bool					生成に成功したら true
     */
    bool Create(
        ID3D11Device* _device,
        const void* _indexData,
        UINT _indexSize,
        UINT _indexCount
    );

    /** @brief インデックスバッファをIAステージにバインドする
     *  @param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
     */
    void Bind(ID3D11DeviceContext* _context) const;

    /** @brief  インデックス数を取得する
     *  @return  UINT   インデックス数
     */
    UINT GetIndexCount() const { return this->indexCount; }

private:
    DXGI_FORMAT format; ///< インデックス型 (R16_UINT or R32_UINT)
    UINT indexCount;    ///< インデックス数
};
