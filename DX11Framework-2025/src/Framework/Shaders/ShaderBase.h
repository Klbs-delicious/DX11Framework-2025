/**@file   ShaderBase.h
 * @date   2025/10/05
 */
#pragma once
#include"Framework/Utils/CommonTypes.h"

#include<string>
#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")

/**	@class	ShaderBase
 *	@brief	シェーダーの基底クラス
 */
class ShaderBase
{
public:
	ShaderBase();
	~ShaderBase();

	/**	@brief シェーダーのバインド
	 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
	 */
	virtual void Bind(ID3D11DeviceContext& _context) = 0;

	/**	@brief シェーダーのバインドを解除
	 *	@param ID3D11Device& _context	D3D11のデバイスコンテキスト
	 */
	virtual void Unbind(ID3D11DeviceContext& _context) = 0;

	/**	@brief シェーダーの生成
	 *	@param ID3D11Device& _device	D3D11のデバイス
	 *	@param std::wstring& _fileName	シェーダーファイル名
	 */
	virtual void CreateShader(ID3D11Device& _device, std::wstring& _fileName) = 0;

	/**	@brief	バイナリデータの取得
	 *	@return ID3DBlob* シェーダーバイナリ
	 */
	ID3DBlob* GetBlob() { return this->blob.Get(); }
protected:
	/**	@brief	シェーダーファイルからバイナリデータを読み込む
	 *	@param ID3D11Device& _device	D3D11のデバイス
	 *	@param std::wstring& _fileName	シェーダーファイル名
	 *	@return ComPtr<ID3DBlob> シェーダーバイナリ
	 */
	void LoadShader(ID3D11Device& _device, std::wstring& _fileName);

	DX::ComPtr<ID3DBlob> blob;	///< シェーダーバイナリ

private:
	/**	@brief シェーダーのコンパイル
	 *	@param ID3D11Device& _device D3D11のデバイス
	 *	@param std::wstring& _fileName シェーダーファイル名
	 *	@return bool コンパイルに成功したら true
	 */
	bool CompileShader(ID3D11Device& _device, std::wstring& _fileName);
};