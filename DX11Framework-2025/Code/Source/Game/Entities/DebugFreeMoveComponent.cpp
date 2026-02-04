/** @file   DebugFreeMoveComponent.cpp
 *  @brief  自由視点移動デバッグコンポーネントの実装
 *  @date   2025/11/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/DebugFreeMoveComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"
#include "Include/Framework/Utils/CommonTypes.h"

//-----------------------------------------------------------------------------
// DebugFreeMoveComponent class
//-----------------------------------------------------------------------------

DebugFreeMoveComponent::DebugFreeMoveComponent(GameObject* _owner, bool _active)
    : Component(_owner, _active),
    inputSystem(SystemLocator::Get<InputSystem>()),
    moveSpeed(10.0f),
    sensitivity(0.1f),
    yaw(0.0f),
    pitch(0.0f)
{}

void DebugFreeMoveComponent::Initialize()
{
    // キーをアクション化
    this->inputSystem.RegisterKeyBinding("Free_MoveForward",
        static_cast<int>(DirectInputDevice::KeyboardKey::UpArrow));
    this->inputSystem.RegisterKeyBinding("Free_MoveBackward",
        static_cast<int>(DirectInputDevice::KeyboardKey::DownArrow));
    this->inputSystem.RegisterKeyBinding("Free_MoveLeft",
        static_cast<int>(DirectInputDevice::KeyboardKey::LeftArrow));
    this->inputSystem.RegisterKeyBinding("Free_MoveRight",
        static_cast<int>(DirectInputDevice::KeyboardKey::RightArrow));
    this->inputSystem.RegisterKeyBinding("Free_MoveUp",
        static_cast<int>(DirectInputDevice::KeyboardKey::E));
    this->inputSystem.RegisterKeyBinding("Free_MoveDown",
        static_cast<int>(DirectInputDevice::KeyboardKey::Q));
}

void DebugFreeMoveComponent::Dispose()
{}

void DebugFreeMoveComponent::Update(float _deltaTime)
{
    Transform* tf = this->Owner()->transform;
    if (!tf) { return; }

    //---------------------------------------------------------
    // マウス入力
    //---------------------------------------------------------
    int mouseX = 0;
    int mouseY = 0;
    this->inputSystem.GetMouseDelta(mouseX, mouseY);

    this->yaw += mouseX * this->sensitivity;
    this->pitch += mouseY * this->sensitivity;

    // ピッチ制限（FPS 方式）
    if (this->pitch > 89.0f)  this->pitch = 89.0f;
    if (this->pitch < -89.0f) this->pitch = -89.0f;

    //---------------------------------------------------------
    // Transform に回転を反映させる
    //---------------------------------------------------------
    DX::Quaternion q =
        DX::Quaternion::CreateFromYawPitchRoll(
            DX::ToRadians(this->yaw),
            DX::ToRadians(this->pitch),
            0.0f);

    tf->SetWorldRotation(q); 

    //---------------------------------------------------------
    // 移動方向を Transform から取得する
    //---------------------------------------------------------
    DX::Vector3 forward = tf->Forward();
    DX::Vector3 right = tf->Right();
    DX::Vector3 up = DX::Vector3(0.0f, 1.0f, 0.0f);

    DX::Vector3 move = DX::Vector3::Zero;

    if (this->inputSystem.IsActionPressed("Free_MoveForward"))  move += forward;
    if (this->inputSystem.IsActionPressed("Free_MoveBackward")) move -= forward;
    if (this->inputSystem.IsActionPressed("Free_MoveRight"))    move += right;
    if (this->inputSystem.IsActionPressed("Free_MoveLeft"))     move -= right;
    if (this->inputSystem.IsActionPressed("Free_MoveUp"))       move += up;
    if (this->inputSystem.IsActionPressed("Free_MoveDown"))     move -= up;

    //---------------------------------------------------------
    // 移動処理
    //---------------------------------------------------------
    if (move.LengthSquared() > 0.0f)
    {
        move.Normalize();
        DX::Vector3 pos = tf->GetWorldPosition();
        pos += move * this->moveSpeed * _deltaTime;
        tf->SetWorldPosition(pos);
    }
}