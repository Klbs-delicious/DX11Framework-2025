/** @file   FogComponent.cpp
 *  @brief  フォグ演出を制御するコンポーネント
 *  @date   2025/12/19
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/FogComponent.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/ResourceHub.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/GameObjectManager.h"
#include "Include/Framework/Entities/MaterialComponent.h"

//-----------------------------------------------------------------------------
// FogComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
FogComponent::FogComponent(GameObject* _owner, bool _active)
	: Component(_owner, _active),
	fogBuffer(nullptr),
	normalBuffer(nullptr),
	camera(nullptr)
{}

/// @brief 初期化処理
void FogComponent::Initialize()
{
	auto& d3d = SystemLocator::Get<D3D11System>();
	this->fogBuffer = std::make_unique<DynamicConstantBuffer<FogBuffer>>();
	this->fogBuffer->Create(d3d.GetDevice());

	this->normalBuffer = std::make_unique<DynamicConstantBuffer<NormalMatrixBuffer>>();
	this->normalBuffer->Create(d3d.GetDevice());

	// カメラ位置取得（Camera3D がシーンに存在する前提）
	auto& gom = SystemLocator::Get<GameObjectManager>();
	GameObject* camObj = gom.GetFindObjectByName("Camera3D");
	if (!camObj) { return; }
	this->camera = camObj->GetComponent<Camera3D>();
	if (!camera) { return; }

	// 設定されているマテリアルをフォグ用に差し替える
	auto materialComponent = this->Owner()->GetComponent<MaterialComponent>();
	if (materialComponent)
	{
		materialComponent->SetMaterial(ResourceHub::Get<MaterialManager>().Get("Fog"));
	}
}

/// @brief 終了処理
void FogComponent::Dispose()
{
	this->fogBuffer.reset();
	this->normalBuffer.reset();
}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間
 */
void FogComponent::Update(float _deltaTime)
{
	if (!this->fogBuffer || !this->normalBuffer) { return; }
	if (!camera) { return; }

	// フォグ用定数バッファのデータを設定する
	FogBuffer f{};
	f.cameraPos = camera->Owner()->transform->GetWorldPosition();
	f.fogStart  = 10.0f;   
	f.fogEnd    = 80.0f; 
	f.fogColor  = DX::Vector3(0.01f, 0.02f, 0.04f); // 暗青系
	f.padding   = 0.0f;

	// 法線行列（逆転置 3x3）を計算（行ベクトル3本）
	DX::Matrix4x4 world = this->Owner()->transform->GetWorldMatrix();
	DX::Vector3 r0(world._11, world._12, world._13);
	DX::Vector3 r1(world._21, world._22, world._23);
	DX::Vector3 r2(world._31, world._32, world._33);

	float a00 = r0.x, a01 = r0.y, a02 = r0.z;
	float a10 = r1.x, a11 = r1.y, a12 = r1.z;
	float a20 = r2.x, a21 = r2.y, a22 = r2.z;

	float c00 =  (a11 * a22 - a12 * a21);
	float c01 = -(a10 * a22 - a12 * a20);
	float c02 =  (a10 * a21 - a11 * a20);
	float c10 = -(a01 * a22 - a02 * a21);
	float c11 =  (a00 * a22 - a02 * a20);
	float c12 = -(a00 * a21 - a01 * a20);
	float c20 =  (a01 * a12 - a02 * a11);
	float c21 = -(a00 * a12 - a02 * a10);
	float c22 =  (a00 * a11 - a01 * a10);

	float det = a00 * c00 + a01 * c01 + a02 * c02;
	float invDet = (det != 0.0f) ? 1.0f / det : 0.0f;

	DX::Vector3 irow0(invDet * c00, invDet * c10, invDet * c20);
	DX::Vector3 irow1(invDet * c01, invDet * c11, invDet * c21);
	DX::Vector3 irow2(invDet * c02, invDet * c12, invDet * c22);

	DX::Vector3 nrow0(irow0.x, irow1.x, irow2.x);
	DX::Vector3 nrow1(irow0.y, irow1.y, irow2.y);
	DX::Vector3 nrow2(irow0.z, irow1.z, irow2.z);

	NormalMatrixBuffer n{};
	n.row0 = nrow0;
	n.row1 = nrow1;
	n.row2 = nrow2;
	n.pad  = 0.0f;

	// 定数バッファを更新してバインド
	auto& d3d = SystemLocator::Get<D3D11System>();
	auto* ctx = d3d.GetContext();
	this->fogBuffer->Update(ctx, f);
	this->fogBuffer->BindPS(ctx, 5);
	this->normalBuffer->Update(ctx, n);
	this->normalBuffer->BindVS(ctx, 6);
}
