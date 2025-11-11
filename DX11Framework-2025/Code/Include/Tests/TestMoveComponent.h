/** @file   TestMoveComponent.h
 *  @brief  テスト用の移動・回転コンポーネント
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Core/InputSystem.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

class TestMoveComponent : public Component, public IUpdatable
{
public:
	TestMoveComponent(GameObject* owner, bool isActive = true);
	~TestMoveComponent() override;

	void Initialize() override;
	void Update(float _deltaTime) override;
	void Dispose() override;

private:
	Transform* transform = nullptr;
	InputSystem& inputSystem;

	float speed = 5.0f;			///< 移動速度
	float rotationSpeed = 20.0f;	///< 回転速度
};