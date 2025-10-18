/** @file   ConstantBuffer.h
 *  @date   2025/10/18
 */
#pragma once
#include "Include/Framework/Graphics/BufferBase.h"

 /** @class DynamicConstantBuffer
  *  @brief 毎フレーム更新を前提とした動的定数バッファ
  */
template<typename T>
class DynamicConstantBuffer : public BufferBase
{
public:
    DynamicConstantBuffer() = default;
    ~DynamicConstantBuffer() override = default;

    /** @brief 定数バッファを作成する
     *  @param ID3D11Device* _device	D3D11デバイス
     */
    bool Create(ID3D11Device* _device)
    {
        UINT byteWidth = sizeof(T);
        byteWidth = (byteWidth + 15) & ~15;

        return BufferBase::Create(
            _device,
            byteWidth,
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC,        // ← 動的
            D3D11_CPU_ACCESS_WRITE,     // ← CPU書き込み許可
            nullptr
        );
    }

    /** @brief 定数データを更新する
     *  @param ID3D11DeviceContext* _context	D3D11コンテキスト
     *  @param const T& _data					更新するデータ
     */
    void Update(ID3D11DeviceContext* _context, const T& _data)
    {
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        _context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, &_data, sizeof(T));
        _context->Unmap(this->buffer.Get(), 0);
    }

    /** @brief 頂点シェーダへ定数バッファをバインドする
     *  @param ID3D11DeviceContext* _context	D3D11コンテキスト
     *  @param UINT _slot						スロット番号（b#）
     */
    void BindVS(ID3D11DeviceContext* _context, UINT _slot) const
    {
        _context->VSSetConstantBuffers(_slot, 1, this->buffer.GetAddressOf());
    }

    /** @brief ピクセルシェーダへ定数バッファをバインドする
     *  @param ID3D11DeviceContext* _context	D3D11コンテキスト
     *  @param UINT _slot						スロット番号（b#）
     */
    void BindPS(ID3D11DeviceContext* _context, UINT _slot) const
    {
        _context->PSSetConstantBuffers(_slot, 1, this->buffer.GetAddressOf());
    }
};

