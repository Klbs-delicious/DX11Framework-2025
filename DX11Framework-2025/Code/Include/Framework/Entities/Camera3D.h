/** @file   Camera3D.h
 *  @date   2025/10/19
 */
#pragma once
#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"

 /** @class Camera3D
  *  @brief  3Dカメラの情報を管理するコンポーネント
  *  @details
  *     - ビュー行列は LookAt 行列で算出
  *     - プロジェクション行列は透視投影（Perspective）
  *     - Transformの位置を使用してカメラを制御
  */
class Camera3D : public Component, public IUpdatable
{
public:
    /** @brief コンストラクタ
     *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
     *  @param bool _isActive コンポーネントの有効/無効
     */
    Camera3D(GameObject* _owner, bool _isActive = true);

    ~Camera3D()override = default;

    /** @brief 初期化処理
     *  @details ビュー・プロジェクション行列を初期計算する
     */
    void Initialize() override;

    /** @brief 更新処理（毎フレーム）
     *  @param float _deltaTime 経過時間（秒）
     *  @details Transform の位置やパラメータに応じて行列を更新する
     */
    void Update(float _deltaTime) override;

    /** @brief 終了処理
     *  @details リソースの解放などがあればここで行う
     */
    void Dispose() override;

    /** @brief ビュー行列の取得
     *  @return 現在のビュー行列
     */
    const DX::Matrix4x4& GetViewMatrix() const;

    /** @brief プロジェクション行列の取得
     *  @return 現在のプロジェクション行列
     */
    const DX::Matrix4x4& GetProjectionMatrix() const;

    /** @brief 透視投影の設定
     *  @param float _fovY   視野角（ラジアン）
     *  @param float _aspect アスペクト比
     *  @param float _nearZ  ニアクリップ距離
     *  @param float _farZ   ファークリップ距離
     */
    void SetPerspective(float _fovY, float _aspect, float _nearZ, float _farZ);

    /** @brief 画面サイズの設定
     *  @param float _width  画面の幅
     *  @param float _height 画面の高さ
     */
    void SetScreenSize(float _width, float _height);

    /** @brief 注視点（LookAt先）の設定
     *  @param const DX::Vector3& _target 注視点座標
     */
    void SetTarget(const DX::Vector3& _target);

    /** @brief 上方向ベクトルの設定
     *  @param const DX::Vector3& _up 上方向ベクトル
     */
    void SetUp(const DX::Vector3& _up);

    /** @brief スクリーン座標をワールド方向ベクトルに変換
     *  @param const DX::Vector2& _screenPos スクリーン座標（ピクセル）
     *  @return ワールド空間でのレイ方向ベクトル
     *  @details マウスピッキングなどに使用する
     */
    DX::Vector3 ScreenToWorldRay(const DX::Vector2& _screenPos) const;

private:
    /// @brief ビュー・プロジェクション行列を内部的に更新
    void UpdateMatrix();

private:
    bool isDirty;               ///< 行列再計算が必要かどうか
    Transform* transform;       ///< 所有オブジェクトのTransform

    DX::Matrix4x4 viewMatrix;       ///< ビュー行列
    DX::Matrix4x4 projectionMatrix; ///< プロジェクション行列

    DX::Vector3 target; ///< 注視点
    DX::Vector3 up;     ///< 上方向ベクトル

    float fovY;     ///< 視野角（ラジアン）
    float aspect;   ///< アスペクト比
    float nearZ;    ///< ニアクリップ距離
    float farZ;     ///< ファークリップ距離

    DX::Vector2 screenSize; ///< 画面サイズ
};
