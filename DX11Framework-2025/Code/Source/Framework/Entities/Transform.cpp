/** @file   Transform.cpp
*   @date   2025/09/19
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

//-----------------------------------------------------------------------------
// Transform class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
 *  @param GameObject* _owner	このコンポーネントがアタッチされるオブジェクト
 *  @param bool _active	コンポーネントの有効/無効
 */
Transform::Transform(GameObject* _owner, bool _isActive) :
    Component(_owner, _isActive),
    isDirty(false), parent(_owner->transform), children(),
	position(), rotation(), scale(1.0f, 1.0f, 1.0f),
    localPosition(), localRotation(), localScale(1.0f, 1.0f, 1.0f)
{
    std::cout << "Transformコンポーネントの生成" << std::endl;
}

/// @brief デストラクタ
Transform::~Transform() {}

/// @brief 初期化処理
void Transform::Initialize() {}

/// @brief 終了処理
void Transform::Dispose() {}

/**	@brief ワールド変換行列を更新
 *	@details 親Transformがいる場合は親のワールド変換行列も考慮して更新する
 */
void Transform::UpdateWorldMatrix()
{
    if (!this->isDirty) return;

    DX::Matrix4x4 localMatrix =
        DX::Matrix4x4::CreateScale(this->localScale) *
        DX::Matrix4x4::CreateFromQuaternion(this->localRotation) *
        DX::Matrix4x4::CreateTranslation(this->localPosition);

    if (this->parent)
    {
        this->parent->UpdateWorldMatrix();
        this->worldMatrix = localMatrix * this->parent->worldMatrix;

        // 親のスケールを考慮してワールドスケールを計算
        this->scale.x = this->parent->scale.x * this->localScale.x;
        this->scale.y = this->parent->scale.y * this->localScale.y;
        this->scale.z = this->parent->scale.z * this->localScale.z;
    }
    else
    {
        this->worldMatrix = localMatrix;
        this->scale = this->localScale;
    }

    // キャッシュ更新
    this->position = this->worldMatrix.Translation();
    this->rotation = DX::Quaternion::CreateFromRotationMatrix(this->worldMatrix);

    this->isDirty = false;
}

/// -------------------------------------------------------------

/**@brief ワールド空間の座標を設定
 * @param const DX::Vector3& _position
 */
void Transform::SetWorldPosition(const DX::Vector3& _position)
{
	this->localPosition = WorldToLocalPosition(_position);
    this->isDirty = true;
}

/**@brief ワールド空間の座標を取得
 * @return DX::Vector3	ワールド空間の座標
 */
DX::Vector3 Transform::GetWorldPosition() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return this->position;
}

/**@brief ワールド空間の回転を設定
 * @param const DX::Quaternion& _rotation
 */
void Transform::SetWorldRotation(const DX::Quaternion& _rotation)
{
    this->localRotation = WorldToLocalRotation(_rotation);
    this->isDirty = true;
}


/**@brief ワールド空間の回転を取得
 * @return DX::Quaternion	ワールド空間の回転
 */
DX::Quaternion Transform::GetWorldRotation() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return this->rotation;
}

/**@brief ワールド空間のスケールを設定
 * @param const DX::Vector3& _scale
 */
void Transform::SetWorldScale(const DX::Vector3& _scale)
{
    this->localScale = WorldToLocalScale(_scale);
    this->isDirty = true;
}

/**@brief ワールド空間のスケールを取得
 * @return DX::Vector3	ワールド空間のスケール
 */
DX::Vector3 Transform::GetWorldScale() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return this->scale;
}

/// -------------------------------------------------------------

/**@brief ローカル空間の座標を設定
 * @param const DX::Vector3& _localPosition
 */
void Transform::SetLocalPosition(const DX::Vector3& _localPosition)
{
    this->localPosition = _localPosition;
    this->isDirty = true;
}

/**@brief ローカル空間の座標を取得
 * @return DX::Vector3  ローカル空間の座標
 */
DX::Vector3 Transform::GetLocalPosition() const
{
    return this->localPosition;
}

/**@brief ローカル空間の回転を設定
 * @param const DX::Quaternion& _localRotation
 */
void Transform::SetLocalRotation(const DX::Quaternion& _localRotation)
{
    this->localRotation = _localRotation;
    this->isDirty = true;
}

/**@brief ローカル空間の回転を取得
 * @return DX::Quaternion   ローカル空間の回転
 */
DX::Quaternion Transform::GetLocalRotation() const
{
    return this->localRotation;
}

/**@brief ローカル空間のスケールを設定
 * @param const DX::Vector3& _localScale
 */
void Transform::SetLocalScale(const DX::Vector3& _localScale)
{
    this->localScale = _localScale;
    this->isDirty = true;
}

/**@brief ローカル空間のスケールを取得
 * @return DX::Vector3  ローカル空間のスケール
 */
DX::Vector3 Transform::GetLocalScale() const
{
    return this->localScale;
}

