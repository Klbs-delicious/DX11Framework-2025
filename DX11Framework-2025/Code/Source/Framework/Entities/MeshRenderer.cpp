/** @file   MeshRenderer.cpp
 *  @brief  Assimpモデル描画テスト
 *  @date   2025/10/26
 */
#include "Include/Framework/Entities/Meshrenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include <iostream>
#include <vector>

using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshRenderer::MeshRenderer(GameObject* _owner, bool _active)
    : Component(_owner, _active)
{
    // 必要なコンポーネントが存在しなければ追加する
    this->materialComponent = this->Owner()->GetComponent<MaterialComponent>();
    if (!this->materialComponent) {
        this->materialComponent = this->Owner()->AddComponent<MaterialComponent>();
    }
    this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
    if (!this->meshComponent) {
        this->meshComponent = this->Owner()->AddComponent<MeshComponent>();
    }
}

MeshRenderer::~MeshRenderer()
{
    Dispose();
}

void MeshRenderer::Initialize()
{
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto device = d3d.GetDevice();

    // コンポーネント取得
    auto* cameraObject = SystemLocator::Get<GameObjectManager>().GetFindObjectByName("Camera3D");
    if (!cameraObject)
    {
		std::cout << "[MeshRenderer] Camera3D object not found.\n";
        return;
    }

    this->camera = cameraObject->GetComponent<Camera3D>();
    if (!this->camera)
    {
		std::cout << "[MeshRenderer] Camera3D component missing.\n";
        return;
    }
    this->transform = this->Owner()->GetComponent<Transform>();

    // シェーダー取得
    auto& materials = this->Owner()->Services()->materials;
    materialComponent->SetMaterial(materials->Get("ModelBasic"));

    // ライト定数バッファの作成
    this->light.lightDir = { 0.3f, -1.0f, 0.2f };
    this->light.baseColor = { 0.8f, 0.7f, 0.6f, 1.0f };

    this->lightBuffer = std::make_unique<DynamicConstantBuffer<LightBuffer>>();
	this->lightBuffer->Create(device);
}

void MeshRenderer::Draw()
{
    if (!this->meshComponent || !this->camera) { return; }

    auto& d3d = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto ctx = d3d.GetContext();
    auto dev = d3d.GetDevice();

	// 変換行列を送る
    Matrix world = transform ? transform->GetWorldMatrix() : Matrix::Identity;
    Matrix view = camera->GetViewMatrix();
    Matrix proj = camera->GetProjectionMatrix();

    render.SetWorldMatrix(&world);
    render.SetViewMatrix(&view);
    render.SetProjectionMatrix(&proj);

    // ブレンドステートを設定
    render.SetBlendState(BlendStateType::BS_NONE);

	// ライト用定数バッファを更新
	this->lightBuffer->Update(ctx, this->light);
	this->lightBuffer->BindPS(ctx, 3);

	// メッシュを描画する
    auto mesh = this->meshComponent->GetMesh();
    mesh->Bind(*ctx);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    for (auto& subset : mesh->GetSubsets())
    {
        auto* material = mesh->GetMaterial(subset.materialIndex);
        if (material)
        {
            material->shaders->Bind(*ctx);
            material->materialBuffer->BindVS(ctx, 3);
            material->materialBuffer->BindPS(ctx, 1);
            if (material->albedoMap) { material->albedoMap->Bind(ctx, 0); }

            // 既存のユーティリティを使用する
            render.SetSampler(material->samplerType); 
        }
        ctx->DrawIndexed(subset.indexCount, subset.indexStart,0);
    }
}

void MeshRenderer::Dispose()
{
	this->lightBuffer.reset();

    this->materialComponent = nullptr;
    this->meshComponent = nullptr;
}
