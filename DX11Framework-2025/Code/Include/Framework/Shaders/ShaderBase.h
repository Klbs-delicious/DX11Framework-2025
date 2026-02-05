/**@file   ShaderBase.h
 * @date   2025/10/05
 */
#pragma once
#include"Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Shaders/ShaderCommon.h"

#include<string>
#include<array>
#include <filesystem>

#include <d3d11.h>
#pragma comment(lib,"d3d11.lib")

using namespace ShaderCommon;

/**	@class	ShaderBase
 *	@brief	シェーダーの基底クラス
 */
class ShaderBase
{
public:
	ShaderBase();
	virtual ~ShaderBase();

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
	 *  @param ShaderInfo _shaderInfo シェーダー情報
	 *	@return bool シェーダーの生成に成功したら true
	 */
	virtual	bool CreateShader(ID3D11Device& _device, const ShaderInfo _shaderInfo) = 0;

	/**	@brief	バイナリデータの取得
	 *	@return ID3DBlob* シェーダーバイナリ
	 */
	ID3DBlob* GetBlob()const { return this->blob.Get(); }
protected:
	/**	@brief	シェーダーファイルからバイナリデータを読み込む
	 *	@param ID3D11Device&    _device	    D3D11のデバイス
	 *	@param const ShaderInfo _shaderInfo シェーダー情報
	 *	@return ComPtr<ID3DBlob>            シェーダーバイナリ
	 */
	void LoadShader(ID3D11Device& _device, const ShaderInfo _shaderInfo);

	mutable DX::ComPtr<ID3DBlob> blob;	///< シェーダーバイナリ

private:
	/** @brief シェーダーのコンパイル
	 *  @param _device     D3D11のデバイス
	 *  @param _shaderInfo シェーダー情報
	 *  @param _saveCso    .cso を書き出す場合 true
	 *  @return コンパイルに成功したら true
	 */
	bool CompileShader(ID3D11Device& _device, const ShaderInfo _shaderInfo, bool _saveCso);

	/** @brief シェーダーの種類からディレクトリ名を取得
	 *  @param ShaderType _type シェーダーの種類
	 *  @return std::wstring ディレクトリ名
	 */
	static std::wstring ShaderTypeToDirectory(ShaderType _type);

	/**	@brief	シェーダーファイルのパスを取得
	 *	@param const ShaderInfo _shaderInfo シェーダー情報
	 *	@return std::wstring シェーダーファイルのパス
	 */
	static std::wstring MakeCsoPath(const ShaderInfo& _shaderInfo);

	/**	@brief	シェーダーファイルのパスを取得
	 *	@param const ShaderInfo _shaderInfo シェーダー情報
	 *	@return std::wstring シェーダーファイルのパス
	 */
	std::wstring MakeHlslPath(const ShaderInfo& _shaderInfo)const ;

	/**	@brief	シェーダーファイルのパスを取得
	 *	@param const ShaderInfo _shaderInfo シェーダー情報
	 *	@return std::wstring シェーダーファイルのパス
	 */
	static void EnsureDirectoryForFile(const std::wstring& _filePath);
};