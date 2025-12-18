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
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h> 
#include <Jolt/Physics/Collision/ObjectLayer.h>

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
            Kinematic = 2,///< 動くが物理影響を受けない物（移動プラットフォームなど）

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
        static constexpr JPH::BroadPhaseLayer Kinematic{ 2 };

        static constexpr JPH::uint NUM_LAYERS = 3;
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

        /// @brief BroadPhaseLayer の名称を返す
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer _bpLayer) const override;

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
    
    /** @class  ShapeCastBroadPhaseLayerFilter
     *  @brief  ShapeCast 時の BroadPhaseLayer フィルタ
     *  @details
     *      - 指定された ObjectLayer がどの BroadPhaseLayer と衝突するかを決定する
     */
    class ShapeCastBroadPhaseLayerFilter final : public JPH::BroadPhaseLayerFilter
    {
    public:
        ShapeCastBroadPhaseLayerFilter(
            const BPLayerInterfaceImpl* _bp,
            const ObjectVsBroadPhaseLayerFilterImpl* _filter,
            JPH::ObjectLayer _layer)
            : bpInterface(_bp)
            , bpFilter(_filter)
            , layer(_layer)
        {}

        bool ShouldCollide(JPH::BroadPhaseLayer bpLayer) const override
        {
            return bpFilter->ShouldCollide(layer, bpLayer);
        }

    private:
        const BPLayerInterfaceImpl* bpInterface;
        const ObjectVsBroadPhaseLayerFilterImpl* bpFilter;
        JPH::ObjectLayer layer;
    };

    /** @class  ShapeCastObjectLayerFilter
     *  @brief  ShapeCast 時の ObjectLayer フィルタ
     *  @details
     *      - 指定された ObjectLayer がどの ObjectLayer と衝突するかを決定する
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

        bool ShouldCollide(JPH::ObjectLayer other) const override
        {
            return pairFilter->ShouldCollide(layer, other);
        }

    private:
        const ObjectLayerPairFilterImpl* pairFilter;
        JPH::ObjectLayer layer;
    };

} // namespace Framework::Physics