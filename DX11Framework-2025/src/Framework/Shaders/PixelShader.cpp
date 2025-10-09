/** @file   VertexShader.cpp
*   @date   2025/10/05
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Shaders/PixelShader.h"

//-----------------------------------------------------------------------------
// VertexShader class
//-----------------------------------------------------------------------------
PixelShader::PixelShader() :pixelShader(nullptr) {}
PixelShader:: ~PixelShader()
{
	this->pixelShader.Reset();
}

/**	@brief シェーダーのバインド
 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
 */
void PixelShader::Bind(ID3D11DeviceContext& _context)
{
	_context.PSSetShader(this->pixelShader.Get(), nullptr, 0);
}

/**	@brief シェーダーのバインドを解除
　*	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
　*/
void PixelShader::Unbind(ID3D11DeviceContext& _context)
{
	_context.PSSetShader(nullptr, nullptr, 0);
}

/**	@brief シェーダーの生成
 *	@param ID3D11Device& _device	D3D11のデバイス
 *  @param ShaderInfo _shaderInfo シェーダー情報
 *	@return bool シェーダーの生成に成功したら true
 */
bool PixelShader::CreateShader(ID3D11Device& _device, const ShaderInfo _shaderInfo)
{	
	// シェーダーファイルからバイナリデータを読み込む
	this->LoadShader(_device, _shaderInfo);
	if (!this->blob) { return false; }

	// ピクセルシェーダーの生成
	HRESULT hr = _device.CreatePixelShader(
		this->blob->GetBufferPointer(),
		this->blob->GetBufferSize(),
		nullptr,
		this->pixelShader.GetAddressOf()
	);
	if (FAILED(hr))
	{
		if (this->blob)this->blob->Release();
		return false;
	}
	return true;
}