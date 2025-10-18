/** @file   IndexBuffer.cpp
 *  @date   2025/10/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/IndexBuffer.h"

#include <cassert>

//-----------------------------------------------------------------------------
// IndexBuffer class
//-----------------------------------------------------------------------------
IndexBuffer::IndexBuffer() :
	format(DXGI_FORMAT_UNKNOWN),
	indexCount(0)
{}

/** @brief インデックスバッファを作成する
 *  @param ID3D11Device* _device	D3D11デバイス
 *  @param const void* _indexData	インデックスデータ配列
 *  @param UINT _indexSize			1インデックスあたりのバイト数 (2 or 4)
 *  @param UINT _indexCount			インデックス数
 *  @return bool					生成に成功したら true
 */
bool IndexBuffer::Create(
	ID3D11Device* _device,
	const void* _indexData,
	UINT _indexSize,
	UINT _indexCount
)
{
	assert(_device);
	assert(_indexSize == 2 || _indexSize == 4);		// インデックスバッファが16bitか32bitであることを保証する

	this->indexCount = _indexCount;
	this->format = (_indexSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	UINT byteWidth = _indexSize * _indexCount;
	UINT bindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
	UINT cpuAccess = 0;

	return BufferBase::Create(_device, byteWidth, bindFlags, usage, cpuAccess, _indexData);
}

/** @brief インデックスバッファをIAステージにバインドする
 *  @param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
 */
void IndexBuffer::Bind(ID3D11DeviceContext* _context) const
{
	assert(_context);
	assert(this->buffer);

	_context->IASetIndexBuffer(
		this->buffer.Get(),
		this->format,
		0 // Offset
	);
}
