/** @file   PhysicsLayers.cpp
 *  @brief  PhysicsLayers の実装
 *  @date   2025/11/17
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Physics/PhysicsLayers.h"

namespace Framework::Physics
{
    //-----------------------------------------------------------------------------
    // BPLayerInterfaceImpl
    //-----------------------------------------------------------------------------

    /// @brief コンストラクタ
    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
    {
        objectToBroadPhase[PhysicsLayer::Static]    = BroadPhaseLayerDef::Static;
        objectToBroadPhase[PhysicsLayer::Dynamic]   = BroadPhaseLayerDef::Dynamic;
        objectToBroadPhase[PhysicsLayer::Kinematic] = BroadPhaseLayerDef::Kinematic;
        objectToBroadPhase[PhysicsLayer::Ground]    = BroadPhaseLayerDef::Ground;
        objectToBroadPhase[PhysicsLayer::Player]    = BroadPhaseLayerDef::Player;
        objectToBroadPhase[PhysicsLayer::Enemy]     = BroadPhaseLayerDef::Enemy;
    }

    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return BroadPhaseLayerDef::NUM_LAYERS;
    }

    /// @brief ObjectLayer → BroadPhaseLayer 対応
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer _layer) const
    {
        return objectToBroadPhase[_layer];
    }

    /// @brief BroadPhaseLayer の名前を返す
    const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer _bpLayer) const
    {
        static constexpr std::array<const char*, BroadPhaseLayerDef::NUM_LAYERS> kNames = {
            "Static",
            "Dynamic",
            "Kinematic",
            "Ground",
            "Player",
            "Enemy",
        };

        const auto idx = static_cast<JPH::uint>(_bpLayer.GetValue());
        return idx < kNames.size() ? kNames[idx] : "Unknown";
    }

    //-----------------------------------------------------------------------------
    // ObjectVsBroadPhaseLayerFilterImpl
    //-----------------------------------------------------------------------------

    /// @brief ObjectLayer と BroadPhaseLayer の衝突可否
    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer _layer, JPH::BroadPhaseLayer _bpLayer) const
    {
        switch (_layer)
        {
        case PhysicsLayer::Static:
            // 静的は動的/キネマのみ候補（従来通り）
            return _bpLayer == BroadPhaseLayerDef::Dynamic || _bpLayer == BroadPhaseLayerDef::Kinematic
                || _bpLayer == BroadPhaseLayerDef::Player || _bpLayer == BroadPhaseLayerDef::Enemy
                || _bpLayer == BroadPhaseLayerDef::Ground;

        case PhysicsLayer::Dynamic:
        case PhysicsLayer::Kinematic:
        case PhysicsLayer::Ground:
        case PhysicsLayer::Player:
        case PhysicsLayer::Enemy:
            // 動く物/地面/プレイヤー/敵は全BroadPhase層と候補にする
            return true;

        default:
            return false;
        }
    }

    //-----------------------------------------------------------------------------
    // ObjectLayerPairFilterImpl
    //-----------------------------------------------------------------------------

    /// @brief ObjectLayer 同士の最終衝突可否
    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer _layer1, JPH::ObjectLayer _layer2) const
    {
        // Static 同士は衝突不要
        if (_layer1 == PhysicsLayer::Static && _layer2 == PhysicsLayer::Static)
        {
            return false;
        }

        return true; // その他は全て衝突する
    }

    //-----------------------------------------------------------------------------
    // ShapeCastBroadPhaseLayerFilter
    //-----------------------------------------------------------------------------

    /// @brief ObjectLayer と BroadPhaseLayer の衝突可否
    bool ShapeCastBroadPhaseLayerFilter::ShouldCollide(JPH::BroadPhaseLayer _bpLayer) const
    {
        return bpFilter->ShouldCollide(layer, _bpLayer);
    }

    //-----------------------------------------------------------------------------
    // ShapeCastObjectLayerFilter
    //-----------------------------------------------------------------------------

    /// @brief ObjectLayer 同士の最終衝突可否
    bool ShapeCastObjectLayerFilter::ShouldCollide(JPH::ObjectLayer _other) const
    {
        return pairFilter->ShouldCollide(layer, _other);
    }
}
