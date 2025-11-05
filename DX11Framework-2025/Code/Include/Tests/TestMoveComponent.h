#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
#include"Include/Framework/Core/InputSystem.h"

class TestMoveComponent : public Component, public IUpdatable
{
public:
	TestMoveComponent(GameObject* owner, bool isActive = true);
	~TestMoveComponent()override;
	void Initialize() override;
	void Update(float _deltaTime) override;
	void Dispose() override;
private:
	Transform* transform;
	float speed = 10.0f; // 移動速度（ピクセル/秒）
	InputSystem& inputSystem;
};