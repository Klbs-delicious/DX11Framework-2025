#include"Framework/Shaders/VertexShader.h"

VertexShader::VertexShader() :vertexShader(nullptr) {}
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
 *	@param std::wstring& _fileName	シェーダーファイル名
 */
void VertexShader::CreateShader(ID3D11Device& _device, std::wstring& _fileName)
{
	_device.CreateVertexShader(
		this->blob->GetBufferPointer(),
		this->blob->GetBufferSize(),
		nullptr,
		this->vertexShader.GetAddressOf()
	);
}

