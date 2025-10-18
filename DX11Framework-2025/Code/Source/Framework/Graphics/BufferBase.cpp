/** @file   BufferBase.cpp
 *  @date   2025/10/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/BufferBase.h"

#include <cassert>

//-----------------------------------------------------------------------------
// BufferBase class
//-----------------------------------------------------------------------------

BufferBase::BufferBase() :
	buffer(nullptr),
	byteWidth(0),
	bindFlags(0),
	usage(D3D11_USAGE_DEFAULT),
	cpuAccess(0)
{}

BufferBase::~BufferBase()
{
	this->Release();
}

/**	@brief バッファを作成する
 *	@param ID3D11Device* _device		D3D11のデバイス
 *	@param UINT _byteWidth				バッファの総バイトサイズ
 *	@param UINT _bindFlags				バインドフラグ（例: D3D11_BIND_VERTEX_BUFFER）
 *	@param D3D11_USAGE _usage			使用モード（例: D3D11_USAGE_DEFAULT）
 *	@param UINT _cpuAccess				CPUアクセス権限（例: D3D11_CPU_ACCESS_WRITE）
 *	@param const void* _initData		初期データ（nullptr可）
 *	@return bool						生成に成功したら true
 */
bool BufferBase::Create(
	ID3D11Device* _device,
	UINT _byteWidth,
	UINT _bindFlags,
	D3D11_USAGE _usage,
	UINT _cpuAccess,
	const void* _initData
)
{
	assert(_device);

	this->byteWidth = _byteWidth;
	this->bindFlags = _bindFlags;
	this->usage = _usage;
	this->cpuAccess = _cpuAccess;

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = this->byteWidth;
	desc.Usage = this->usage;
	desc.BindFlags = this->bindFlags;
	desc.CPUAccessFlags = this->cpuAccess;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	D3D11_SUBRESOURCE_DATA* pInit = nullptr;

	if (_initData)
	{
		initData.pSysMem = _initData;
		pInit = &initData;
	}

	HRESULT hr = _device->CreateBuffer(&desc, pInit, this->buffer.ReleaseAndGetAddressOf());
	if (FAILED(hr)) { OutputDebugStringA("BufferBase::Create failed\n"); }

	return SUCCEEDED(hr);
}

/**	@brief バッファ内容を更新する
 *	@param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
 *	@param const void* _data				書き込み元データ
 *	@param size_t _size						データサイズ（バイト）
 *	@note D3D11_USAGE_DYNAMIC の場合のみ有効
 */
void BufferBase::Update(ID3D11DeviceContext* _context, const void* _data, size_t _size)
{
	assert(_context);
	assert(this->buffer);

	// Dynamicバッファ専用
	if (this->usage != D3D11_USAGE_DYNAMIC)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = _context->Map(this->buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		memcpy(mapped.pData, _data, _size);
		_context->Unmap(this->buffer.Get(), 0);
	}
}

///	@brief バッファを解放する
void BufferBase::Release()
{
	this->buffer.Reset();
}