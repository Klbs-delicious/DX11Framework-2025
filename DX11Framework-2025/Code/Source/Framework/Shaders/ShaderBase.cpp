/** @file   ShaderBase.cpp
*   @date   2025/10/05
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Shaders/ShaderBase.h"
#include "Include/Framework/Shaders/ShaderCommon.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

using namespace ShaderCommon;

//-----------------------------------------------------------------------------
// ShaderBase class
//-----------------------------------------------------------------------------

ShaderBase::ShaderBase() :blob(nullptr) {}

ShaderBase::~ShaderBase()
{
	this->blob.Reset();
}

/**	@brief	シェーダーファイルからバイナリデータを読み込む
 *	@param ID3D11Device&    _device	    D3D11のデバイス
 *	@param const ShaderInfo _shaderInfo シェーダー情報
 *	@return ComPtr<ID3DBlob>            シェーダーバイナリ
 */
void ShaderBase::LoadShader(ID3D11Device& _device, const ShaderInfo _shaderInfo)
{
#ifdef _DEBUG
    this->CompileShader(_device, _shaderInfo);

#else
    // 指定のシェーダーファイルを読み込み
    std::wstring fullPath = L"Assets/Shaders/" + _fileName + L".cso";

    HRESULT hr = D3DReadFileToBlob(fullPath.c_str(), this->blob.GetAddressOf());

    if (FAILED(hr)) {
        OutputDebugString(L"[CSO Load Error] シェーダーバイナリの読み込みに失敗しました。\n");

        // 読み込みに失敗した場合はコンパイルする
        this->CompileShader(_device, _fileName);
        return;
    }
#endif
}

/** @brief シェーダーのコンパイル
 *  @param ID3D11Device& _device D3D11のデバイス
 *  @param ShaderInfo _shaderInfo シェーダー情報
 *  @return bool コンパイルに成功したら true
 */
bool ShaderBase::CompileShader(ID3D11Device& _device, const ShaderInfo _shaderInfo)
{
    DX::ComPtr<ID3DBlob> errorBlob;

    // 指定のシェーダーファイルをコンパイル
    std::wstring fullPath = L"Code/Shaders/" + _shaderInfo.filePath + L".hlsl";

    HRESULT hrVS = D3DCompileFromFile(
        fullPath.c_str(),
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        EntryPointName[static_cast<int>(_shaderInfo.shaderType)].c_str(), TargetName[static_cast<int>(_shaderInfo.shaderType)].c_str(), 0, 0,
        this->blob.GetAddressOf(), errorBlob.GetAddressOf());

    if(SUCCEEDED(hrVS))
    {
        return true;
	}

    if (errorBlob)
    {
        const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
        std::string errorString(errorMsg, errorBlob->GetBufferSize());
        std::wstring wErrorString(errorString.begin(), errorString.end());
        OutputDebugString(L"[Shader Compilation Error]\n");
        OutputDebugString(wErrorString.c_str());
    }
    else
    {
        OutputDebugString(L"シェーダーのコンパイルに失敗しましたが、errorBlob に情報がありません。\n");
    }
    return false;
}