/// -------------------------------------------------------------

/**@brief 親Transformを設定
 * @param Transform* _parent 親Transform
 */
void Transform::SetParent(Transform* _parent)
{
    // 自己参照防止
	if (_parent == this) { return; }

    // 循環参照防止
    Transform* ancestor = _parent;
    while (ancestor)
    {
        if (ancestor == this)
            return; // 循環参照になるので何もしない
        ancestor = ancestor->parent;
    }

    if (this->parent == _parent) { return; }

    // 旧親から自分を削除
    if (this->parent)
    {
        this->parent->RemoveChild(this);
    }

    this->parent = _parent;

    // 新しい親に自分を追加
    if (this->parent)
    {
        this->parent->AddChild(this);
    }

    this->isDirty = true;
}

/**@brief 子Transformを追加
 * @param Transform* _child 追加する子Transform
 */
void Transform::AddChild(Transform* _child)
{
    if (!_child) return;

    // 重複追加防止
    for (Transform* child : this->children)
    {
        if (child == _child)
            return;
    }

    this->children.push_back(_child);
}

/**@brief 子Transformを削除
 * @param Transform* _child 削除する子Transform
 */
void Transform::RemoveChild(Transform* _child)
{
    if (!_child) return;

    auto it = std::find(this->children.begin(), this->children.end(), _child);
    if (it != this->children.end())
    {
        this->children.erase(it);
    }
}

/// -------------------------------------------------------------

/**@brief ローカル空間の座標をワールド空間に変換する
 * @param const DX::Vector3& _localPoint
 * @return DX::Vector3 ワールド空間座標
 */
DX::Vector3 Transform::TransformPoint(const DX::Vector3& _localPoint) const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return DX::Vector3::Transform(_localPoint, this->worldMatrix);
}

/**@brief ワールド空間の座標をローカル空間に変換する
 * @param const DX::Vector3& _worldPoint
 * @return DX::Vector3	ローカル空間座標
 */
DX::Vector3 Transform::InverseTransformPoint(const DX::Vector3& _worldPoint) const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    DX::Matrix4x4 inv = this->worldMatrix;
    inv.Invert();
    return DX::Vector3::Transform(_worldPoint, inv);
}

/**@brief ローカル空間 → ワールド空間への変換行列を取得する
 * @return const DX::Matrix4x4&	ワールド空間への変換行列
 */
const DX::Matrix4x4& Transform::GetLocalToWorldMatrix() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return this->worldMatrix;
}

/**@brief ワールド空間 → ローカル空間への変換行列を取得する
 * @return const DX::Matrix4x4&	ローカル空間への変換行列
 */
DX::Matrix4x4 Transform::GetWorldToLocalMatrix() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    DX::Matrix4x4 inv = this->worldMatrix;
    inv.Invert();
    return inv;
}

/**@brief ワールド変換行列を取得
 * @return const DX::Matrix4x4&	ワールド変換行列
 */
const DX::Matrix4x4& Transform::GetWorldMatrix() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return this->worldMatrix;
}

/// -------------------------------------------------------------
/**@brief   前方向ベクトルを取得
 * @return DX::Vector3  前方向ベクトル
 */
DX::Vector3 Transform::Forward() const
{
    // +Zを正面にする（DirectX標準）
    const_cast<Transform*>(this)->UpdateWorldMatrix();  
    return DX::Vector3(this->worldMatrix._31, this->worldMatrix._32, this->worldMatrix._33);
}

/**@brief   上方向ベクトルを取得
 * @return DX::Vector3  上方向ベクトル
 */
DX::Vector3 Transform::Up() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return DX::Vector3(this->worldMatrix._21, this->worldMatrix._22, this->worldMatrix._23);
}
/**@brief   右方向ベクトルを取得
 * @return DX::Vector3  右方向ベクトル
 */
DX::Vector3 Transform::Right() const
{
    const_cast<Transform*>(this)->UpdateWorldMatrix();
    return DX::Vector3(this->worldMatrix._11, this->worldMatrix._12, this->worldMatrix._13);
}

/**@brief	指定した位置を向くように回転を設定する
 * @param const DX::Vector3& _target	注視点のワールド座標
 * @param const DX::Vector3& _up		上方向ベクトル（デフォルトはY軸）
 */
