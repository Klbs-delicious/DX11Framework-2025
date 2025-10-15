#include"Include/Tests/TestMoveComponent.h"
#include"Include/Framework/Entities/GameObject.h"
#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Core/DirectInputDevice.h"

TestMoveComponent::TestMoveComponent(GameObject* owner, bool isActive ):
	Component(owner, isActive),
	inputSystem(SystemLocator::Get<InputSystem>())
{
	this->transform = owner->GetComponent<Transform>();
}

TestMoveComponent::~TestMoveComponent(){}
void TestMoveComponent::Initialize()
{
	// キーバインドの登録（テスト移動用）
	this->inputSystem.RegisterKeyBinding("MoveUp", static_cast<int>(DirectInputDevice::KeyboardKey::UpArrow));
	this->inputSystem.RegisterKeyBinding("MoveDown", static_cast<int>(DirectInputDevice::KeyboardKey::DownArrow));
	this->inputSystem.RegisterKeyBinding("MoveLeft", static_cast<int>(DirectInputDevice::KeyboardKey::LeftArrow));
	this->inputSystem.RegisterKeyBinding("MoveRight", static_cast<int>(DirectInputDevice::KeyboardKey::RightArrow));
}
void TestMoveComponent::Update(float _deltaTime)
{
	DX::Vector3 movement(0.0f, 0.0f, 0.0f);
	if (this->inputSystem.IsActionPressed("MoveDown"))
	{
		movement.y += 1.0f;
	}
	if (this->inputSystem.IsActionPressed("MoveUp"))
	{
		movement.y -= 1.0f;
	}
	if (this->inputSystem.IsActionPressed("MoveLeft"))
	{
		movement.x -= 1.0f;
	}
	if (this->inputSystem.IsActionPressed("MoveRight"))
	{
		movement.x += 1.0f;
	}
	if (movement.LengthSquared() > 0.0f)
	{
		movement.Normalize();
		movement *= this->speed * _deltaTime;
		DX::Vector3 newPosition = this->transform->GetLocalPosition() + movement;
		this->transform->SetLocalPosition(newPosition);
	}
}
void TestMoveComponent::Dispose(){}
