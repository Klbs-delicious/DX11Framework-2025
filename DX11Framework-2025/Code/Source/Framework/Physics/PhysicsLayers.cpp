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
        objectToBroadPhase[PhysicsLayer::Ground]    = BroadPhaseLayerDef::Ground;   // 追加
        objectToBroadPhase[PhysicsLayer::Player]    = BroadPhaseLayerDef::Player;   // 追加
        objectToBroadPhase[PhysicsLayer::Enemy]     = BroadPhaseLayerDef::Enemy;    // 追加
    }

    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return BroadPhaseLayerDef::NUM_LAYERS;
    }

    /// @brief ObjectLayer → BroadPhaseLayer 対応
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer _layer) const
    {
        return this->objectToBroadPhase[_layer];
    }

    /// @brief BroadPhaseLayer の名前を返す
    const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer _bpLayer) const
    {
        switch (static_cast<JPH::uint>(_bpLayer.GetValue()))
        {
        case 0: return "Static";
        case 1: return "Dynamic";
        case 2: return "Kinematic";
        case 3: return "Ground";   // 追加
        case 4: return "Player";   // 追加
        case 5: return "Enemy";    // 追加
        default: return "Unknown";
        }
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
        if (_layer1 == PhysicsLayer::Static && _layer2 == PhysicsLayer::Static)
            return false;

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
