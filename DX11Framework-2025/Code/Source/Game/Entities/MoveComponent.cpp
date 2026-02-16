/** @file   MoveComponent.cpp
 *  @brief  物理移動（速度・回転）を行う共通移動コンポーネント
 *  @date   2026/02/16
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Game/Entities/MoveComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include <cmath>
#include <iostream>

//-----------------------------------------------------------------------------
// MoveComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _isActive コンポーネントの有効/無効
 */
MoveComponent::MoveComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive)
{
}

/// @brief 初期化処理
void MoveComponent::Initialize()
{
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
	if (!this->rigidbody)
	{
		std::cout << "[MoveComponent] Rigidbody3D component not found on owner.\n";
	}
}

/// @brief 更新処理
void MoveComponent::Update(float _deltaTime)
{
	if (!this->rigidbody) { return; }

	// dt が 0 以下でも「このフレームの指示は持ち越さない」
	if (_deltaTime > 0.0f)
	{
		// 物理移動の適用
		this->ApplyToRigidbody(_deltaTime);
	}
	else
	{
		// 不正 dt の場合は安全側で停止させる
		this->StopHorizontal();
	}

	// クリア
	this->hasPendingIntent = false;
	this->pendingDirWorld = DX::Vector3::Zero;
	this->pendingSpeedScale = 1.0f;
}

/** @brief 移動パラメータを設定する
 *  @param _moveSpeed 移動速度
 *  @param _turnSpeed 回転速度
 */
void MoveComponent::SetMoveParams(float _moveSpeed, float _turnSpeed)
{
	this->moveSpeed = _moveSpeed;
	this->turnSpeed = _turnSpeed;
}

/** @brief 移動指示を設定する（ワールド方向）
 *  @param _moveDirWorld 進みたい方向（ワールド）。長さ0なら停止。Yは無視される
 *  @param _speedScale 速度倍率
 */
void MoveComponent::SetMoveIntentWorld(const DX::Vector3& _moveDirWorld, float _speedScale)
{
	DX::Vector3 dir = _moveDirWorld;
	dir.y = 0.0f;

	if (dir.LengthSquared() <= 1.0e-6f)
	{
		// 長さがほぼ0なら停止指示とみなす
		this->hasPendingIntent = false;
		this->pendingDirWorld = DX::Vector3::Zero;
		this->pendingSpeedScale = 1.0f;
		return;
	}

	dir.Normalize();

	this->hasPendingIntent = true;
	this->pendingDirWorld = dir;

	if (_speedScale < 0.0f) { _speedScale = 0.0f; }
	this->pendingSpeedScale = _speedScale;
}

/// @brief 明示的に停止させる
void MoveComponent::ClearMoveIntent()
{
	this->hasPendingIntent = false;
	this->pendingDirWorld = DX::Vector3::Zero;
	this->pendingSpeedScale = 1.0f;
}

/** @brief 移動を許可/禁止する
 *  @param _enabled true で移動適用、false で停止固定
 */
void MoveComponent::SetMoveEnabled(bool _enabled)
{
	this->moveEnabled = _enabled;
}

bool MoveComponent::IsMoving() const
{
	if (!this->rigidbody) { return false; }

	const DX::Vector3 v = this->rigidbody->GetLinearVelocity();
	const float h2 = (v.x * v.x) + (v.z * v.z);
	return (h2 > 1.0e-4f);
}

void MoveComponent::StopHorizontal() const
{
	DX::Vector3 currentVel = this->rigidbody->GetLinearVelocity();
	this->rigidbody->SetLinearVelocity(DX::Vector3(0.0f, currentVel.y, 0.0f));
}

void MoveComponent::ApplyToRigidbody(float _deltaTime)
{
	if (!this->moveEnabled)
	{
		// 移動禁止中は停止固定
		this->StopHorizontal();
		return;
	}

	if (!this->hasPendingIntent)
	{
		//-----------------------------------------------------------------------------
		// このフレームに指示が無ければ停止する（前フレームの指示は保持しない）
		//-----------------------------------------------------------------------------
		this->StopHorizontal();
		return;
	}

	DX::Vector3 currentVel = this->rigidbody->GetLinearVelocity();

	//----------------------------------------
	// 回転と移動の計算
	//----------------------------------------
	const DX::Vector3 moveDir = this->pendingDirWorld;
	const DX::Vector3 facingDir = -moveDir;

	// 回転の更新（進行方向へ向ける）
	DX::Quaternion currentRot = this->rigidbody->GetLogicalRotation();

	DX::Quaternion targetRot = DX::Quaternion::CreateFromRotationMatrix(
		DX::CreateWorldLH(DX::Vector3::Zero, facingDir, DX::Vector3::UnitY)
	);

	//---------------------------------------
	// 回転の安定化
	//---------------------------------------
	if (DX::IsFiniteQuaternion(targetRot))
	{
		DX::Quaternion newRot = DX::Quaternion::Slerp(currentRot, targetRot, this->turnSpeed * _deltaTime);

		if (!DX::IsFiniteQuaternion(newRot) || fabsf(newRot.LengthSquared() - 1.0f) > 1.0e-3f)
		{
			// Slerpの結果が不安定な場合は、直接ターゲット回転を使用してリセットする
			newRot = targetRot;
		}

		// 回転を正規化してから適用する
		newRot.Normalize();
		this->rigidbody->SetLogicalRotation(newRot);
	}

	//---------------------------------------
	// 移動処理
	// --------------------------------------
	const float scaledSpeed = this->moveSpeed * this->pendingSpeedScale;

	// 水平成分のみ上書き（Yは維持）
	DX::Vector3 desiredVel = moveDir * scaledSpeed;
	desiredVel.y = currentVel.y;

	this->rigidbody->SetLinearVelocity(desiredVel);
}
