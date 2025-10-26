/** @file   TestRenderer.cpp
 *  @brief  Assimpモデル描画テスト
 *  @date   2025/10/26
 */
#include "Include/Framework/Entities/TestRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include <iostream>
#include <vector>

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct ModelVertexGPU
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
};

TestRenderer::TestRenderer(GameObject* _owner, bool _active)
    : Component(_owner, _active)
{   
    // MaterialComponentが存在しなければ追加する
    this->materialComponent = this->Owner()->GetComponent<MaterialComponent>();
    if (!this->materialComponent) {
        this->materialComponent = this->Owner()->AddComponent<MaterialComponent>();
    }
}

TestRenderer::~TestRenderer()
{
    Dispose();
}

void TestRenderer::Initialize()
{
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto device = d3d.GetDevice();

    // コンポーネント取得
    auto* cameraObject = SystemLocator::Get<GameObjectManager>().GetFindObjectByName("Camera3D");
    if (!cameraObject)
    {
        OutputDebugStringA("[TestRenderer] Camera3D object not found.\n");
        return;
    }

    this->camera = cameraObject->GetComponent<Camera3D>();
    if (!this->camera)
    {
        OutputDebugStringA("[TestRenderer] Camera3D component missing.\n");
        return;
    }

    this->transform = this->Owner()->GetComponent<Transform>();

    // モデル読み込み
    ModelImporter_t importer;
    modelLoaded = importer.Load(
        "Assets/Models/Woman/woman.fbx",   // モデルパス
        "",                                // テクスチャフォルダ（空でOK）
        modelData
    );

    if (!modelLoaded)
    {
        OutputDebugStringA("[TestRenderer] Failed to load model.\n");
        return;
    }

    // バッファ生成
    if (!modelData.vertices.empty())
    {
        size_t totalVertexCount = 0;
        for (const auto& meshVertices : modelData.vertices)
        {
            totalVertexCount += meshVertices.size();
        }

        size_t totalIndexCount = 0;
        for (const auto& meshIndices : modelData.indices)
        {
            totalIndexCount += meshIndices.size();
        }

        std::vector<ModelVertexGPU> gpuVertices;
        gpuVertices.reserve(totalVertexCount);

        std::vector<unsigned int> gpuIndices;
        gpuIndices.reserve(totalIndexCount);

        size_t vertexOffset = 0;
        for (size_t meshIndex = 0; meshIndex < modelData.vertices.size(); ++meshIndex)
        {
            const auto& meshVertices = modelData.vertices[meshIndex];
            for (const auto& vertex : meshVertices)
            {
                ModelVertexGPU gpuVertex{};
                gpuVertex.position = { vertex.pos.x, vertex.pos.y, vertex.pos.z };
                gpuVertex.normal = { vertex.normal.x, vertex.normal.y, vertex.normal.z };
                gpuVertices.emplace_back(gpuVertex);
            }

            if (meshIndex < modelData.indices.size())
            {
                const auto& meshIndices = modelData.indices[meshIndex];
                for (auto index : meshIndices)
                {
                    gpuIndices.emplace_back(static_cast<unsigned int>(index + vertexOffset));
                }
            }

            vertexOffset += meshVertices.size();
        }

        if (!gpuVertices.empty())
        {
            vertexBuffer = std::make_unique<VertexBuffer>();
            vertexBuffer->Create(device, gpuVertices.data(), sizeof(ModelVertexGPU), static_cast<UINT>(gpuVertices.size()));
        }

        if (!gpuIndices.empty())
        {
            indexBuffer = std::make_unique<IndexBuffer>();
            indexBuffer->Create(device, gpuIndices.data(), sizeof(unsigned int), static_cast<UINT>(gpuIndices.size()));
        }

        if (vertexBuffer && indexBuffer)
        {
            std::cout << "[TestRenderer] Buffers created.\n";
        }
    }

    // シェーダー取得
    auto& materials = this->Owner()->Services()->materials;
    materialComponent->SetMaterial(materials->Get("ModelTest"));

    // ライト定数バッファ作成（b3）
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(XMFLOAT4) * 2;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    device->CreateBuffer(&desc, nullptr, lightBuffer.GetAddressOf());

    std::cout << "[TestRenderer] Initialized.\n";
}

void TestRenderer::Draw()
{
    if (!modelLoaded || !vertexBuffer || !indexBuffer || !camera)
        return;

    auto& d3d = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto ctx = d3d.GetContext();
    auto dev = d3d.GetDevice();

    Matrix world = transform ? transform->GetWorldMatrix() : Matrix::Identity;
    Matrix view = camera->GetViewMatrix();
    Matrix proj = camera->GetProjectionMatrix();

    render.SetWorldMatrix(&world);
    render.SetViewMatrix(&view);
    render.SetProjectionMatrix(&proj);

    // 簡易ライト設定
    struct LightBuffer
    {
        XMFLOAT3 lightDir;
        float pad1;
        XMFLOAT4 baseColor;
    };

    LightBuffer light = {};
    light.lightDir = { 0.3f, -1.0f, 0.2f };
    light.baseColor = { 0.8f, 0.7f, 0.6f, 1.0f };

    ctx->UpdateSubresource(lightBuffer.Get(), 0, nullptr, &light, 0, 0);
    ctx->PSSetConstantBuffers(3, 1, lightBuffer.GetAddressOf());

    // バインド
    vertexBuffer->Bind(ctx);
    indexBuffer->Bind(ctx);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    materialComponent->Apply(ctx, &render);

    ctx->DrawIndexed(indexBuffer->GetIndexCount(), 0, 0);
}

void TestRenderer::Dispose()
{
    vertexBuffer.reset();
    indexBuffer.reset();
    lightBuffer.Reset();

    modelData.vertices.clear();
    modelData.indices.clear();
    modelData.subsets.clear();
    modelData.materials.clear();
    modelData.diffuseTextures.clear();
    materialComponent = nullptr;

    std::cout << "[TestRenderer] Disposed.\n";
}