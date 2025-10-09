#include "Framework/Entities/TestRenderer.h"
#include "Framework/Entities/GameObject.h"
#include "Framework/Core/SystemLocator.h"
#include "Framework/Core/ResourceHub.h"
#include "Framework/Core/D3D11System.h"
#include "Framework/Core/RenderSystem.h"
#include "Framework/Core/SpriteManager.h"
#include "Framework/Core/ShaderManager.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include	<iostream>

TestRenderer::TestRenderer(GameObject* owner, bool isActive)
    : Component(owner, isActive)
{
    // シェーダーの取得
    auto& shaderManager = ResourceHub::Get<ShaderBase>();
    this->shaders.push_back(shaderManager.Get("TestVS"));
    this->shaders.push_back(shaderManager.Get("TestPS"));

    // デフォルトの画像情報を取得する
    IResourceManager<Sprite>& spriteManager = ResourceHub::Get<Sprite>();
    this->sprite = spriteManager.Get("Default");

    this->camera = this->owner->GetComponent<Camera2D>();
	this->transform = this->owner->GetComponent<Transform>();
}

TestRenderer::~TestRenderer() {}

void TestRenderer::Initialize()
{
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto device = d3d11.GetDevice();

    Vertex vertices[] = {
        { {-0.5f, -0.5f, 0}, {1, 1, 1, 1}, {0.0f, 0.0f} }, // 左下 → 左上
        { { 0.5f, -0.5f, 0}, {1, 1, 1, 1}, {1.0f, 0.0f} }, // 右下 → 右上
        { {-0.5f,  0.5f, 0}, {1, 1, 1, 1}, {0.0f, 1.0f} }, // 左上 → 左下
        { { 0.5f,  0.5f, 0}, {1, 1, 1, 1}, {1.0f, 1.0f} }, // 右上 → 右下
    };
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &initData, vertexBuffer.GetAddressOf());

    uint16_t indices[] = {
    0, 1, 2,  // 第1三角形（左下 → 右下 → 左上）
    1, 3, 2   // 第2三角形（右下 → 右上 → 左上）
    };

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    device->CreateBuffer(&ibDesc, &ibData, indexBuffer.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, sizeof(float) * 7,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    // 一旦テスト的に頂点シェーダーを指定していれる
    device->CreateInputLayout(layout, ARRAYSIZE(layout), this->shaders[0]->GetBlob()->GetBufferPointer(), this->shaders[0]->GetBlob()->GetBufferSize(), inputLayout.GetAddressOf());
}

void TestRenderer::Draw()
{
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();

    // 変換行列を送る
    using namespace DirectX::SimpleMath;
	Matrix view = this->camera->GetViewMatrix();
	Matrix proj = this->camera->GetProjectionMatrix();
    Matrix world = this->transform->GetWorldMatrix();
    render.SetWorldMatrix(&world);
	render.SetViewMatrix(&view);
	render.SetProjectionMatrix(&proj);

    // バッファを送る
    auto ctx = d3d11.GetContext();
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    ctx->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    ctx->IASetInputLayout(inputLayout.Get());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // テクスチャを送る
    ID3D11ShaderResourceView* srv = this->sprite->texture.Get();
    if (!srv) {
		std::cerr << "テクスチャがセットされていません。" << std::endl;
		return;
    }
    ctx->PSSetShaderResources(0, 1, &srv);

    // シェーダーの設定
    for (auto& shader : this->shaders)
    {
        shader->Bind(*ctx);
    }

    // 描画
    ctx->DrawIndexed(6, 0, 0);
}

void TestRenderer::Dispose()
{
    // ComPtr に任せる
}

/** @brief Spriteの設定
 *  @param std::string _spriteName    Spriteの情報
 *  @return bool 設定出来たら true
 */
bool TestRenderer::SetSprite(const std::string& _spriteName)
{
    IResourceManager<Sprite>& spriteManager = ResourceHub::Get<Sprite>();
    this->sprite = spriteManager.Get(_spriteName);
    if (!this->sprite) {
        std::cerr << "その名前のSpriteが存在しません。" << std::endl;
        return false;
    }
    return true;
}