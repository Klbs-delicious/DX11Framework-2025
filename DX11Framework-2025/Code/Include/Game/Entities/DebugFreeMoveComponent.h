/** @file   DebugFreeMoveComponent.h
 *  @brief  自由に移動できるデバッグ移動用コンポーネント
 *  @date   2025/11/18
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/InputSystem.h"

 /** @class DebugFreeMoveComponent
  *  @brief 自由に移動・回転可能なデバッグ移動用コンポーネント
  *  @details
  *          - WASD で前後左右移動
  *          - E/Q で上下移動
  *          - マウス移動で視点回転
  */
class DebugFreeMoveComponent : public Component, public IUpdatable
{
public:
    DebugFreeMoveComponent(GameObject* _owner, bool _active = true);
    virtual ~DebugFreeMoveComponent() = default;

    void Initialize() override;
    void Dispose() override;
    void Update(float _deltaTime) override;
	void SetSpeed(float _speed) { this->moveSpeed = _speed; }

private:
    InputSystem& inputSystem;
    float moveSpeed;
    float sensitivity;

    float yaw;
    float pitch;
};