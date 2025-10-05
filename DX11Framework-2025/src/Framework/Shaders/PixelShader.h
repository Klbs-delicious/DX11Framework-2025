/**@file   PixelShader.h
 * @date   2025/10/05
 */
#pragma once
#include"Framework/Utils/CommonTypes.h"
#include"Framework/Shaders/ShaderBase.h"

 /** @class	PixelShader
  *	 @brief	頂点シェーダークラス
  */
class PixelShader : public ShaderBase
{
public:
	PixelShader();
	~PixelShader();

	/**	@brief シェーダーのバインド
	 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
	 */
	void Bind(ID3D11DeviceContext& _context) override;

	/**	@brief シェーダーのバインドを解除
	 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
	 */
	void Unbind(ID3D11DeviceContext& _context) override;

	/**	@brief シェーダーの生成
	 *	@param ID3D11Device& _device	D3D11のデバイス
	 *	@return bool シェーダーの生成に成功したら true
	 */
	bool CreateShader(ID3D11Device& _device, std::wstring& _fileName) override;

private:
	DX::ComPtr<ID3D11PixelShader> pixelShader;	///< ピクセルシェーダー
};
