/** @file   Camera3D.cpp
 *  @date   2025/10/19
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/Application.h"

//-----------------------------------------------------------------------------
// Camera3D class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
 *  @param bool _isActive コンポーネントの有効/無効
 */
Camera3D::Camera3D(GameObject* _owner, bool _isActive)
    : Component(_owner),
    isDirty(true),
    target(DX::Vector3(0.0f, 0.0f, 1.0f)),
    up(DX::Vector3::Up),
    fovY(DirectX::XMConvertToRadians(60.0f)),
    aspect(16.0f / 9.0f),
    nearZ(0.1f),
    farZ(1000.0f)
{
    // 画面サイズを取得
    auto& window = SystemLocator::Get<WindowSystem>();
    this->screenSize = DX::Vector2(
        static_cast<float>(window.GetWidth()),
        static_cast<float>(window.GetHeight())
    );

    this->transform = this->Owner()->GetComponent<Transform>();
}

/** @brief 初期化処理
 *  @details ビュー・プロジェクション行列を初期計算する
 */
void Camera3D::Initialize()
{
    // 行列の初期計算
    this->UpdateMatrix();
}

/** @brief 更新処理（毎フレーム）
 *  @details Transformの位置や回転に応じて行列を更新する
 */
void Camera3D::Update(float _deltaTime)
{
    // Transform の変化に合わせて更新
    this->UpdateMatrix();
}

/** @brief 終了処理
 */
void Camera3D::Dispose() {}

/** @brief ビュー行列の取得
 *  @return 現在のビュー行列
 */
const DX::Matrix4x4& Camera3D::GetViewMatrix() const
{
    return this->viewMatrix;
}

/** @brief プロジェクション行列の取得
 *  @return 現在のプロジェクション行列
 */
const DX::Matrix4x4& Camera3D::GetProjectionMatrix() const
{
    return this->projectionMatrix;
}

/** @brief 透視投影パラメータの設定
 *  @param float _fovY   視野角（ラジアン）
 *  @param float _aspect アスペクト比
 *  @param float _nearZ  ニアクリップ
 *  @param float _farZ   ファークリップ
 */
void Camera3D::SetPerspective(float _fovY, float _aspect, float _nearZ, float _farZ)
{
    this->fovY = _fovY;
    this->aspect = _aspect;
    this->nearZ = _nearZ;
    this->farZ = _farZ;
    this->isDirty = true;
}

/** @brief 画面サイズの設定
 *  @param float _width 画面の幅
 *  @param float _height 画面の高さ
 */
void Camera3D::SetScreenSize(float _width, float _height)
{
    this->screenSize = DX::Vector2(_width, _height);
    this->aspect = _width / _height;
    this->isDirty = true;
}

/** @brief 注視点の設定
 *  @param const DX::Vector3& _target 注視点
 */
void Camera3D::SetTarget(const DX::Vector3& _target)
{
    this->target = _target;
    this->isDirty = true;
}

/** @brief 上方向ベクトルの設定
 *  @param const DX::Vector3& _up 上方向ベクトル
 */
void Camera3D::SetUp(const DX::Vector3& _up)
{
    this->up = _up;
    this->isDirty = true;
}

/** @brief ビュー・プロジェクション行列を内部的に更新
 *  @details Transformの位置を基準にLookAt行列を生成する
 */
void Camera3D::UpdateMatrix()
{
    if (!this->isDirty && !this->transform->GetIsDirty()) { return; }

    DX::Vector3 pos = this->transform->GetWorldPosition();

    // ビュー行列を作成（LookAt式）
    this->viewMatrix = DX::Matrix4x4::CreateLookAt(pos, this->target, this->up);

    // プロジェクション行列を作成（透視投影）
    this->projectionMatrix = DX::Matrix4x4::CreatePerspectiveFieldOfView(
        this->fovY, this->aspect, this->nearZ, this->farZ
    );

    this->isDirty = false;
}

/** @brief スクリーン座標 → ワールド方向ベクトル変換
 *  @param const DX::Vector2& _screenPos スクリーン座標（ピクセル）
 *  @return ワールド空間でのレイ方向ベクトル
 */
DX::Vector3 Camera3D::ScreenToWorldRay(const DX::Vector2& _screenPos) const
{
    // スクリーン → NDC変換
    float ndcX = (2.0f * _screenPos.x / this->screenSize.x - 1.0f);
    float ndcY = (1.0f - 2.0f * _screenPos.y / this->screenSize.y);

    DX::Vector3 rayNdc = DX::Vector3(ndcX, ndcY, 1.0f);

    // ビュー × プロジェクション行列の逆行列を取得
    DX::Matrix4x4 invViewProj = (this->viewMatrix * this->projectionMatrix).Invert();

    // NDC空間→ワールド空間へ変換
    DX::Vector4 worldPos = DX::Vector4::Transform(DX::Vector4(rayNdc.x, rayNdc.y, rayNdc.z, 1.0f), invViewProj);
    worldPos /= worldPos.w;

    DX::Vector3 camPos = this->transform->GetWorldPosition();
    DX::Vector3 dir = (DX::Vector3(worldPos.x, worldPos.y, worldPos.z) - camPos);
    dir.Normalize();

    return dir;
}