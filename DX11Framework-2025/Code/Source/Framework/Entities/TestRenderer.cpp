#include "Include/Framework/Entities/TestRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Shaders/ShaderManager.h"

#pragma comment(lib, "d3dcompiler.lib")
#include	<iostream>

TestRenderer::TestRenderer(GameObject* owner, bool isActive):
    Component(owner, isActive),
    shaders()
{
	// SpriteComponentが存在しなければ追加する
    this->spriteComponent = this->Owner()->GetComponent<SpriteComponent>();
    if (!this->spriteComponent) 
    {
        this->spriteComponent = this->Owner()->AddComponent<SpriteComponent>();
	}
}

TestRenderer::~TestRenderer() {}

void TestRenderer::Initialize()
{
    // シェーダーの取得
    auto shaderManager = this->Owner()->Services()->shaders;
    this->program = shaderManager->DefaultProgram();

	// スプライトコンポーネントの取得
    this->spriteComponent = this->Owner()->GetComponent<SpriteComponent>();

    this->camera = this->Owner()->GetComponent<Camera2D>();
    this->transform = this->Owner()->GetComponent<Transform>();

    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto device = d3d11.GetDevice();

    // 頂点バッファの作成
    Vertex vertices[] = {
        { {-0.5f, -0.5f, 0}, {1, 1, 1, 1}, {0.0f, 0.0f} }, // 左下 → 左上
        { { 0.5f, -0.5f, 0}, {1, 1, 1, 1}, {1.0f, 0.0f} }, // 右下 → 右上
        { {-0.5f,  0.5f, 0}, {1, 1, 1, 1}, {0.0f, 1.0f} }, // 左上 → 左下
        { { 0.5f,  0.5f, 0}, {1, 1, 1, 1}, {1.0f, 1.0f} }, // 右上 → 右下
    };
    this->vertexBuffer = std::make_unique<VertexBuffer>();
    this->vertexBuffer->Create(device, vertices, sizeof(Vertex), _countof(vertices));

    // インデックスバッファの作成
    uint16_t indices[] = {
    0, 1, 2,  // 第1三角形（左下 → 右下 → 左上）
    1, 3, 2   // 第2三角形（右下 → 右上 → 左上）
    };
    this->indexBuffer = std::make_unique<IndexBuffer>();
    this->indexBuffer->Create(device, indices, sizeof(uint16_t), _countof(indices));
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
    this->vertexBuffer->Bind(ctx);
    this->indexBuffer->Bind(ctx);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // テクスチャを送る
    const Sprite* sprite = this->spriteComponent->GetSprite();
    ID3D11ShaderResourceView* srv = sprite ? sprite->texture.Get() : nullptr;
    if (!srv) {
		std::cerr << "テクスチャがセットされていません。" << std::endl;
		return;
    }
    ctx->PSSetShaderResources(0, 1, &srv);

    // シェーダーの設定
    this->program->Bind(*ctx);

    // 描画
    ctx->DrawIndexed(this->indexBuffer->GetIndexCount(), 0, 0);
}

void TestRenderer::Dispose() {
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto ctx = d3d11.GetContext();
    if (ctx) {
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        ctx->PSSetShaderResources(0, 1, nullSRV);
    }

    this->indexBuffer.reset();
    this->vertexBuffer.reset();

    shaders.clear();
}