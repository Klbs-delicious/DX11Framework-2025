/** @file   Camera2D.cpp
*   @date   2025/09/23
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Camera2D.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/Application.h"

//-----------------------------------------------------------------------------
// Camera2D class
//-----------------------------------------------------------------------------

/** @brief  コンストラクタ
 *  @param GameObject* _owner	このコンポーネントがアタッチされるオブジェクト
 *  @param bool _active	コンポーネントの有効/無効
 */
Camera2D::Camera2D(GameObject* _owner, bool _isActive)
    : Component(_owner), 
    isDirty(true), 
    transformChanged(false), 
    zoom(1.0f), 
    originMode(OriginMode::TopLeft)
{
	// 画面サイズを取得
    auto& window = SystemLocator::Get<WindowSystem>();
    this->screenSize = DX::Vector2(static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));

    this->transform = this->Owner()->GetComponent<Transform>();
    if (this->transform)
    {
        // もしTransformが変更されたらカメラの再計算を行う
        this->transform->RegisterOnChanged([this](Transform*)
            {
                this->transformChanged = true;
            });
    }
}

/** @brief 初期化処理
 *  @details ビュー・プロジェクション行列を初期計算する
 */
void Camera2D::Initialize()
{
	// 行列の初期計算
	this->UpdateMatrix();
}

/** @brief 更新処理（毎フレーム）
 *  @details Transformの位置に応じてビュー行列を更新する
 */
void Camera2D::Update(float _deltaTime)
{
    // Transform の変化に合わせて更新
    if (this->isDirty || this->transformChanged)
    {
        this->UpdateMatrix();
        this->isDirty = false;
        this->transformChanged = false;
    }
}

/** @brief 終了処理
 *  @details リソースの解放などがあればここで行う
 */
void Camera2D::Dispose()
{
    if (this->transform)
    {
        this->transform->UnregisterAllCallbacks();
    }
}

/** @brief 画面サイズの設定
 *  @param float _width 画面の幅
 *  @param float _height 画面の高さ
 */
void Camera2D::SetScreenSize(float _width, float _height)
{
    this->screenSize = DX::Vector2(_width, _height);
	this->isDirty = true;  
}

/** @brief ビュー行列の取得
 *  @return 現在のビュー行列
 */
const DX::Matrix4x4& Camera2D::GetViewMatrix() const
{
    return this->viewMatrix;
}

/** @brief プロジェクション行列の取得
 *  @return 現在のプロジェクション行列
 */
const DX::Matrix4x4& Camera2D::GetProjectionMatrix() const
{
    return this->projectionMatrix;
}

/** @brief ズーム倍率の設定
 *  @details プロジェクション行列の再計算が必要になる
 */
void Camera2D::SetZoom(float _zoom)
{
    // 極端に小さい値で描画が崩れないように下限を設定
    this->zoom = std::max(_zoom, 0.001f);
    this->isDirty = true;
}

/** @brief ズーム倍率の取得
 *  @return 現在のズーム倍率
 */
float Camera2D::GetZoom() const
{
    return this->zoom;
}

/** @brief 画面サイズの設定
 *  @details プロジェクション行列の再計算が必要になる
 */
void Camera2D::SetScreenSize(const DX::Vector2& size)
{
    this->screenSize = size;
    this->isDirty = true;
}

/** @brief 画面サイズの取得
 *  @return 現在の画面サイズ
 */
const DX::Vector2& Camera2D::GetScreenSize() const
{
    return this->screenSize;
}

/** @brief スクリーン座標 → ワールド座標変換
 *  @details UIやクリック判定に使用する想定
 */
DX::Vector2 Camera2D::ScreenToWorld(const DX::Vector2& _screenPos) const
{
    if (originMode == OriginMode::Center)
    {
        // スクリーン座標 → NDC（中心原点）
        float ndcX = (_screenPos.x / screenSize.x) * 2.0f - 1.0f;
        float ndcY = 1.0f - (_screenPos.y / screenSize.y) * 2.0f;
        DX::Vector4 ndcPos = DX::Vector4(ndcX, ndcY, 0.0f, 1.0f);

        DX::Vector4 worldPos = DX::Vector4::Transform(ndcPos, screenToWorldMatrix);
        return DX::Vector2(worldPos.x, worldPos.y);
    }
    else if (originMode == OriginMode::TopLeft)
    {
        // スクリーン座標そのままがワールド座標（ピクセル単位）
        return _screenPos;
    }
    return _screenPos;
}

/// @brief ビュー・プロジェクション行列を内部的に更新
void Camera2D::UpdateMatrix()
{
    if (!this->isDirty && !this->transformChanged && !this->transform->GetIsDirty()) { return; }

    DX::Vector3 pos = this->transform->GetWorldPosition();

    // ビュー変換行列とプロジェクション変換行列を初期化
    this->viewMatrix = DX::Matrix4x4::CreateTranslation(-pos.x, -pos.y, 0.0f);

    float halfWidth = this->screenSize.x * 0.5f / this->zoom;
    float halfHeight = this->screenSize.y * 0.5f / this->zoom;

    if (originMode == OriginMode::Center)
    {
        this->projectionMatrix = DX::Matrix4x4::CreateOrthographicOffCenter(
            -halfWidth, halfWidth,
            halfHeight, -halfHeight,
            -1.0f, 1.0f);

        // 描画や座標変換に使用するため、ビュー×プロジェクション行列を計算しておく
        // zoom > 1.0f の場合は拡大（視野が狭くなる）
        // zoom < 1.0f の場合は縮小（視野が広がる）
        DX::Matrix4x4 invView = DX::Matrix4x4::CreateTranslation(pos.x, pos.y, 0.0f);
        float scaleX = this->zoom / this->screenSize.x;
        float scaleY = this->zoom / this->screenSize.y;
        // ズームによって縮小された空間を元に戻すため、スケーリング係数は 1 / zoom
        DX::Matrix4x4 invProj = DX::Matrix4x4::CreateScale(1.0f / scaleX, 1.0f / scaleY, 1.0f);

        this->screenToWorldMatrix = invView * invProj;
    }
    else if (originMode == OriginMode::TopLeft)
    {
        this->viewMatrix = DX::Matrix4x4::Identity;
        this->projectionMatrix = DX::Matrix4x4::CreateOrthographicOffCenter(
            0.0f, this->screenSize.x,
            this->screenSize.y, 0.0f,
            -1.0f, 1.0f);

        // NDC → スクリーン座標 → ワールド座標（そのまま）
        this->screenToWorldMatrix = DX::Matrix4x4::Identity;
    }

    this->isDirty = false;
}