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

#include <iostream>

//-----------------------------------------------------------------------------
// CharacterController class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
CharacterController::CharacterController(GameObject* _owner, bool _active)
	: Component(_owner, _active)
	, inputSystem(SystemLocator::Get<InputSystem>())
	, timeScaleSystem(SystemLocator::Get<TimeScaleSystem>())
{
}

/// @brief 初期化処理
void CharacterController::Initialize()
{
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

	this->attackComponent = this->Owner()->GetComponent<AttackComponent>();
	if (!this->attackComponent)
	{
		std::cout << "[CharacterController] AttackComponent not found on owner.\n";
	}

	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
	if (!this->animationComponent)
	{
		std::cout << "[CharacterController] AnimationComponent not found on owner.\n";
	}

	this->moveComponent = this->Owner()->GetComponent<MoveComponent>();
	if (!this->moveComponent)
	{
		std::cout << "[CharacterController] MoveComponent not found on owner.\n";
	}
	else
	{
		this->moveComponent->SetMoveParams(this->moveSpeed, this->turnSpeed);
	}

	this->dodgeComponent = this->Owner()->GetComponent<DodgeComponent>();
	if (!this->dodgeComponent)
	{
		std::cout << "[CharacterController] DodgeComponent not found on owner.\n";
	}

	//-----------------------------------------------------------------------------
	// キーバインドの登録（暫定）
	//-----------------------------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("MoveForward", static_cast<int>(DirectInputDevice::KeyboardKey::W));
	this->inputSystem.RegisterKeyBinding("MoveBackward", static_cast<int>(DirectInputDevice::KeyboardKey::S));
	this->inputSystem.RegisterKeyBinding("MoveLeft", static_cast<int>(DirectInputDevice::KeyboardKey::A));
	this->inputSystem.RegisterKeyBinding("MoveRight", static_cast<int>(DirectInputDevice::KeyboardKey::D));

	this->inputSystem.RegisterKeyBinding("Punch", static_cast<int>(DirectInputDevice::MouseButton::Left));
	this->inputSystem.RegisterKeyBinding("Dodge", static_cast<int>(DirectInputDevice::MouseButton::Right));

	//-----------------------------------------------------------------------------
	// 攻撃定義（テスト用）
	//-----------------------------------------------------------------------------
	this->currentAttackDef.attackClip = "Punch";
	this->currentAttackDef.attackType = AttackType::Melee;
	this->currentAttackDef.damage = 10.0f;

	this->currentState = PlayerState::Normal;
	this->previousState = PlayerState::Normal;

	this->counterTarget = nullptr;

	this->pendingCounter = false;
	this->pendingCounterTarget = nullptr;
	this->pendingCounterType = AttackType::Melee;

	this->counterRemainingSec = 0.0f;

	this->StateEnter();
}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void CharacterController::Update(float _deltaTime)
{
	if (!this->cameraTransform) { return; }

	//-----------------------------------------------------------------------------
	// 入力処理
	//-----------------------------------------------------------------------------
	float inputX = 0.0f;
	float inputZ = 0.0f;

	if (this->inputSystem.IsActionPressed("MoveForward")) { inputZ += 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveBackward")) { inputZ -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveLeft")) { inputX -= 1.0f; }
	if (this->inputSystem.IsActionPressed("MoveRight")) { inputX += 1.0f; }

	//-----------------------------------------------------------------------------
	// 移動指示（毎フレーム適用）
	//-----------------------------------------------------------------------------
	if (this->moveComponent)
	{
		this->moveComponent->ClearMoveIntent();

		if (!(inputX == 0.0f && inputZ == 0.0f))
		{
			DX::Vector3 camForward = this->cameraTransform->Forward();
			camForward.y = 0.0f;

			DX::Vector3 camRight = this->cameraTransform->Right();
			camRight.y = 0.0f;

			if (camForward.LengthSquared() > 1.0e-6f && camRight.LengthSquared() > 1.0e-6f)
			{
				camForward.Normalize();
				camRight.Normalize();

				DX::Vector3 moveDir = camForward * inputZ + camRight * inputX;
				moveDir.y = 0.0f;

				if (moveDir.LengthSquared() > 1.0e-6f)
				{
					moveDir.Normalize();
					this->moveComponent->SetMoveIntentWorld(moveDir, 1.0f);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// 状態更新（Enter/Update/Exit）
	//-----------------------------------------------------------------------------
	const PlayerState prevState = this->currentState;

	this->StateUpdate(_deltaTime);

	if (this->currentState != prevState)
	{
		{
			const PlayerState temp = this->currentState;
			this->currentState = prevState;
			this->StateExit();
			this->currentState = temp;
		}

		this->StateEnter();
		this->previousState = this->currentState;
	}
}

void CharacterController::StateEnter()
{
	switch (this->currentState)
	{
	case CharacterController::PlayerState::Normal:
		if (this->animationComponent)
		{
			this->animationComponent->RequestState(PlayerAnimState::Idle, 0.2f);
		}
		break;

	case CharacterController::PlayerState::Attacking:
		if (this->animationComponent)
		{
			this->animationComponent->RequestState(PlayerAnimState::Punching, 0.2f);
		}
		if (this->attackComponent)
		{
			this->attackComponent->StartAttack(this->currentAttackDef);
		}
		break;

	case CharacterController::PlayerState::Dodging:
		if (this->animationComponent)
		{
			this->animationComponent->RequestState(PlayerAnimState::Dodging, 0.10f);
		}

		if (this->dodgeComponent)
		{
			this->dodgeComponent->StartDodge(1.0f);
		}
		break;

	case CharacterController::PlayerState::Jumping:
		break;

	case CharacterController::PlayerState::Countering:
		this->counterRemainingSec = this->counterTimeoutSec;

		if (this->animationComponent)
		{
			this->animationComponent->RequestState(PlayerAnimState::Idle, 0.2f);
		}
		break;

	default:
		break;
	}
}

void CharacterController::StateUpdate(float _deltaTime)
{
	switch (this->currentState)
	{
	case CharacterController::PlayerState::Normal:
		if (this->inputSystem.IsActionTriggered("Dodge"))
		{
			std::cout << "DODGE\n";
			this->currentState = PlayerState::Dodging;
			break;
		}

		if (this->inputSystem.IsActionTriggered("Punch"))
		{
			std::cout << "PUNCH\n";
			this->currentState = PlayerState::Attacking;
			break;
		}
		break;

	case CharacterController::PlayerState::Attacking:
		if (this->attackComponent && !this->attackComponent->IsAttacking())
		{
			this->currentState = PlayerState::Normal;
		}
		break;

	case CharacterController::PlayerState::Dodging:
		//-----------------------------------------------------------------------------
		// ジャスト回避成立後の処理
		// 回避が終わるまで見た目は回避のまま維持し、終わった瞬間に追撃へ分岐する
		//-----------------------------------------------------------------------------
		if (this->IsDodgeFinished())
		{
			if (this->pendingCounter && this->pendingCounterTarget)
			{
				this->counterTarget = this->pendingCounterTarget;

				this->pendingCounter = false;
				this->pendingCounterTarget = nullptr;

				this->currentState = PlayerState::Countering;
			}
			else
			{
				this->currentState = PlayerState::Normal;
			}
		}
		break;

	case CharacterController::PlayerState::Jumping:
		break;

	case CharacterController::PlayerState::Countering:
		//-----------------------------------------------------------------------------
		// 追撃：一定時間内にターゲットへ近づけなければ諦めて通常へ戻す
		// 到達したら即パンチ開始し、攻撃が始まらなければタイムアウトで戻る
		//-----------------------------------------------------------------------------
		if (!this->counterTarget || !this->moveComponent || !this->attackComponent)
		{
			this->counterTarget = nullptr;
			this->currentState = PlayerState::Normal;
			break;
		}

		// 攻撃せずに追撃時間を追えたら自動的に追撃終了
		this->counterRemainingSec -= _deltaTime;
		if (this->counterRemainingSec <= 0.0f)
		{
			this->counterTarget = nullptr;
			this->currentState = PlayerState::Normal;
			break;
		}

		// 追撃時に近づく
		{
			Transform* myTr = this->Owner()->GetComponent<Transform>();
			Transform* targetTr = this->counterTarget->GetComponent<Transform>();
			if (!myTr || !targetTr)
			{
				this->counterTarget = nullptr;
				this->currentState = PlayerState::Normal;
				break;
			}

			DX::Vector3 myPos = myTr->GetWorldPosition();
			DX::Vector3 targetPos = targetTr->GetWorldPosition();

			DX::Vector3 dir = targetPos - myPos;
			dir.y = 0.0f;

			if (dir.LengthSquared() <= 1.0e-6f)
			{
				this->currentState = PlayerState::Normal;
				break;
			}

			// 近づいたら攻撃処理に移行
			const float dist = dir.Length();
			if (dist <= this->counterStopDistance)
			{
				this->attackComponent->StartAttack(this->currentAttackDef);

				this->counterTarget = nullptr;
				this->currentState = PlayerState::Attacking;
				break;
			}

			dir.Normalize();
			this->moveComponent->SetMoveIntentWorld(dir, 1.0f);
		}
		break;

	default:
		break;
	}
}

void CharacterController::StateExit()
{
	switch (this->currentState)
	{
	case CharacterController::PlayerState::Normal:
		break;

	case CharacterController::PlayerState::Attacking:
		break;

	case CharacterController::PlayerState::Dodging:
		break;

	case CharacterController::PlayerState::Jumping:
		break;

	case CharacterController::PlayerState::Countering:
		break;

	default:
		break;
	}
}

void CharacterController::OnJustDodgeSuccess(GameObject* _attacker, AttackType _type)
{
	std::cout << "JUST SUCCESS\n";

	//-----------------------------------------------------------------------------
	// スローは成立した瞬間に要求
	//-----------------------------------------------------------------------------
	this->timeScaleSystem.RequestEvent(TimeScaleEventId::TestDodge);

	//-----------------------------------------------------------------------------
	// 回避を強制終了しない
	//-----------------------------------------------------------------------------
	if (_type == AttackType::Melee)
	{
		this->pendingCounter = true;
		this->pendingCounterTarget = _attacker;
		this->pendingCounterType = _type;
	}
}

bool CharacterController::IsDodgeFinished() const
{
	if (!this->dodgeComponent) { return true; }
	return !this->dodgeComponent->IsDodging();
}