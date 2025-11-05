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

TestRenderer::TestRenderer(GameObject* _owner, bool _active)
    : Component(_owner, _active)
{   
 //   // MaterialComponentが存在しなければ追加する
 //   this->materialComponent = this->Owner()->GetComponent<MaterialComponent>();
 //   if (!this->materialComponent) {
 //       this->materialComponent = this->Owner()->AddComponent<MaterialComponent>();
 //   }
 //   this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
 //   if(!this->meshComponent) {
 //       this->meshComponent = this->Owner()->AddComponent<MeshComponent>();
	//}
}

TestRenderer::~TestRenderer()
{
    Dispose();
}

void TestRenderer::Initialize()
{
 //   auto& d3d = SystemLocator::Get<D3D11System>();
 //   auto device = d3d.GetDevice();

 //   // コンポーネント取得
 //   auto* cameraObject = SystemLocator::Get<GameObjectManager>().GetFindObjectByName("Camera3D");
 //   if (!cameraObject)
 //   {
 //       OutputDebugStringA("[TestRenderer] Camera3D object not found.\n");
 //       return;
 //   }

 //   this->camera = cameraObject->GetComponent<Camera3D>();
 //   if (!this->camera)
 //   {
 //       OutputDebugStringA("[TestRenderer] Camera3D component missing.\n");
 //       return;
 //   }

 //   this->transform = this->Owner()->GetComponent<Transform>();

 //   // モデル読み込み
 //   ModelImporter_t importer;
 //   modelLoaded = importer.Load(
 //       "Assets/Models/Woman/woman.fbx",   // モデルパス
 //       "",                                // テクスチャフォルダ（空でOK）
 //       modelData
 //   );

 //   if (!modelLoaded)
 //   {
 //       OutputDebugStringA("[TestRenderer] Failed to load model.\n");
 //       return;
 //   }

	//// メッシュの作成
	//auto mesh = std::make_unique<Graphics::Mesh>();
 //   auto& shaders = this->Owner()->Services()->shaders;
 //   mesh->Create(device, d3d.GetContext(), shaders, modelData);

	//// テスト的にメッシュをセット（現状はmeshComponent内でメモリリークを起こす）
	//this->meshComponent->SetMesh(mesh.release());

 //   // シェーダー取得
 //   auto& materials = this->Owner()->Services()->materials;
 //   materialComponent->SetMaterial(materials->Get("ModelBasic"));

 //   // ライト定数バッファ作成（b3）
 //   D3D11_BUFFER_DESC desc = {};
 //   desc.ByteWidth = sizeof(XMFLOAT4) * 2;
 //   desc.Usage = D3D11_USAGE_DEFAULT;
 //   desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
 //   desc.CPUAccessFlags = 0;
 //   device->CreateBuffer(&desc, nullptr, lightBuffer.GetAddressOf());

 //   std::cout << "[TestRenderer] Initialized.\n";
}

void TestRenderer::Draw()
{
   // if (!modelLoaded || !meshComponent || !camera)
   //     return;

   // auto& d3d = SystemLocator::Get<D3D11System>();
   // auto& render = SystemLocator::Get<RenderSystem>();
   // auto ctx = d3d.GetContext();
   // auto dev = d3d.GetDevice();

   // Matrix world = transform ? transform->GetWorldMatrix() : Matrix::Identity;
   // Matrix view = camera->GetViewMatrix();
   // Matrix proj = camera->GetProjectionMatrix();

   // render.SetWorldMatrix(&world);
   // render.SetViewMatrix(&view);
   // render.SetProjectionMatrix(&proj);

   // // ブレンドステートを設定
   // render.SetBlendState(BlendStateType::BS_NONE);

   // // 簡易ライト設定
   // struct LightBuffer
   // {
   //     XMFLOAT3 lightDir;
   //     float pad1;
   //     XMFLOAT4 baseColor;
   // };

   // LightBuffer light = {};
   // light.lightDir = { 0.3f, -1.0f, 0.2f };
   // light.baseColor = { 0.8f, 0.7f, 0.6f, 1.0f };

   // ctx->UpdateSubresource(lightBuffer.Get(), 0, nullptr, &light, 0, 0);
   // ctx->PSSetConstantBuffers(3, 1, lightBuffer.GetAddressOf());

   // // バインド
   // auto mesh = this->meshComponent->GetMesh();
   // mesh->Bind(*ctx);
   // ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


   // for (auto& subset : mesh->GetSubsets())
   // {
   //     auto* material = mesh->GetMaterial(subset.materialIndex);
   //     if (material)
   //     {
   //         material->shaders->Bind(*ctx);
			//material->materialBuffer->BindVS(ctx, 3);
			//material->materialBuffer->BindPS(ctx, 1);
   //         if (material->albedoMap)
   //             material->albedoMap->Bind(ctx, 0);
   //         render.SetSampler(material->samplerType); // 既存のユーティリティを使用
   //     }
   //     ctx->DrawIndexed(subset.indexCount, subset.indexStart, subset.vertexBase);
   // }
}

void TestRenderer::Dispose()
{
    lightBuffer.Reset();

    modelData.vertices.clear();
    modelData.indices.clear();
    modelData.subsets.clear();
    modelData.materials.clear();
    modelData.diffuseTextures.clear();
    materialComponent = nullptr;
    meshComponent = nullptr;

    std::cout << "[TestRenderer] Disposed.\n";
}