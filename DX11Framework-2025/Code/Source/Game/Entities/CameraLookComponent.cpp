/** @file   CameraLookComponent.cpp
 *  @brief  カメラの注視点制御を行うコンポーネントの実装
 *  @date   2025/11/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/CameraLookComponent.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Entities/GameObject.h"

//-----------------------------------------------------------------------------
// CameraLookComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
CameraLookComponent::CameraLookComponent(GameObject* _owner, bool _active)
    : Component(_owner, _active),
    inputSystem(SystemLocator::Get<InputSystem>()),
    target(nullptr),
    offset(DX::Vector3(0.0f, 2.0f, -5.0f)), 
	yaw(0.0f),
	pitch(0.0f),
	sensitivity(0.1f),
	smoothSpeed(5.0f)
{}

/// @brief 初期化処理
void CameraLookComponent::Initialize()
{
}

/// @brief 終了処理
void CameraLookComponent::Dispose()
{
}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void CameraLookComponent::Update(float _deltaTime)
{
    if (!target) return;

    //---------------------------------------------------------
    // マウス入力を取得
    //---------------------------------------------------------
    int mouseX = 0;
    int mouseY = 0;
    this->inputSystem.GetMouseDelta(mouseX, mouseY);

    //---------------------------------------------------------
    // 角度を更新する（今は横の角度のみ）
    //---------------------------------------------------------
    this->yaw += mouseX * this->sensitivity;

    //---------------------------------------------------------
    // ターゲット位置を基準に公転させる
    //---------------------------------------------------------
    DX::Vector3 targetPos = target->GetWorldPosition();

    // 半径（オフセット長）を保持
    float radius = offset.Length();
    if (radius < 0.001f) { radius = 0.001f; } // 安定化用

    // offsetを回転させて新しい位置を計算する
    DX::Matrix4x4 rotation = DX::Matrix4x4::CreateRotationY(DX::ToRadians(yaw));
    DX::Vector3 rotatedOffset = DX::Vector3::Transform(offset, rotation);

    // 新しいPivot位置を設定する
    DX::Vector3 newPos = targetPos + rotatedOffset;
    this->Owner()->transform->SetWorldPosition(newPos);

    //---------------------------------------------------------
    // カメラが常にターゲットを向くようにする
    //---------------------------------------------------------
    this->Owner()->transform->LookAt(targetPos);
}