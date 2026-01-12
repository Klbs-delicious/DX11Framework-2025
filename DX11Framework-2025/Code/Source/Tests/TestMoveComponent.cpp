#include "Include/Tests/TestMoveComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"

#include <iostream>

TestMoveComponent::TestMoveComponent(GameObject* owner, bool isActive)
	: Component(owner, isActive),
	inputSystem(SystemLocator::Get<InputSystem>()),
	speed(5.0f), rotationSpeed(1.5f)
{
	this->transform = owner->GetComponent<Transform>();
}

TestMoveComponent::~TestMoveComponent() {}

void TestMoveComponent::Initialize()
{
	std::cout << "[TestMoveComponent] owner=" << this->Owner()->GetName().c_str()
		<< " transform=" << this->transform << std::endl;		

	// Rigidbody の取得（なければ追加）
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
	if (!this->rigidbody)
	{
		this->rigidbody = this->Owner()->AddComponent<Framework::Physics::Rigidbody3D>();
	}

	// --- 移動用キー ---
	this->inputSystem.RegisterKeyBinding("MoveUp", static_cast<int>(DirectInputDevice::KeyboardKey::UpArrow));
	this->inputSystem.RegisterKeyBinding("MoveDown", static_cast<int>(DirectInputDevice::KeyboardKey::DownArrow));
	this->inputSystem.RegisterKeyBinding("MoveLeft", static_cast<int>(DirectInputDevice::KeyboardKey::LeftArrow));
	this->inputSystem.RegisterKeyBinding("MoveRight", static_cast<int>(DirectInputDevice::KeyboardKey::RightArrow));
	this->inputSystem.RegisterKeyBinding("MoveForward", static_cast<int>(DirectInputDevice::KeyboardKey::W));
	this->inputSystem.RegisterKeyBinding("MoveBack", static_cast<int>(DirectInputDevice::KeyboardKey::S));

	// --- 回転用キー ---
	this->inputSystem.RegisterKeyBinding("RotateLeft", static_cast<int>(DirectInputDevice::KeyboardKey::A));   // 左回転（Yaw -）
	this->inputSystem.RegisterKeyBinding("RotateRight", static_cast<int>(DirectInputDevice::KeyboardKey::D));  // 右回転（Yaw +）
	this->inputSystem.RegisterKeyBinding("RotateUp", static_cast<int>(DirectInputDevice::KeyboardKey::Q));     // 上向き（Pitch -）
	this->inputSystem.RegisterKeyBinding("RotateDown", static_cast<int>(DirectInputDevice::KeyboardKey::E));   // 下向き（Pitch +）
}

void TestMoveComponent::Update(float _deltaTime)
{
	if (!this->transform || !this->rigidbody) return;

	DX::Vector3 movementLocal(0.0f, 0.0f, 0.0f);
	DX::Vector3 rotationInput(0.0f, 0.0f, 0.0f);

	// --- 移動入力（ローカル空間） ---
	if (this->inputSystem.IsActionPressed("MoveDown"))  movementLocal.y += 1.0f;
	if (this->inputSystem.IsActionPressed("MoveUp"))    movementLocal.y -= 1.0f;
	if (this->inputSystem.IsActionPressed("MoveLeft"))  movementLocal.x -= 1.0f;
	if (this->inputSystem.IsActionPressed("MoveRight")) movementLocal.x += 1.0f;
	if (this->inputSystem.IsActionPressed("MoveForward")) movementLocal.z += 1.0f;
	if (this->inputSystem.IsActionPressed("MoveBack"))    movementLocal.z -= 1.0f;

	// --- 回転入力（キーボード） ---
	if (this->inputSystem.IsActionPressed("RotateLeft"))  rotationInput.y -= 1.0f;
	if (this->inputSystem.IsActionPressed("RotateRight")) rotationInput.y += 1.0f;
	if (this->inputSystem.IsActionPressed("RotateUp"))    rotationInput.x -= 1.0f;
	if (this->inputSystem.IsActionPressed("RotateDown"))  rotationInput.x += 1.0f;

	// --- マウス移動による回転入力 ---
	int mouseDX = 0, mouseDY = 0;
	if (this->inputSystem.GetMouseDelta(mouseDX, mouseDY))
	{
		constexpr float MOUSE_SENSITIVITY = 0.1f; // 感度
		rotationInput.y += static_cast<float>(mouseDX) * MOUSE_SENSITIVITY; // 左右（Yaw）
		rotationInput.x += static_cast<float>(mouseDY) * MOUSE_SENSITIVITY; // 上下（Pitch）
	}

	// --- 回転処理（Rigidbody のロジック回転に適用） ---
	if (rotationInput.LengthSquared() > 0.0f)
	{
		rotationInput *= this->rotationSpeed * _deltaTime;

		DX::Quaternion currentRot = this->rigidbody->GetLogicalRotation();
		DX::Quaternion deltaRot = DX::Quaternion::CreateFromYawPitchRoll(
			rotationInput.y, rotationInput.x, rotationInput.z
		);

		DX::Quaternion newRot = deltaRot * currentRot;
		newRot.Normalize();
		this->rigidbody->SetLogicalRotation(newRot);
	}

	// --- 移動処理（速度設定：Rigidbody に委譲） ---
	if (movementLocal.LengthSquared() > 0.0f)
	{
		movementLocal.Normalize();
		// ローカルベクトルをワールド速度へ変換
		const DX::Quaternion worldRot = this->rigidbody->GetLogicalRotation();
		const DX::Vector3 forward = DX::Vector3::Transform(DX::Vector3::Forward, worldRot);
		const DX::Vector3 right   = DX::Vector3::Transform(DX::Vector3::Right, worldRot);
		const DX::Vector3 up      = DX::Vector3::Transform(DX::Vector3::Up, worldRot);

		DX::Vector3 desiredVel = right * movementLocal.x + up * movementLocal.y + forward * movementLocal.z;
		desiredVel *= this->speed; // 単位は units/sec
		this->rigidbody->SetLinearVelocity(desiredVel);
	}
	else
	{
		// 入力がなければ移動を止める
		this->rigidbody->SetLinearVelocity(DX::Vector3::Zero);
	}
}


void TestMoveComponent::Dispose() {}

void TestMoveComponent::OnCollisionEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	std::cout << "[TestMoveComponent] OnCollisionEnter: self=" << _self->Owner()->GetName().c_str()
		<< " other=" << _other->Owner()->GetName().c_str() << std::endl;
}
