/** @file   SpriteRenderer.cpp
 *  @date   2025/10/13
 */
  
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/SpriteRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/RenderSystem.h"

//-----------------------------------------------------------------------------
// SpriteRenderer class
//-----------------------------------------------------------------------------

SpriteRenderer::SpriteRenderer(GameObject* owner, bool isActive) :
    Component(owner, isActive),
    camera(nullptr),
    transform(nullptr),
    spriteComponent(nullptr),
    materialComponent(nullptr),
    vertexBuffer(nullptr),
    indexBuffer(nullptr)
{
    // SpriteComponentが存在しなければ追加する
    this->spriteComponent = this->Owner()->GetComponent<SpriteComponent>();
    if (!this->spriteComponent){
        this->spriteComponent = this->Owner()->AddComponent<SpriteComponent>();
    }
    // MaterialComponentが存在しなければ追加する
    this->materialComponent = this->Owner()->GetComponent<MaterialComponent>();
    if (!this->materialComponent) {
        this->materialComponent = this->Owner()->AddComponent<MaterialComponent>();
    }
}

SpriteRenderer::~SpriteRenderer() {}

void SpriteRenderer::Initialize()
{
    // コンポーネントの取得
    this->camera = SystemLocator::Get<GameObjectManager>().GetFindObjectByName("Camera2D")->GetComponent<Camera2D>();
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

void SpriteRenderer::Draw()
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

    // SpriteComponentの画像をMaterialComponentにセットする
    this->materialComponent->SetTexture(const_cast<TextureResource*>(this->spriteComponent->GetSprite()));

    // マテリアルの適用
    this->materialComponent->Apply(ctx, &render);

    // 描画
    ctx->DrawIndexed(this->indexBuffer->GetIndexCount(), 0, 0);
}

void SpriteRenderer::Dispose()
{
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto ctx = d3d11.GetContext();
    if (ctx) {
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        ctx->PSSetShaderResources(0, 1, nullSRV);
    }

    this->indexBuffer.reset();
    this->vertexBuffer.reset();
}