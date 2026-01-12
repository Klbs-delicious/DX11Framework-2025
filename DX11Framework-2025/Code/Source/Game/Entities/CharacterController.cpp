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
	rigidbody(nullptr),
	moveSpeed(10.0f),
	turnSpeed(15.0f)
{}

/// @brief 初期化処理
void CharacterController::Initialize()
{
	// カメラオブジェクトのTransformを取得する
    auto& mgr = SystemLocator::Get<GameObjectManager>();
    GameObject* camObj = mgr.GetFindObjectByName("Camera3D");
    if (!camObj)
    {
        std::cout << "[CharacterController] Camera3D not found.\n";
        return;
    }

    this->cameraTransform = camObj->GetComponent<Transform>();
    if (!this->cameraTransform)
    {
        std::cout << "[CharacterController] Camera3D has no Transform.\n";
        return;
    }

	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
	if (!this->rigidbody)
	{
		std::cout << "[CharacterController] Rigidbody3D component not found on owner.\n";
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
    if (!this->rigidbody || !this->cameraTransform) { return; }

    // ------------------------------------------------------
    // 入力処理
    // ------------------------------------------------------
    float inputX = 0.0f;
    float inputZ = 0.0f;

    if (this->inputSystem.IsActionPressed("MoveForward")) { inputZ += 1.0f; }
    if (this->inputSystem.IsActionPressed("MoveBackward")) { inputZ -= 1.0f; }
    if (this->inputSystem.IsActionPressed("MoveLeft")) { inputX -= 1.0f; }
    if (this->inputSystem.IsActionPressed("MoveRight")) { inputX += 1.0f; }

    // 現在の物理速度を取得（Y成分は重力などの物理によるものを保持する）
    DX::Vector3 currentVel = this->rigidbody->GetLinearVelocity();

    // 入力がない場合は水平方向の速度のみゼロにして終了（重力は維持）
    if (inputX == 0.0f && inputZ == 0.0f) {
        this->rigidbody->SetLinearVelocity(DX::Vector3(0.0f, currentVel.y, 0.0f));
        return;
    }

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
    // 入力方向をワールドベクトルへ変換
    // ------------------------------------------------------
    DX::Vector3 moveDir = camForward * inputZ + camRight * inputX;

    if (moveDir.LengthSquared() > 0.0f)
    {
        moveDir.Normalize();
    }
    else {
        // 安全のため水平速度のみゼロに（重力は維持）
        this->rigidbody->SetLinearVelocity(DX::Vector3(0.0f, currentVel.y, 0.0f));
        return;
    }

    // ------------------------------------------------------
    // 回転の更新（キャラクターを進行方向へ向ける）
    // → Rigidbody の論理回転に反映
    // ------------------------------------------------------
    DX::Quaternion currentRot = this->rigidbody->GetLogicalRotation();
    DX::Quaternion targetRot = DX::Quaternion::CreateFromRotationMatrix(
        DX::CreateWorldLH(DX::Vector3::Zero, moveDir, DX::Vector3::UnitY)
    );

    DX::Quaternion newRot = DX::Quaternion::Slerp(currentRot, targetRot, this->turnSpeed * _deltaTime);
    this->rigidbody->SetLogicalRotation(newRot);

    // ------------------------------------------------------
    // 移動処理（物理へ委譲：線形速度を設定）
    // ------------------------------------------------------
    DX::Vector3 desiredVel = moveDir * this->moveSpeed;
    // Y成分は重力による既存の速度を維持して、水平成分のみ上書き
    desiredVel.y = currentVel.y;
    this->rigidbody->SetLinearVelocity(desiredVel);
}