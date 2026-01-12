/** @file   TestMoveComponent.h
 *  @brief  テスト用の移動・回転コンポーネント
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/InputSystem.h"

#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Rigidbody3D.h"

class TestMoveComponent : public Component, public IUpdatable, public BaseColliderDispatcher3D
{
public:
	TestMoveComponent(GameObject* owner, bool isActive = true);
	~TestMoveComponent() override;

	void Initialize() override;
	void Update(float _deltaTime) override;
	void Dispose() override;

	void OnCollisionEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other) override;

private:
	Transform* transform = nullptr;
	InputSystem& inputSystem;
	Framework::Physics::Rigidbody3D* rigidbody = nullptr;

	float speed = 5.0f;				///< 移動速度
	float rotationSpeed = 20.0f;	///< 回転速度
};