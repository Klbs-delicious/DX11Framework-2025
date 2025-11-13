/** @file   CharacterController.cpp
 *  @brief  キャラクターの操作を行う
 *  @date   2025/11/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/CharacterController.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"

#include<iostream>

//-----------------------------------------------------------------------------
// CharacterController class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
 *  @param bool _active コンポーネントの有効/無効
 */
CharacterController::CharacterController(GameObject* _owner, bool _active)
	: Component(_owner, _active), 
	inputSystem(SystemLocator::Get<InputSystem>()), 
	cameraTransform(nullptr),
	moveSpeed(10.0f),
	turnSpeed(15.0f)
{}

/// @brief 初期化処理
void CharacterController::Initialize()
{
	// カメラオブジェクトのTransformを取得する
	this->cameraTransform = SystemLocator::Get<GameObjectManager>().GetFindObjectByName("Camera3D")->GetComponent<Transform>();	
	if(!this->cameraTransform)
	{
		std::cout << "[CharacterController] Camera3D component missing.\n";
		return;
	}

	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("MoveForward", static_cast<int>(DirectInputDevice::KeyboardKey::W));
	this->inputSystem.RegisterKeyBinding("MoveBackward", static_cast<int>(DirectInputDevice::KeyboardKey::S));
	this->inputSystem.RegisterKeyBinding("MoveLeft", static_cast<int>(DirectInputDevice::KeyboardKey::A));
	this->inputSystem.RegisterKeyBinding("MoveRight", static_cast<int>(DirectInputDevice::KeyboardKey::D));
}

/** @brief 更新処理
 *  @param float _deltaTime 前フレームからの経過時間（秒）
 */
void CharacterController::Update(float _deltaTime)
{
	// 
	auto transform = this->Owner()->transform;
	if (!transform) { return; }
	if (!this->cameraTransform) { return; }

	// ------------------------------------------------------
	// 入力処理
	// ------------------------------------------------------
	float inputX = 0.0f;	// 左(-1) ～ 右(+1)
	float inputZ = 0.0f;	// 後ろ(-1) ～ 前(+1)

	if (this->inputSystem.IsActionPressed("MoveForward")) { inputZ += 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveBackward")) { inputZ -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveLeft")) { inputX -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveRight")) { inputX += 1.0f; }

	// 入力なし
	if (inputX == 0.0f && inputZ == 0.0f) { return; }

	// ------------------------------------------------------
	// カメラの前方向・右方向（水平成分のみ）
	// ------------------------------------------------------
	DX::Vector3 camForward = this->cameraTransform->Forward();
	camForward.y = 0.0f;
	camForward.Normalize();

	DX::Vector3 camRight = this->cameraTransform->Right();
	camRight.y = 0.0f;
	camRight.Normalize();

	// ------------------------------------------------------
	// カメラ基準入力 → ワールド方向ベクトルへ変換
	// ------------------------------------------------------
	DX::Vector3 moveDir = camForward * inputZ + camRight * inputX;

	// 斜め移動のときも速度一定にするため正規化
	if (moveDir.LengthSquared() > 0.0f)
	{
		moveDir.Normalize();
	}
	else { return; }

	// ------------------------------------------------------
	// 向きの更新（キャラの正面を moveDir に向ける）
	// ------------------------------------------------------
	DX::Quaternion currentRot = transform->GetLocalRotation();
	DX::Quaternion targetRot = DX::Quaternion::CreateFromRotationMatrix(
		DX::CreateWorldLH(DX::Vector3::Zero, moveDir, DX::Vector3::UnitY)
	);

	// 球形補間で回転を滑らかに行う
	DX::Quaternion newRot = DX::Quaternion::Slerp(currentRot, targetRot, turnSpeed * _deltaTime);
	transform->SetLocalRotation(newRot);

	// ------------------------------------------------------
	// 実際の移動処理
	// ------------------------------------------------------
	DX::Vector3 pos = transform->GetLocalPosition();
	pos += moveDir * this->moveSpeed * _deltaTime;
	transform->SetLocalPosition(pos);
}