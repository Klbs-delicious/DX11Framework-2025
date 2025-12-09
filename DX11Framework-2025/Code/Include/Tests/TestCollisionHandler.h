/** @file   TestCollisionHandler.h
 *  @brief  自由に移動できるデバッグ移動用コンポーネント
 *  @date   2025/11/18
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/InputSystem.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

 /** @class TestCollisionHandler
  *  @brief 自由に移動・回転可能なデバッグ移動用コンポーネント
  *  @details
  *          - WASD で前後左右移動
  *          - E/Q で上下移動
  *          - マウス移動で視点回転
  */
class TestCollisionHandler : public Component, public BaseColliderDispatcher3D
{
public:
    TestCollisionHandler(GameObject* _owner, bool _active = true);
    virtual ~TestCollisionHandler() = default;

    void Initialize() override;
    void Dispose() override;

	void OnCollisionEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other) override;
	void OnCollisionStay(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other) override;
	void OnCollisionExit(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other) override;
private:
};