/** @file   SpriteManager.cpp
*   @date   2025/09/25
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Core/D3D11System.h"
#include"Include/Framework/Utils/CommonTypes.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../External/stb/stb_image.h"

#include	<filesystem>
#include	<fstream>
#include	<iostream>

//-----------------------------------------------------------------------------
// SpriteManager class
//-----------------------------------------------------------------------------

/// @brief 画像の読み込みパスを登録
void SpriteManager::TexturepathRegister()
{
	// 画像のパスを登録していく
	this->spritePathMap["Default"] = u8"Assets/Textures/Default.png";
	this->spritePathMap["Eidan"] = u8"Assets/Textures/Eidan.png";
}

//// @brief コンストラクタ
SpriteManager::SpriteManager() :spriteMap()
{
	// 画像の読み込みパスを登録
	this->TexturepathRegister();
}
/// @brief デストラクタ
SpriteManager::~SpriteManager()
{
	this->spriteMap.clear();		// Spriteデストラクタ内部で明示的にShaderResourceViewを解放している
	this->spritePathMap.clear();
}

/** @brief  リソースを登録する
 *	@param  const std::string& _key	リソースのキー
 *	@return bool	登録に成功したら true
 */
bool SpriteManager::Register(const std::string& _key)
{
	// 存在する場合は登録成功にする
	if (this->spriteMap.contains(_key)) { return true; }

	// 画像を読み込む
	const std::u8string& path = this->spritePathMap.at(_key);
	std::unique_ptr<Sprite> resource = this->LoadTexture(path);

	if (!resource) {
		std::cerr << "Failed to LoadTexture: " << std::string(path.begin(), path.end()) << std::endl;
		return false;
	}

	this->spriteMap[_key] = std::move(resource);
	return true;
}

/**	@brief リソースの登録を解除する
 *	@param  const std::string& _key	リソースのキー
 */
void SpriteManager::Unregister(const std::string& _key)
{
	// 存在しない場合は処理を行わない
	if (!this->spriteMap.contains(_key)) { return; }

	auto it = this->spriteMap.find(_key);
	if (it != this->spriteMap.end())
	{
		it->second.reset();			// リソースの中身（unique_ptr）を解放        
		this->spriteMap.erase(it);	// キーと空のポインタをマップから削除
	}
}

/**	@brief	キーに対応するリソースを取得する
 *	@param	const std::string& _key	リソースのキー
 *	@return	T*	リソースのポインタ、見つからなかった場合は nullptr
 */
Sprite* SpriteManager::Get(const std::string& _key)
{
	// すでに登録済みならそのまま返す
	auto it = this->spriteMap.find(_key);
	if (it != this->spriteMap.end()) 
	{
		return it->second.get();
	}
	// パスが存在するか確認
	auto pathIt = this->spritePathMap.find(_key);
	if (pathIt == this->spritePathMap.end()) 
	{
		std::cerr << "Sprite path not found for key: " << _key << std::endl;
		return nullptr;
	}

	// 登録処理
	if (!this->Register(_key)) {
		std::cerr << "Failed to register sprite for key: " << _key << std::endl;
		return nullptr;
	}

	return this->spriteMap.at(_key).get();
}

/**	@brief	画像の読み込み
 *	@param	const std::u8string& _path	画像のファイルパス
 *	@return	std::unique_ptr<Sprite>	画像データ（失敗した場合は nullptr）
 */
std::unique_ptr<Sprite> SpriteManager::LoadTexture(const std::u8string& _path)
{
	std::filesystem::path filepath = _path;

	std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
	if (!ifs) {
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return nullptr;
	}

	// Spriteの生成
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

	std::streamsize size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	// ファイルの読み込み
	std::vector<unsigned char> buffer(size);
	if (!ifs.read(reinterpret_cast<char*>(buffer.data()), size)) {
		std::cerr << "Failed to read file: " << filepath << std::endl;
		return nullptr;
	}

	// 画像をデコード（ピクセル配列に変換）
	unsigned char* pixels = stbi_load_from_memory(
		buffer.data(), static_cast<int>(buffer.size()),
		&sprite->width, &sprite->height, &sprite->bpp, STBI_rgb_alpha);

	if (!pixels) {
		std::cerr << "Failed to decode image: " << filepath << std::endl;
		return nullptr;
	}

	// テクスチャの作成
	DX::ComPtr<ID3D11Texture2D> pTexture;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = sprite->width;
	desc.Height = sprite->height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = sprite->width * 4;

	auto& d3d11 = SystemLocator::Get<D3D11System>();
	HRESULT hr = d3d11.GetDevice()->CreateTexture2D(&desc, &subResource, pTexture.GetAddressOf());
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return nullptr;
	}

	// ShaderResourceViewの作成
	hr = d3d11.GetDevice()->CreateShaderResourceView(pTexture.Get(), nullptr, sprite->texture.GetAddressOf());
	stbi_image_free(pixels);
	if (FAILED(hr)) {
		std::cerr << "Failed to CreateShaderResourceView: " << filepath << std::endl;
		return nullptr;
	}

	return std::move(sprite);
}

/**	@brief 画像データをメモリから読み込む
*	@param	const unsigned char*	_data	画像のバイナリデータ（メモリ上にある）
*	@param	int						_len	そのデータのサイズ（バイト数）
 */
std::unique_ptr<Sprite> SpriteManager::LoadFromMemory(const unsigned char* _data, int _len)
{
	bool sts = true;
	unsigned char* pixels;

	// Spriteの生成
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

	// 画像をデコード（ピクセル配列に変換）
	pixels = stbi_load_from_memory(_data,
		_len,
		&sprite->width,
		&sprite->height,
		&sprite->bpp,
		STBI_rgb_alpha);

	// テクスチャ2Dリソース生成
	ComPtr<ID3D11Texture2D> pTexture;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Width = sprite->width;
	desc.Height = sprite->height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;			// RGBA
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	// ピクセルデータを渡す
	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = desc.Width * 4;			// RGBA = 4 bytes per pixel
	subResource.SysMemSlicePitch = 0;

	ID3D11Device* device = SystemLocator::Get<D3D11System>().GetDevice();

	// テクスチャの作成
	HRESULT hr = device->CreateTexture2D(&desc, &subResource, pTexture.GetAddressOf());
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return nullptr;
	}

	// ShaderResourceViewの生成
	hr = device->CreateShaderResourceView(pTexture.Get(), nullptr, sprite->texture.GetAddressOf());
	if (FAILED(hr)) {
		stbi_image_free(pixels);
		return nullptr;
	}

	// ピクセルイメージを解放
	stbi_image_free(pixels);

	return std::move(sprite);
}