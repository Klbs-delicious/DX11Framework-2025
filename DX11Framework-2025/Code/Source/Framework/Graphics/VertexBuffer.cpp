/** @file   VertexBuffer.cpp
 *  @date   2025/10/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/VertexBuffer.h"

#include <cassert>

//-----------------------------------------------------------------------------
// VertexBuffer class
//-----------------------------------------------------------------------------
VertexBuffer::VertexBuffer() :
	stride(0),
	offset(0),
	vertexCount(0)
{}

/** @brief 頂点バッファを作成する
 *	@param ID3D11Device* _device		D3D11のデバイス
 *	@param const void* _vertexData		頂点データ配列
 *	@param UINT _vertexSize				1頂点あたりのサイズ
 *	@param UINT _vertexCount			頂点数
 *	@param bool _dynamic				trueならDynamicバッファで作成
 *	@return bool						生成に成功したらtrue
 */
bool VertexBuffer::Create(
	ID3D11Device* _device,
	const void* _vertexData,
	UINT _vertexSize,
	UINT _vertexCount,
	bool _dynamic
)
{
	assert(_device);

	this->stride = _vertexSize;
	this->offset = 0;
	this->vertexCount = _vertexCount;

	UINT byteWidth = _vertexSize * _vertexCount;
	UINT bindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_USAGE usage = _dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	UINT cpuAccess = _dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	// バッファの作成
	return BufferBase::Create(_device, byteWidth, bindFlags, usage, cpuAccess, _vertexData);
}

/** @brief 頂点バッファをIAステージにバインドする
 *	@param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
 *	@param UINT _slot						スロット番号（通常0）
 */
void VertexBuffer::Bind(ID3D11DeviceContext* _context, UINT _slot) const
{
	assert(_context);
	assert(this->buffer);

	_context->IASetVertexBuffers(
		_slot,
		1,
		this->buffer.GetAddressOf(),
		&this->stride,
		&this->offset
	);
}