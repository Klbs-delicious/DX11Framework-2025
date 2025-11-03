/**	@file	Camera2D.h
*	@date	2025/09/23
*/
#pragma once
#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"

/**	@class	Camera2D
 *	@brief	２Dカメラの情報を管理するコンポーネント
 */
class Camera2D : public Component, public IUpdatable
{
public:
    /** @enum OriginMode
     *  @brief カメラの原点モード
     */
    enum class OriginMode
    {
        Center,     // 中心原点（ゲーム空間）
        TopLeft     // 左上原点（UI）
    };

    /** @brief  コンストラクタ
     *  @param GameObject* _owner	このコンポーネントがアタッチされるオブジェクト
     *  @param bool _active	コンポーネントの有効/無効
     */
    Camera2D(GameObject* _owner, bool _isActive = true);

    ~Camera2D()override = default;

    /// @brief 初期化処理
    void Initialize() override;

    /** @brief 更新処理（毎フレーム）
	*   @param float _deltaTime	前フレームからの経過時間（秒）
    */
    void Update(float _deltaTime) override;

    /// @brief 終了処理
    void Dispose() override;

	/** @brief 画面サイズの設定
	 *  @param float _width 画面の幅
	 *  @param float _height 画面の高さ
     */
    void SetScreenSize(float _width, float _height);

    /** @brief  ビューマトリクスの取得
	 *  @return const DX::Matrix4x4& ビューマトリクス
    */
    const DX::Matrix4x4& GetViewMatrix() const;

    /** @brief プロジェクション行列の取得
	*   @return const DX::Matrix4x4& プロジェクション行列
    */
    const DX::Matrix4x4& GetProjectionMatrix() const;

    /** @brief ズーム倍率の設定
	*   @param float _zoom	ズーム倍率 (1.0f = 100%)
    */
    void SetZoom(float _zoom);

    /// @brief ズーム倍率の取得
    float GetZoom() const;

    /**@brief 画面サイズの設定
	 * @param const DX::Vector2& size 画面サイズ
     */
    void SetScreenSize(const DX::Vector2& size);

	/** @brief 画面サイズの取得
	*   @return const DX::Vector2& 画面サイズ
    */
    const DX::Vector2& GetScreenSize() const;

	/** @brief スクリーン座標をワールド座標に変換
	*   @param const DX::Vector2& screenPos スクリーン座標
	*   @return DX::Vector2 ワールド座標
    */
    DX::Vector2 ScreenToWorld(const DX::Vector2& screenPos) const;

	/** @brief カメラの原点モードを設定
	 *  @param OriginMode _mode 原点モード
	 */
	void SetOriginMode(OriginMode _mode) 
    { 
        this->originMode = this->originMode; 
        this->isDirty = true; 
    }
private:
    /// @brief ビュー・プロジェクション行列を内部的に更新
    void UpdateMatrix();
private:
	bool isDirty;                   ///< 行列の再計算が必要かどうか
    Transform* transform;
    bool transformChanged;          ///< Transformからの通知用

	DX::Matrix4x4 viewMatrix;           ///< ビュー変換行列
	DX::Matrix4x4 projectionMatrix;     ///< プロジェクション変換行列
	DX::Matrix4x4 screenToWorldMatrix;  ///< ビュー × プロジェクション行列（描画や座標変換に使用）

	DX::Vector2 screenSize;             ///< 画面サイズ
	float zoom;                         ///< ズーム倍率

	OriginMode originMode = OriginMode::Center;     ///< カメラの原点モード
};
