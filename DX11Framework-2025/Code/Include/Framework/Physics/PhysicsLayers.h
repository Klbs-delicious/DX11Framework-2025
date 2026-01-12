/** @file   PhysicsLayers.h
 *  @brief  Jolt のレイヤー管理と衝突フィルタを定義する
 *  @date   2025/11/17
 */
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <array>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>

namespace Framework::Physics
{
    //-----------------------------------------------------------------------------
    // Object (ゲーム側) レイヤー
    //-----------------------------------------------------------------------------
    namespace PhysicsLayer
    {
        enum : JPH::ObjectLayer
        {
            Static    = 0,  ///< 動かない物（床・壁など）
            Dynamic   = 1,  ///< 動く物（プレイヤー・敵など）
            Kinematic = 2,  ///< 動くが物理影響を受けない物（移動プラットフォームなど）
            Ground    = 3,  ///< 地面
            Player    = 4,  ///< プレイヤー
            Enemy     = 5,  ///< 敵

            NUM_LAYERS
        };
    }

    //-----------------------------------------------------------------------------
    // BroadPhase (広域判定) レイヤー
    //  NOTE: 現状は ObjectLayer と 1:1 で対応させる
    //-----------------------------------------------------------------------------
    namespace BroadPhaseLayerDef
    {
        static constexpr JPH::BroadPhaseLayer Static    { PhysicsLayer::Static };
        static constexpr JPH::BroadPhaseLayer Dynamic   { PhysicsLayer::Dynamic };
        static constexpr JPH::BroadPhaseLayer Kinematic { PhysicsLayer::Kinematic };
        static constexpr JPH::BroadPhaseLayer Ground    { PhysicsLayer::Ground };
        static constexpr JPH::BroadPhaseLayer Player    { PhysicsLayer::Player };
        static constexpr JPH::BroadPhaseLayer Enemy     { PhysicsLayer::Enemy };

        static constexpr JPH::uint NUM_LAYERS = static_cast<JPH::uint>(PhysicsLayer::NUM_LAYERS);
    }

    /** @class  BPLayerInterfaceImpl
     *  @brief  ObjectLayer を BroadPhaseLayer に変換する
     */
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl();

        JPH::uint GetNumBroadPhaseLayers() const override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer _layer) const override;
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer _bpLayer) const override;

    private:
        std::array<JPH::BroadPhaseLayer, PhysicsLayer::NUM_LAYERS> objectToBroadPhase{};
    };

    /** @class  ObjectVsBroadPhaseLayerFilterImpl
     *  @brief  ObjectLayer がどの BroadPhaseLayer と衝突するかを決定する
     */
    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer _layer, JPH::BroadPhaseLayer _bpLayer) const override;
    };

    /** @class  ObjectLayerPairFilterImpl
     *  @brief  ObjectLayer 同士の衝突可否を判定する
     */
    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer _layer1, JPH::ObjectLayer _layer2) const override;
    };

    //-----------------------------------------------------------------------------
    // ShapeCast 用フィルタ
    //  NOTE: 既存のレイヤーフィルタ実装に委譲する
    //-----------------------------------------------------------------------------

    /** @class  ShapeCastBroadPhaseLayerFilter
     *  @brief  ShapeCast 時の BroadPhaseLayer フィルタ
     */
    class ShapeCastBroadPhaseLayerFilter final : public JPH::BroadPhaseLayerFilter
    {
    public:
        ShapeCastBroadPhaseLayerFilter(
            const BPLayerInterfaceImpl* /*_bp*/, // 互換のため受け取る（現実装では未使用）
            const ObjectVsBroadPhaseLayerFilterImpl* _filter,
            JPH::ObjectLayer _layer)
            : bpFilter(_filter)
            , layer(_layer)
        {}

        bool ShouldCollide(JPH::BroadPhaseLayer _bpLayer) const override;

    private:
        const ObjectVsBroadPhaseLayerFilterImpl* bpFilter;
        JPH::ObjectLayer layer;
    };

    /** @class  ShapeCastObjectLayerFilter
     *  @brief  ShapeCast 時の ObjectLayer フィルタ
     */
    class ShapeCastObjectLayerFilter final : public JPH::ObjectLayerFilter
    {
    public:
        ShapeCastObjectLayerFilter(
            const ObjectLayerPairFilterImpl* _pairFilter,
            JPH::ObjectLayer _layer)
            : pairFilter(_pairFilter)
            , layer(_layer)
        {}

        bool ShouldCollide(JPH::ObjectLayer _other) const override;

    private:
        const ObjectLayerPairFilterImpl* pairFilter;
        JPH::ObjectLayer layer;
    };

} // namespace Framework::Physics