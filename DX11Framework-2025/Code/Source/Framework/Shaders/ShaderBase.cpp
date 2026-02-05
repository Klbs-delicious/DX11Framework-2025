/** @file   ShaderBase.cpp
 *  @date   2025/10/05
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Shaders/ShaderBase.h"
#include "Include/Framework/Shaders/ShaderCommon.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

using namespace ShaderCommon;

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	/** @brief HRESULT を人が読める文字列に変換する
	 *  @param _hr HRESULT
	 *  @return std::wstring メッセージ
	 */
	static std::wstring HrToMessageW(HRESULT _hr)
	{
		LPWSTR buffer = nullptr;

		const DWORD flags =
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS;

		const DWORD size = FormatMessageW(
			flags,
			nullptr,
			static_cast<DWORD>(_hr),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&buffer),
			0,
			nullptr);

		std::wstring msg;
		if (size != 0 && buffer)
		{
			msg.assign(buffer, buffer + size);
			LocalFree(buffer);
		}
		else
		{
			msg = L"(エラーメッセージ取得失敗)";
		}

		return msg;
	}

	/** @brief 実行時にユーザーへエラーを通知する
	 *  @param _title タイトル
	 *  @param _body  本文
	 */
	static void ShowErrorPopupW(const wchar_t* _title, const std::wstring& _body)
	{
		MessageBoxW(nullptr, _body.c_str(), _title, MB_OK | MB_ICONERROR);
	}
}

//-----------------------------------------------------------------------------
// ShaderBase class
//-----------------------------------------------------------------------------

ShaderBase::ShaderBase() : blob(nullptr) {}

ShaderBase::~ShaderBase()
{
	this->blob.Reset();
}

/**	@brief	シェーダーファイルからバイナリデータを読み込む
 *	@param	_device	    D3D11のデバイス
 *	@param	_shaderInfo シェーダー情報
 */
void ShaderBase::LoadShader(ID3D11Device& _device, const ShaderInfo _shaderInfo)
{
#ifdef _DEBUG
	// Debug は常に hlsl をコンパイルして最新を使い、.cso を書き出す
	this->CompileShader(_device, _shaderInfo, true);
#else
	// Release は .cso を読み込む（無い場合はフォールバックでコンパイルして .cso を生成する）
	const std::wstring csoPath = ShaderBase::MakeCsoPath(_shaderInfo);

	const HRESULT hr = D3DReadFileToBlob(csoPath.c_str(), this->blob.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstringstream ss;
		ss << L".cso の読み込みに失敗しました。\n\n"
			<< L"Path:\n" << csoPath << L"\n\n"
			<< L"HRESULT: 0x"
			<< std::hex << std::setw(8) << std::setfill(L'0') << hr << L"\n"
			<< L"Message:\n" << HrToMessageW(hr);

		ShowErrorPopupW(L"Shader Load Error", ss.str());
		OutputDebugString(L"[CSO Load Error] シェーダーバイナリの読み込みに失敗しました。\n");

		// フォールバックとして hlsl からコンパイルし、次回以降は .cso 読み込みで済むようにする
		this->CompileShader(_device, _shaderInfo, true);
		return;
	}
#endif
}

/** @brief シェーダーのコンパイル
 *  @param _device     D3D11のデバイス
 *  @param _shaderInfo シェーダー情報
 *  @param _saveCso    .cso を書き出す場合 true
 *  @return コンパイルに成功したら true
 */
