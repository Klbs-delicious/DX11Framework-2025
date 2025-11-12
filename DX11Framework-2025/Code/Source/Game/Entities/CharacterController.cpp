/** @file   CharacterController.cpp
 *  @brief  キャラクターの操作を行う
 *  @date   2025/11/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/CharacterController.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"

//-----------------------------------------------------------------------------
// CharacterController class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
 *  @param bool _active コンポーネントの有効/無効
 */
CharacterController::CharacterController(GameObject* _owner, bool _active)
	: Component(_owner, _active), inputSystem(SystemLocator::Get<InputSystem>()), moveSpeed(10.0f)
{}

/// @brief 初期化処理
void CharacterController::Initialize()
{
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
	auto transform = this->Owner()->transform;
	if (!transform) { return; }

	DX::Vector3 moveDir = DX::Vector3::Zero;

	// ------------------------------------------------------
	// 入力方向（ワールド座標系ベース）
	// ------------------------------------------------------
	if (this->inputSystem.IsActionPressed("MoveForward")) { moveDir.z += 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveBackward")) { moveDir.z -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveLeft")) { moveDir.x -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveRight")) { moveDir.x += 1.0f; }

	// ------------------------------------------------------
	// 移動処理
	// ------------------------------------------------------
	if (moveDir.LengthSquared() > 0.0f)
	{
		moveDir.Normalize();

		// --- 方向回転 ---
		DX::Quaternion currentRot = transform->GetLocalRotation();
		DX::Quaternion targetRot = DX::Quaternion::CreateFromRotationMatrix(
			DX::CreateWorldLH(DX::Vector3::Zero, moveDir, DX::Vector3::UnitY)
		);

		// --- 向きの更新 ---
		float turnSpeed = 15.0f;
		DX::Quaternion newRot = DX::Quaternion::Slerp(currentRot, targetRot, turnSpeed * _deltaTime);
		transform->SetLocalRotation(newRot);

		// --- 前進 ---
		DX::Vector3 pos = transform->GetLocalPosition();
		pos += transform->Forward() * moveSpeed * _deltaTime;
		transform->SetLocalPosition(pos);
	}
}