/** @file   TextureLoader.cpp
 *  @brief  TextureLoaderクラスの実装
 *  @date   2025/10/27
 */

#include "Include/Framework/Graphics/TextureLoader.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"

#include <fstream>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_TGA			// ← TGA ローダ自体を無効化（switchのフォーススルー警告を消す）
#define STBI_NO_GIF			// ← GIF ローダ自体を無効化（関数で確保している自動変数（スタック上のローカル領域）が大きい警告を消す）
#include "../External/stb/stb_image.h"

//-----------------------------------------------------------------------------
// ファイルからテクスチャを読み込む
//-----------------------------------------------------------------------------
std::unique_ptr<TextureResource> TextureLoader::FromFile(const std::string& _path) const
{
    std::ifstream ifs(_path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        OutputDebugStringA(("TextureLoader::FromFile - ファイルを開けませんでした: " + _path + "\n").c_str());
        return nullptr;
    }

    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!ifs.read(reinterpret_cast<char*>(buffer.data()), size)) {
        OutputDebugStringA(("TextureLoader::FromFile - ファイル読み込み失敗: " + _path + "\n").c_str());
        return nullptr;
    }

    return FromMemory(buffer.data(), static_cast<int>(buffer.size()));
}

//-----------------------------------------------------------------------------
// メモリ上の画像データからテクスチャを生成
//-----------------------------------------------------------------------------
std::unique_ptr<TextureResource> TextureLoader::FromMemory(const unsigned char* _data, int _len) const
{
    auto tex = std::make_unique<TextureResource>();

    int channels = 0;
    unsigned char* pixels = stbi_load_from_memory(
        _data, _len, &tex->width, &tex->height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        OutputDebugStringA("TextureLoader::FromMemory - stb_image 読み込み失敗\n");
        return nullptr;
    }

    DX::ComPtr<ID3D11Texture2D> pTexture;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = tex->width;
    desc.Height = tex->height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sub{};
    sub.pSysMem = pixels;
    sub.SysMemPitch = tex->width * 4;

    auto& d3d = SystemLocator::Get<D3D11System>();
    HRESULT hr = d3d.GetDevice()->CreateTexture2D(&desc, &sub, pTexture.GetAddressOf());

    if (FAILED(hr)) {
        stbi_image_free(pixels);
        OutputDebugStringA("TextureLoader::FromMemory - CreateTexture2D 失敗\n");
        return nullptr;
    }

    hr = d3d.GetDevice()->CreateShaderResourceView(pTexture.Get(), nullptr, tex->texture.GetAddressOf());
    stbi_image_free(pixels);

    if (FAILED(hr)) {
        OutputDebugStringA("TextureLoader::FromMemory - CreateSRV 失敗\n");
        return nullptr;
    }

    return std::move(tex);
}

std::unique_ptr<TextureResource> TextureLoader::FromRawRGBA(
    const unsigned char* data,
    unsigned int width,
    unsigned int height)
{
    if (!data || width == 0 || height == 0)
        return nullptr;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = width * 4; // 1ピクセル4バイト(RGBA)

    auto device = SystemLocator::Get<D3D11System>().GetDevice();
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&texDesc, &initData, &texture);
    if (FAILED(hr))
        return nullptr;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &srv);
    if (FAILED(hr))
        return nullptr;

    auto result = std::make_unique<TextureResource>();
    result->texture = srv;
    result->width = width;
    result->height = height;
    return result;
}
