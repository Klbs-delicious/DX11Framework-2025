#include "Include/Framework/Entities/TestRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Shaders/ShaderManager.h"
#include <iostream>

TestRenderer::TestRenderer(GameObject* owner, bool active)
    : Component(owner, active)
{
}

TestRenderer::~TestRenderer() {}

void TestRenderer::Initialize()
{
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto device = d3d.GetDevice();

    this->transform = this->Owner()->GetComponent<Transform>();
    this->camera = this->Owner()->GetComponent<Camera3D>();

    // 頂点データ（簡単な平面）
    Vertex vertices[] = {
        { {-0.5f, 0.0f, -0.5f}, {0,1,0}, {0,0} },
        { { 0.5f, 0.0f, -0.5f}, {0,1,0}, {1,0} },
        { {-0.5f, 0.0f,  0.5f}, {0,1,0}, {0,1} },
        { { 0.5f, 0.0f,  0.5f}, {0,1,0}, {1,1} },
    };

    uint16_t indices[] = {
        0, 1, 2,
        2, 1, 3,
    };

    vertexBuffer = std::make_unique<VertexBuffer>();
    vertexBuffer->Create(device, vertices, sizeof(Vertex), _countof(vertices));

    indexBuffer = std::make_unique<IndexBuffer>();
    indexBuffer->Create(device, indices, sizeof(uint16_t), _countof(indices));

    // シェーダー取得（ShaderManagerなどから取得する想定）
    auto& shaderMgr = this->Owner()->Services()->shaders;
    vertexShader = shaderMgr->Get("DefaultVS");
    pixelShader = shaderMgr->Get("DefaultPS");

    std::cout << "[TestRenderer] Initialized." << std::endl;
}

void TestRenderer::Draw()
{
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto ctx = d3d.GetContext();

    if (!vertexBuffer || !indexBuffer) return;

    // 行列設定
    using namespace DirectX::SimpleMath;
    Matrix world = transform ? transform->GetWorldMatrix() : Matrix::Identity;
    Matrix view = camera ? camera->GetViewMatrix() : Matrix::Identity;
    Matrix proj = camera ? camera->GetProjectionMatrix() : Matrix::Identity;
    Matrix wvp = world * view * proj;

    render.SetWorldMatrix(&world);
    render.SetViewMatrix(&view);
    render.SetProjectionMatrix(&proj);

    // バインド
    vertexBuffer->Bind(ctx);
    indexBuffer->Bind(ctx);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // シェーダー適用
    if (vertexShader) vertexShader->Bind(*ctx);
    if (pixelShader) pixelShader->Bind(*ctx);

    // 描画
    ctx->DrawIndexed(indexBuffer->GetIndexCount(), 0, 0);
}

void TestRenderer::Dispose()
{
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto ctx = d3d.GetContext();

    vertexBuffer.reset();
    indexBuffer.reset();

    vertexShader = nullptr;
    pixelShader = nullptr;
}
