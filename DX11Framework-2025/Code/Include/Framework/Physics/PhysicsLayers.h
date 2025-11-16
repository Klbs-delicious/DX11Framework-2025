/** @file   PhysicsLayers.h
 *  @brief  Jolt のレイヤー管理と衝突フィルタを定義する
 *  @date   2025/11/17
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace Framework::Physics
{
    //-----------------------------------------------------------------------------
    // Object (ゲーム側) レイヤー
    //-----------------------------------------------------------------------------
    namespace PhysicsLayer
    {
        enum : JPH::ObjectLayer
        {
            Static = 0,   ///< 動かない物（床・壁など）
            Dynamic = 1,  ///< 動く物（プレイヤー・敵など）

            NUM_LAYERS
        };
    }

    //-----------------------------------------------------------------------------
    // BroadPhase (広域判定) レイヤー
    //-----------------------------------------------------------------------------
    namespace BroadPhaseLayerDef
    {
        static constexpr JPH::BroadPhaseLayer Static{ 0 };
        static constexpr JPH::BroadPhaseLayer Dynamic{ 1 };

        static constexpr JPH::uint NUM_LAYERS = 2;
    }

    /** @class  BPLayerInterfaceImpl
     *  @brief  ObjectLayer を BroadPhaseLayer に変換する
     */
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        /// @brief コンストラクタ
        BPLayerInterfaceImpl();

        /// @brief 利用可能な BroadPhaseLayer 数
        JPH::uint GetNumBroadPhaseLayers() const override;

        /// @brief ObjectLayer が属する BroadPhaseLayer を返す
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer _layer) const override;

    private:
        JPH::BroadPhaseLayer objectToBroadPhase[PhysicsLayer::NUM_LAYERS];
    };

    /** @class  ObjectVsBroadPhaseLayerFilterImpl
     *  @brief  ObjectLayer がどの BroadPhaseLayer と衝突するかを決定する
     */
    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        /// @brief 衝突すべきなら true
        bool ShouldCollide(JPH::ObjectLayer _layer, JPH::BroadPhaseLayer _bpLayer) const override;
    };

    /** @class  ObjectLayerPairFilterImpl
     *  @brief  ObjectLayer 同士の衝突可否を判定する
     */
    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        /// @brief 衝突すべきなら true
        bool ShouldCollide(JPH::ObjectLayer _layer1, JPH::ObjectLayer _layer2) const override;
    };
}
