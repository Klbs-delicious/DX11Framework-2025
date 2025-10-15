/** @file   VertexShader.cpp
*   @date   2025/10/05
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Shaders/VertexShader.h"

//-----------------------------------------------------------------------------
// VertexShader class
//-----------------------------------------------------------------------------
VertexShader::VertexShader() :vertexShader(nullptr), inputLayout(nullptr) {}
VertexShader:: ~VertexShader()
{
	this->vertexShader.Reset();
}

/**	@brief シェーダーのバインド
 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
 */
void VertexShader::Bind(ID3D11DeviceContext& _context)
{
	_context.VSSetShader(this->vertexShader.Get() , nullptr, 0);
}

/**	@brief シェーダーのバインドを解除
　*	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
　*/
void VertexShader::Unbind(ID3D11DeviceContext& _context)
{
	_context.VSSetShader(nullptr, nullptr, 0);
}

/**	@brief シェーダーの生成
 *	@param ID3D11Device& _device	D3D11のデバイス
 *  @param ShaderInfo _shaderInfo シェーダー情報
 *	@return bool シェーダーの生成に成功したら true
 */
bool VertexShader::CreateShader(ID3D11Device& _device, const ShaderInfo _shaderInfo)
{
	// シェーダーファイルからバイナリデータを読み込む
	this->LoadShader(_device, _shaderInfo);
	if (!this->blob) { return false; }

	// 頂点シェーダーの生成
	HRESULT hr = _device.CreateVertexShader(
		this->blob->GetBufferPointer(),
		this->blob->GetBufferSize(),
		nullptr,
		this->vertexShader.GetAddressOf()
	);
	if (FAILED(hr))
	{
		if (this->blob)this->blob->Release();
		return false;
	}
	return true;
}