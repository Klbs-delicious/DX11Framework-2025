/** @file   SkinnedMeshRenderer.cpp
 *  @brief  スキニング済みメッシュを描画するコンポーネント
 *  @date   2026/01/15
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/SkinnedMeshRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Shaders/ShaderManager.h"
#include "Include/Framework/Graphics/VertexTypes.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

//-----------------------------------------------------------------------------
// SkinnedMeshRenderer class
//-----------------------------------------------------------------------------

SkinnedMeshRenderer::SkinnedMeshRenderer(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	transform(nullptr),
	camera(nullptr),
	meshComponent(nullptr),
	materialComponent(nullptr),
	animationComponent(nullptr),
	lightBuffer(nullptr)
{
}

SkinnedMeshRenderer::~SkinnedMeshRenderer()
{
	this->Dispose();
}

void SkinnedMeshRenderer::Initialize()
{
	//-------------------------------------------------------------
	// 必要なコンポーネントが存在しなければ追加する
	//-------------------------------------------------------------
	this->materialComponent = this->Owner()->GetComponent<MaterialComponent>();
	if (!this->materialComponent)
	{
		this->materialComponent = this->Owner()->AddComponent<MaterialComponent>();
	}

	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
	if (!this->meshComponent)
	{
		this->meshComponent = this->Owner()->AddComponent<MeshComponent>();
	}
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();

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

	//-------------------------------------------------------------
	// 必要なコンポーネントを取得
	//-------------------------------------------------------------

	auto& d3d = SystemLocator::Get<D3D11System>();
	auto device = d3d.GetDevice();

	// シェーダーをスキニング用に変更する
	auto& shaders = this->Owner()->Services()->shaders;
	Material* current = this->materialComponent->GetMaterial();
	if (!current)
	{
		std::cout << "[SkinnedMeshRenderer] Material not set.\n";
		return;
	}

	ShaderCommon::ShaderProgram* program = current->shaders;
	program = shaders->GetShaderProgram("SkinnedModel");
	if (!program)
	{
		std::cout << "[SkinnedMeshRenderer] SkinnedModel shader program not found.\n";
		return;
	}
	current->shaders = program;

	// ライト定数バッファを作成
	this->light.lightDir = { 0.4f, -1.0f, 0.3f };
	this->light.baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };

	this->lightBuffer = std::make_unique<DynamicConstantBuffer<LightBuffer>>();
	this->lightBuffer->Create(device);
}

void SkinnedMeshRenderer::Dispose()
{
	this->lightBuffer.reset();

	this->materialComponent = nullptr;
	this->meshComponent = nullptr;
	this->animationComponent = nullptr;
	this->camera = nullptr;
	this->transform = nullptr;
}

void SkinnedMeshRenderer::Draw()
{
	if (!this->meshComponent || !this->camera) { return; }

	// -------------------------------------------------------------
	// デバッグ情報の出力
	// -------------------------------------------------------------
	const DirectX::SimpleMath::Vector3 camPos = this->camera->Owner()->GetComponent<Transform>()
		? this->camera->Owner()->GetComponent<Transform>()->GetWorldPosition()
		: DirectX::SimpleMath::Vector3::Zero;

	const DirectX::SimpleMath::Vector3 objPos = this->transform
		? this->transform->GetWorldPosition()
		: DirectX::SimpleMath::Vector3::Zero;

	const float dist = (objPos - camPos).Length();

	//std::cout << "[SkinnedMeshRenderer][Debug] camPos=(" << camPos.x << "," << camPos.y << "," << camPos.z << ") "
	//	<< "objPos=(" << objPos.x << "," << objPos.y << "," << objPos.z << ") "
	//	<< "dist=" << dist
	//	<< std::endl;


	auto& d3d = SystemLocator::Get<D3D11System>();
	auto& render = SystemLocator::Get<RenderSystem>();
	auto ctx = d3d.GetContext();


	//-------------------------------------------------------------
	// 変換行列を送る
	//-------------------------------------------------------------
	Matrix world = this->transform ? this->transform->GetWorldMatrix() : Matrix::Identity;
	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();


	render.SetWorldMatrix(&world);
	render.SetViewMatrix(&view);
	render.SetProjectionMatrix(&proj);


	//-------------------------------------------------------------
	// ライト用定数バッファを更新
	//-------------------------------------------------------------
	if (this->lightBuffer)
	{
		this->lightBuffer->Update(ctx, this->light);
		this->lightBuffer->BindPS(ctx, 4);
	}


	//-------------------------------------------------------------
	// メッシュ情報をバインドする
	//-------------------------------------------------------------
	auto mesh = this->meshComponent->GetMesh();
	if (!mesh) { return; }


	// 2つ目のログ：IA の stride を取得して出す（Bind 後に見る）
	mesh->Bind(*ctx);


	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//-------------------------------------------------------------
	// マテリアルを適用する
	//-------------------------------------------------------------
	this->materialComponent->Apply(ctx, &render);

	//-------------------------------------------------------------
	// スキニング用の定数バッファを更新
	//-------------------------------------------------------------
	if (!this->animationComponent) { return; }
	this->animationComponent->BindBoneCBVS(ctx, 7);


	//-------------------------------------------------------------
	// 1つ目のログ：Draw 直前で subset の indexCount/indexStart を出す
	//-------------------------------------------------------------
	const auto& subsets = mesh->GetSubsets();

	for (size_t i = 0; i < subsets.size(); i++)
	{
		const auto& subset = subsets[i];

		// 描画（今は単一マテリアルを使い回す）
		ctx->DrawIndexed(subset.indexCount, subset.indexStart, 0);
	}
}