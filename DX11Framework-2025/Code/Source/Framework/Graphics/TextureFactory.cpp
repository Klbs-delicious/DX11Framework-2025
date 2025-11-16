/** @file   TextureFactory.cpp
 *  @date   2025/11/15
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/TextureFactory.h"

//-----------------------------------------------------------------------------
// TextureFactory class
//-----------------------------------------------------------------------------
std::unique_ptr<TextureResource> TextureFactory::CreateSolidColorTexture(
    ID3D11Device* _device,
    const DX::Color& _color,
    int _width,
    int _height)
{
    if (!_device) { return nullptr; }

    // ピクセル数
    const int pixelCount = _width * _height;

    // RGBA 4byte
    std::vector<unsigned char> pixels(pixelCount * 4);

    for (int i = 0; i < pixelCount; ++i)
    {
        pixels[i * 4 + 0] = static_cast<unsigned char>(_color.R() * 255);
        pixels[i * 4 + 1] = static_cast<unsigned char>(_color.G() * 255);
        pixels[i * 4 + 2] = static_cast<unsigned char>(_color.B() * 255);
        pixels[i * 4 + 3] = static_cast<unsigned char>(_color.A() * 255);
    }

    // テクスチャ作成
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = _width;
    desc.Height = _height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = _width * 4;

    DX::ComPtr<ID3D11Texture2D> tex = nullptr;
    HRESULT hr = _device->CreateTexture2D(&desc, &initData, tex.GetAddressOf());
    if (FAILED(hr)) { return nullptr; }

    // SRVを作成
    DX::ComPtr<ID3D11ShaderResourceView> srv = nullptr;
    hr = _device->CreateShaderResourceView(tex.Get(), nullptr, srv.GetAddressOf());
    if (FAILED(hr)) { return nullptr; }

    // TextureResource を作成して返す
    auto texture = std::make_unique<TextureResource>();
    texture->texture = srv;
    texture->width = _width;
    texture->height = _height;
    texture->bpp = 32;   // RGBA

    return texture;
}
