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
    if (!this->transform) return;

    DX::Vector3 movement(0.0f, 0.0f, 0.0f);
    DX::Vector3 rotationInput(0.0f, 0.0f, 0.0f);

    // --- 移動入力 ---
    if (this->inputSystem.IsActionPressed("MoveDown"))  movement.y += 1.0f;
    if (this->inputSystem.IsActionPressed("MoveUp"))    movement.y -= 1.0f;
    if (this->inputSystem.IsActionPressed("MoveLeft"))  movement.x -= 1.0f;
    if (this->inputSystem.IsActionPressed("MoveRight")) movement.x += 1.0f;
    if (this->inputSystem.IsActionPressed("MoveForward")) movement.z += 1.0f;
    if (this->inputSystem.IsActionPressed("MoveBack"))    movement.z -= 1.0f;

    // --- 回転入力（クォータニオンベース） ---
    if (this->inputSystem.IsActionPressed("RotateLeft"))  rotationInput.y -= 1.0f;
    if (this->inputSystem.IsActionPressed("RotateRight")) rotationInput.y += 1.0f;
    if (this->inputSystem.IsActionPressed("RotateUp"))    rotationInput.x -= 1.0f;
    if (this->inputSystem.IsActionPressed("RotateDown"))  rotationInput.x += 1.0f;

    // --- 移動処理 ---
    if (movement.LengthSquared() > 0.0f)
    {
        movement.Normalize();
        movement *= this->speed * _deltaTime;

        DX::Vector3 pos = this->transform->GetLocalPosition();
        pos += movement;
        this->transform->SetLocalPosition(pos);
    }

    // --- 回転処理 ---
    if (rotationInput.LengthSquared() > 0.0f)
    {
        rotationInput *= this->rotationSpeed * _deltaTime;

        // 現在の回転を取得
        DX::Quaternion currentRot = this->transform->GetLocalRotation();

        // 回転入力をクォータニオンに変換（YawPitchRoll順で適用）
        DX::Quaternion deltaRot = DX::Quaternion::CreateFromYawPitchRoll(
            rotationInput.y, rotationInput.x, rotationInput.z
        );

        // 合成
        DX::Quaternion newRot = deltaRot * currentRot;
        newRot.Normalize();

        this->transform->SetLocalRotation(newRot);
    }
}

void TestMoveComponent::Dispose() {}