bool ShaderBase::CompileShader(ID3D11Device& _device, const ShaderInfo _shaderInfo, bool _saveCso) 
{
	DX::ComPtr<ID3DBlob> errorBlob;

	const std::wstring hlslPath = ShaderBase::MakeHlslPath(_shaderInfo);

	const HRESULT hr = D3DCompileFromFile(
		hlslPath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		EntryPointName[static_cast<int>(_shaderInfo.shaderType)].c_str(),
		TargetName[static_cast<int>(_shaderInfo.shaderType)].c_str(),
		0,
		0,
		this->blob.GetAddressOf(),
		errorBlob.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		if (_saveCso)
		{
			// コンパイル成功時に .cso を書き出す（Debug 用キャッシュ、Release フォールバック時の自己修復）
			const std::wstring csoPath = ShaderBase::MakeCsoPath(_shaderInfo);

			// 親ディレクトリが無いと書き込みに失敗するため、事前に作成する
			ShaderBase::EnsureDirectoryForFile(csoPath);

			const HRESULT hrWrite = D3DWriteBlobToFile(this->blob.Get(), csoPath.c_str(), TRUE);
			if (FAILED(hrWrite))
			{
				std::wstringstream ss;
				ss << L".cso の書き出しに失敗しました。\n\n"
					<< L"Path:\n" << csoPath << L"\n\n"
					<< L"HRESULT: 0x"
					<< std::hex << std::setw(8) << std::setfill(L'0') << hrWrite << L"\n"
					<< L"Message:\n" << HrToMessageW(hrWrite);

				ShowErrorPopupW(L"Shader Save Error", ss.str());
				OutputDebugString(L"[CSO Save Error] シェーダーバイナリの書き出しに失敗しました。\n");
			}
		}

		return true;
	}

	if (errorBlob)
	{
		const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
		std::string errorString(errorMsg, errorBlob->GetBufferSize());
		std::wstring wErrorString(errorString.begin(), errorString.end());

		OutputDebugString(L"[Shader Compilation Error]\n");
		OutputDebugString(wErrorString.c_str());

#ifndef _DEBUG
		std::wstringstream ss;
		ss << L"シェーダーのコンパイルに失敗しました。\n\n"
			<< L"Path:\n" << hlslPath << L"\n\n"
			<< L"Message:\n" << wErrorString;

		ShowErrorPopupW(L"Shader Compile Error", ss.str());
#endif
	}
	else
	{
		OutputDebugString(L"シェーダーのコンパイルに失敗しましたが、errorBlob に情報がありません。\n");

#ifndef _DEBUG
		std::wstringstream ss;
		ss << L"シェーダーのコンパイルに失敗しました。\n\n"
			<< L"Path:\n" << hlslPath << L"\n\n"
			<< L"詳細情報が取得できませんでした。";

		ShowErrorPopupW(L"Shader Compile Error", ss.str());
#endif
	}

	return false;
}

/** @brief シェーダーの種類からディレクトリ名を取得
 *  @param _type シェーダーの種類
 *  @return ディレクトリ名
 */
std::wstring ShaderBase::ShaderTypeToDirectory(ShaderType _type)
{
	switch (_type)
	{
	case ShaderType::VertexShader:
		return L"VertexShader";
	case ShaderType::PixelShader:
		return L"PixelShader";
	case ShaderType::GeometryShader:
		return L"GeometryShader";
	default:
		return L"UnknownShader";
	}
}

/** @brief シェーダーファイルのパスを取得
 *  @param _shaderInfo シェーダー情報
 *  @return シェーダーファイルのパス
 */
std::wstring ShaderBase::MakeCsoPath(const ShaderInfo& _shaderInfo)
{
	return L"Assets/Shaders/" + _shaderInfo.filePath + L".cso";
}

/** @brief シェーダーファイルのパスを取得
 *  @param _shaderInfo シェーダー情報
 *  @return シェーダーファイルのパス
 */
std::wstring ShaderBase::MakeHlslPath(const ShaderInfo& _shaderInfo) const
{
	return L"Code/Shaders/" + _shaderInfo.filePath + L".hlsl";
}

/** @brief 指定されたファイルパスの親ディレクトリを保証する
 *  @param _filePath ファイルパス
 */
void ShaderBase::EnsureDirectoryForFile(const std::wstring& _filePath)
{
	std::filesystem::path p(_filePath);
	std::filesystem::create_directories(p.parent_path());
}