void Transform::LookAt(const DX::Vector3& _target, const DX::Vector3& _up)
{
    this->UpdateWorldMatrix();

    DX::Vector3 eye = this->position;

    // forward（Z軸）
    DX::Vector3 forward;
    forward.x = _target.x - eye.x;
    forward.y = _target.y - eye.y;
    forward.z = _target.z - eye.z;
    forward.Normalize();

    // right（X軸）
    DX::Vector3 right;
    right.x = _up.y * forward.z - _up.z * forward.y;
    right.y = _up.z * forward.x - _up.x * forward.z;
    right.z = _up.x * forward.y - _up.y * forward.x;
    right.Normalize();

    // up（Y軸）
    DX::Vector3 up;
    up.x = forward.y * right.z - forward.z * right.y;
    up.y = forward.z * right.x - forward.x * right.z;
    up.z = forward.x * right.y - forward.y * right.x;

	// 回転行列を作成
    DX::Matrix4x4 lookMatrix = DX::Matrix4x4::Identity;
    lookMatrix._11 = right.x;  lookMatrix._12 = right.y;  lookMatrix._13 = right.z;
    lookMatrix._21 = up.x;     lookMatrix._22 = up.y;     lookMatrix._23 = up.z;
    lookMatrix._31 = forward.x; lookMatrix._32 = forward.y; lookMatrix._33 = forward.z;

	// クォータニオンに変換して設定
    DX::Quaternion worldRot = DX::Quaternion::CreateFromRotationMatrix(lookMatrix);
    this->SetWorldRotation(worldRot);
}


/**@brief	指定した軸の周りに回転する
 * @param const DX::Vector3& _center	回転の中心点のワールド座標
 * @param const DX::Vector3& _axis		回転軸（ワールド空間）
 * @param float _angle				回転角（ラジアン）
 */
void Transform::RotateAround(const DX::Vector3& _center, const DX::Vector3& _axis, float _angle)
{
    // ワールド位置を取得
    this->UpdateWorldMatrix();
    DX::Vector3 worldPos = this->position;

    // 中心点からの相対ベクトルを求める（手動減算）
    DX::Vector3 offset;
    offset.x = worldPos.x - _center.x;
    offset.y = worldPos.y - _center.y;
    offset.z = worldPos.z - _center.z;

    // 回転行列を生成（軸・角度から）
    DX::Matrix4x4 rotationMatrix = DX::Matrix4x4::CreateFromAxisAngle(_axis, _angle);

    // 相対ベクトルを回転
    DX::Vector3 rotatedOffset = DX::Vector3::Transform(offset, rotationMatrix);

    // 新しいワールド位置を求める（手動加算）
    DX::Vector3 newWorldPos;
    newWorldPos.x = _center.x + rotatedOffset.x;
    newWorldPos.y = _center.y + rotatedOffset.y;
    newWorldPos.z = _center.z + rotatedOffset.z;

    // 回転も更新（現在の回転に軸回転を連結）
    DX::Quaternion deltaRot = DX::Quaternion::CreateFromAxisAngle(_axis, _angle);
    DX::Quaternion newWorldRot = DX::Quaternion::Concatenate(this->rotation, deltaRot);

    // ワールド空間で更新
    this->SetWorldPosition(newWorldPos);
    this->SetWorldRotation(newWorldRot);
}

/**@brief	クォータニオンをオイラー角に変換
 * @param const DX::Quaternion& _quat	変換するクォータニオン
 * @return DX::Vector3	オイラー角
 */
DX::Vector3 Transform::QuaternionToEuler(const DX::Quaternion& _quat) const
{
    // クォータニオンから回転行列を生成
    DX::Matrix4x4 rotMatrix = DX::Matrix4x4::CreateFromQuaternion(_quat);

    DX::Vector3 euler;
    euler.x = std::asin(-rotMatrix._32);                // Pitch（X軸回転）
    euler.y = std::atan2(rotMatrix._31, rotMatrix._33); // Yaw（Y軸回転）
    euler.z = std::atan2(rotMatrix._12, rotMatrix._22); // Roll（Z軸回転）

    return euler; 
}

// ワールド座標→ローカル座標変換
DX::Vector3 Transform::WorldToLocalPosition(const DX::Vector3& worldPos) const
{
    if (this->parent)
    {
        DX::Matrix4x4 invParent = this->parent->GetLocalToWorldMatrix();
        invParent.Invert();
        return DX::Vector3::Transform(worldPos, invParent);
    }
    else
    {
        return worldPos;
    }
}

// ワールド回転→ローカル回転変換
DX::Quaternion Transform::WorldToLocalRotation(const DX::Quaternion& worldRot) const
{
    if (this->parent)
    {
        DX::Quaternion invParentRot;
        this->parent->GetWorldRotation().Inverse(invParentRot);
        return DX::Quaternion::Concatenate(invParentRot, worldRot);
    }
    else
    {
        return worldRot;
    }
}

// ワールドスケール→ローカルスケール変換
DX::Vector3 Transform::WorldToLocalScale(const DX::Vector3& worldScale) const
{
    if (this->parent)
    {
        DX::Vector3 parentScale = this->parent->GetWorldScale();
        return DX::Vector3(
            parentScale.x != 0.0f ? worldScale.x / parentScale.x : worldScale.x,
            parentScale.y != 0.0f ? worldScale.y / parentScale.y : worldScale.y,
            parentScale.z != 0.0f ? worldScale.z / parentScale.z : worldScale.z
        );
    }
    else
    {
        return worldScale;
    }
}
