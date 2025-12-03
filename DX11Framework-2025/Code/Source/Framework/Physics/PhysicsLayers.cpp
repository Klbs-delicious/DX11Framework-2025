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
        objectToBroadPhase[PhysicsLayer::Static] = BroadPhaseLayerDef::Static;
        objectToBroadPhase[PhysicsLayer::Dynamic] = BroadPhaseLayerDef::Dynamic;
        objectToBroadPhase[PhysicsLayer::Kinematic] = BroadPhaseLayerDef::Kinematic;
    }

    /// @brief 利用可能なレイヤー数
    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return BroadPhaseLayerDef::NUM_LAYERS;
    }

    /// @brief ObjectLayer → BroadPhaseLayer 対応
    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer _layer) const
    {
        return this->objectToBroadPhase[_layer];
    }

    /// @brief BroadPhaseLayer の名前を返す（←コレが必須）
    const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer _bpLayer) const
    {
        switch (static_cast<JPH::uint>(_bpLayer.GetValue()))
        {
        case 0: return "Static";
        case 1: return "Dynamic";
        case 2: return "Kinematic";
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
            return _bpLayer == BroadPhaseLayerDef::Dynamic || _bpLayer == BroadPhaseLayerDef::Kinematic;

        case PhysicsLayer::Dynamic:
            return true; // すべてと衝突する

        case PhysicsLayer::Kinematic:
            return true; // すべてと衝突する

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

        //if (_layer1 == PhysicsLayer::Kinematic && _layer2 == PhysicsLayer::Kinematic)
        //    return false;

        return true; // その他は全て衝突する
    }
}
