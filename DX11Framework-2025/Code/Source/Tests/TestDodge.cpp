/** @file   TestDodge.cpp
 *  @brief  ジャスト回避挙動を検証するテスト用コンポーネント
 *  @date   2026/01/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Tests/TestDodge.h"

#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/DirectInputDevice.h"
#include "Include/Framework/Core/SystemLocator.h"

//-----------------------------------------------------------------------------
// TestDodge class
//-----------------------------------------------------------------------------

TestDodge::TestDodge(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	isDodging(false),
	dodgeTimer(0.0f),
	animComponent(nullptr),
	timeScaleGroup(nullptr),
	inputSystem(SystemLocator::Get<InputSystem>()),
	rigidbody(nullptr)
{
}

void TestDodge::Initialize()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;

	// 必要なコンポーネントの取得
	this->animComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->timeScaleGroup = this->Owner()->GetComponent<TimeScaleGroup>();
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();

	// ------------------------------------------------------
	// キーバインドの登録
	// ------------------------------------------------------
	this->inputSystem.RegisterKeyBinding("Dodge", static_cast<int>(DirectInputDevice::KeyboardKey::B));	// 左クリックで回避
	this->inputSystem.RegisterKeyBinding("Idle", static_cast<int>(DirectInputDevice::KeyboardKey::V));	// 左クリックで回避
	this->inputSystem.RegisterKeyBinding("Jump", static_cast<int>(DirectInputDevice::KeyboardKey::C));	// 左クリックで回避
}

void TestDodge::Dispose()
{
}



#include<iostream>
void TestDodge::FixedUpdate(float _deltaTime)
{
	if (!this->IsActive()) { return; }

	// 回避中の継続処理
	if (this->isDodging)
	{
		// タイマーを減らす（FixedUpdate は既に時間スケールが考慮されている想定)
		this->dodgeTimer -= _deltaTime;

		if (this->dodgeTimer <= 0.0f)
		{
			this->isDodging = false;

			//// アニメーションを停止（必要なら)
			//if (this->animComponent)
			//{
			//	this->animComponent->Stop();
			//}

			// 回避終了後は水平速度をゼロにする（垂直速度は維持)
			if (this->rigidbody)
			{
				DX::Vector3 curVel = this->rigidbody->GetLinearVelocity();
				this->rigidbody->SetLinearVelocity(DX::Vector3(0.0f, curVel.y, 0.0f));
			}
		}
	}
}

void TestDodge::Update(float _deltaTime)
{
	// 回避開始のトリガー（押した瞬間のみ)
	if (!this->isDodging && this->inputSystem.IsActionTriggered("Dodge"))
	{
		std::cout << "[TestDodge]←ボタン押したマウス" << std::endl;

		// 回避行動を開始する
		this->isDodging = true;
		// 回避時間（秒）
		this->dodgeTimer = 0.35f; // 短いステップ

		// 回避アニメーションを再生する
		if (this->animComponent)
		{
			this->animComponent->RequestState(TestPlayerAnimState::Dodging, 0.5f);
		}

		// Rigidbody を使って左へステップする（ワールドではなくローカルの左)
		if (this->rigidbody)
		{
			// 現在の垂直速度を維持
			DX::Vector3 curVel = this->rigidbody->GetLinearVelocity();

			// キャラクターのローカル右ベクトルを取得して左向きにする
			DX::Quaternion rot = this->rigidbody->GetLogicalRotation();
			DX::Vector3 right = DX::Vector3::Transform(DX::Vector3::Right, rot);
			DX::Vector3 left = -right;

			const float dodgeSpeed = 8.0f; // 回避の横方向速度
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
}
