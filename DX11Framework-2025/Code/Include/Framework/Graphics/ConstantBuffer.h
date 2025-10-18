/** @file   ConstantBuffer.h
 *  @date   2025/10/18
 */
#pragma once
#include "Include/Framework/Graphics/BufferBase.h"

 /** @class ConstantBuffer
  *  @brief 定数バッファクラス
  *  @details
  *  - 静的オブジェクトを更新するバッファとして想定
  */
template<typename T>
class ConstantBuffer : public BufferBase
{
public:
    ConstantBuffer() = default;
    ~ConstantBuffer() override = default;

    /** @brief 定数バッファを作成する
     *  @param ID3D11Device* _device	D3D11デバイス
     */
    bool Create(ID3D11Device* _device)
    {
        UINT byteWidth = sizeof(T);
        byteWidth = (byteWidth + 15) & ~15; // 16バイト単位に切り上げ

        return BufferBase::Create(
            _device,
            byteWidth,
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DEFAULT,   
            0,                     
            nullptr
        );
    }

    /** @brief 定数データを更新する
     *  @param ID3D11DeviceContext* _context	D3D11コンテキスト
     *  @param const T& _data					更新するデータ
     */
    void Update(ID3D11DeviceContext* _context, const T& _data)
    {
        _context->UpdateSubresource(this->buffer.Get(), 0, nullptr, &_data, 0, 0);
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
