/**@file   VertexShader.h
 * @date   2025/10/05
 */
#pragma once
#include"Framework/Utils/CommonTypes.h"
#include"Framework/Shaders/ShaderBase.h"

 /** @class	VertexShader
  *	 @brief	頂点シェーダークラス
  */
class VertexShader : public ShaderBase
{
public:
	VertexShader();
	~VertexShader();

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
	 *	@param std::wstring& _fileName	シェーダーファイル名
	 */
	void CreateShader(ID3D11Device& _device, std::wstring& _fileName) override;

	/** @brief 入力レイアウトの設定
	 *  @param ID3D11InputLayout* _inputLayout 
	 */
	void SetInputLayout(ID3D11InputLayout* _inputLayout) { this->inputLayout = _inputLayout; }

private:
	DX::ComPtr<ID3D11VertexShader> vertexShader;	///< 頂点シェーダー
	ID3D11InputLayout* inputLayout;					///< 頂点入力レイアウト
};
