/** @file   TestDodge.cpp
 *  @brief  ジャスト回避挙動を検証するテスト用コンポーネント
 *  @date   2026/01/18
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include<iostream>

#include "Include/Tests/TestDodge.h"

#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/DirectInputDevice.h"
#include "Include/Framework/Core/SystemLocator.h"

//-----------------------------------------------------------------------------
// TestDodge class
//-----------------------------------------------------------------------------

TestDodge::TestDodge(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	dodgeComponent(nullptr),
	animComponent(nullptr),
	timeScaleGroup(nullptr),
	inputSystem(SystemLocator::Get<InputSystem>()),
	rigidbody(nullptr),
	prevIsDodging(false),
	prevTimingValid(false)
{
}

void TestDodge::Initialize()
{
	//------------------------------------------------------
	// 必要なコンポーネントの取得
	//------------------------------------------------------
	this->animComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->timeScaleGroup = this->Owner()->GetComponent<TimeScaleGroup>();
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
	this->dodgeComponent = this->Owner()->GetComponent<DodgeComponent>();

	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("Dodge", static_cast<int>(DirectInputDevice::KeyboardKey::B));	// 左クリックで回避
	this->inputSystem.RegisterKeyBinding("Idle", static_cast<int>(DirectInputDevice::KeyboardKey::V));	// 左クリックで回避
	this->inputSystem.RegisterKeyBinding("Jump", static_cast<int>(DirectInputDevice::KeyboardKey::C));	// 左クリックで回避

	//------------------------------------------------------
	// 状態初期化
	//------------------------------------------------------
	this->prevIsDodging = false;
	this->prevTimingValid = false;
}

void TestDodge::Dispose()
{
}

void TestDodge::Update(float _deltaTime)
{
	// 回避開始（押した瞬間）
	if (this->inputSystem.IsActionTriggered("Dodge"))
	{
		if (this->dodgeComponent)
		{
			// 回避の持続（例：0.35秒）
			this->dodgeComponent->StartDodge(0.35f);

			//std::cout
			//	<< "[TestDodge] Dodge Triggered"
			//	<< " IsDodging=" << (this->dodgeComponent->IsDodging() ? "true" : "false")
			//	<< " TimingValid=" << (this->dodgeComponent->IsDodgeTimingValid() ? "true" : "false")
			//	<< std::endl;
		}

		// アニメは今のままでOK（見た目確認用）
		if (this->animComponent)
		{
			this->animComponent->RequestState(TestPlayerAnimState::Dodging, 0.5f);
		}

		// 移動（速度付与）も今のままでOK（挙動確認用）
		if (this->rigidbody)
		{
			DX::Vector3 curVel = this->rigidbody->GetLinearVelocity();

			DX::Quaternion rot = this->rigidbody->GetLogicalRotation();
			DX::Vector3 right = DX::Vector3::Transform(DX::Vector3::Right, rot);
			DX::Vector3 left = -right;

			const float dodgeSpeed = 8.0f;
			DX::Vector3 dodgeVel = left * dodgeSpeed;
			dodgeVel.y = curVel.y;

			this->rigidbody->SetLinearVelocity(dodgeVel);
		}
	}

	if (this->inputSystem.IsActionTriggered("Idle"))
	{
		if (this->animComponent)
		{
			this->animComponent->RequestState(TestPlayerAnimState::Idle, 0.5f);
		}
	}

	if (this->inputSystem.IsActionTriggered("Jump"))
	{
		if (this->animComponent)
		{
			this->animComponent->RequestState(TestPlayerAnimState::Jumping, 0.5f);
		}
	}

	// 状態変化の観測（DodgeComponentのテスト）
	if (this->dodgeComponent)
	{
		const bool nowIsDodging = this->dodgeComponent->IsDodging();
		const bool nowTimingValid = this->dodgeComponent->IsDodgeTimingValid();

		if (nowIsDodging != this->prevIsDodging)
		{
			std::cout
				<< "[TestDodge] IsDodging changed: "
				<< (this->prevIsDodging ? "true" : "false")
				<< " -> "
				<< (nowIsDodging ? "true" : "false")
				<< std::endl;
		}

		if (nowTimingValid != this->prevTimingValid)
		{
			std::cout
				<< "[TestDodge] TimingValid changed: "
				<< (this->prevTimingValid ? "true" : "false")
				<< " -> "
				<< (nowTimingValid ? "true" : "false")
				<< std::endl;
		}

		this->prevIsDodging = nowIsDodging;
		this->prevTimingValid = nowTimingValid;
	}
}
