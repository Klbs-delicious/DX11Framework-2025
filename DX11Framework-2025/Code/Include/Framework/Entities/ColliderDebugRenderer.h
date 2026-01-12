/** @file   ColliderDebugRenderer.h
 *  @brief  3Dコライダー形状をワイヤーフレームで描画するデバッグコンポーネント
 *  @date   2025/11/29
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/Collider3DComponent.h"
#include "Include/Framework/Entities/Rigidbody3D.h"

#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/DynamicConstantBuffer.h"
#include "Include/Framework/Shaders/ShaderCommon.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <vector>
#include <memory>

/** @class ColliderDebugRenderer
 *  @brief Collider3DComponent の形状をワイヤーフレームで描画する
 *  @details
 *          - Box / Sphere / Capsule 形状を簡易ラインで可視化する
 *          - Transform のワールド行列を使用して配置・回転・スケールを反映する
 *          - RenderSystem のラスタライザステートを一時的にワイヤーフレームに切り替える
 */
class ColliderDebugRenderer : public Component, public IDrawable
{
public:
    /** @brief コンストラクタ
     *  @param _owner アタッチ先のゲームオブジェクト
     *  @param _active 有効 / 無効
     */
    ColliderDebugRenderer(GameObject* _owner, bool _active = true);

    /// @brief デストラクタ
    ~ColliderDebugRenderer() noexcept override;

    /// @brief 初期化処理
    void Initialize() override;

    /// @brief 描画処理
    void Draw() override;

    /// @brief 終了処理
    void Dispose() override;

private:
    /// @brief ボックス形状のラインを構築する
    void BuildBoxWire(const DX::Vector3& _halfExtent);

    /// @brief 球形状のラインを構築する（3リング簡易版）
    void BuildSphereWire(float _radius);

    /// @brief カプセル形状のラインを構築する（簡易版）
    void BuildCapsuleWire(float _radius, float _halfHeight);

    /// @brief 実際の描画処理（ラスタライザ切替済み前提）
    void DrawInternal(ID3D11DeviceContext* _context);

	/// @brief デバッグカラー用定数バッファ構造体
    struct DebugColor
    {
        DirectX::XMFLOAT4 color;
    };

private:
    Transform* transform;                               ///< 対象オブジェクトの Transform
    Camera3D* camera;                                   ///< 描画に使用するカメラ
    Framework::Physics::Collider3DComponent* collider;  ///< 参照対象のコライダー
	Framework::Physics::Rigidbody3D* rigidbody;         ///< 参照対象のリジッドボディ

    std::unique_ptr<VertexBuffer>     vertexBuffer;     ///< ライン用頂点バッファ
    std::vector<DX::Vector3>          linePoints;       ///< ローカル空間でのライン頂点列

    ShaderCommon::ShaderProgram* shaders;               ///< デバッグライン用シェーダ

    std::unique_ptr<DynamicConstantBuffer<DebugColor>> lineColorBuffer; ///< デバッグカラー用定数バッファ
};