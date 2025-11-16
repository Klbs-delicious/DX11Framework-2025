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
        this->objectToBroadPhase[PhysicsLayer::Static] = BroadPhaseLayerDef::Static;
        this->objectToBroadPhase[PhysicsLayer::Dynamic] = BroadPhaseLayerDef::Dynamic;
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

    //-----------------------------------------------------------------------------
    // ObjectVsBroadPhaseLayerFilterImpl
    //-----------------------------------------------------------------------------

    /// @brief ObjectLayer と BroadPhaseLayer の衝突可否
    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer _layer, JPH::BroadPhaseLayer _bpLayer) const
    {
        switch (_layer)
        {
        case PhysicsLayer::Static:
            // 静的物体は Dynamic とだけ衝突
            return _bpLayer == BroadPhaseLayerDef::Dynamic;

        case PhysicsLayer::Dynamic:
            // 動的物体は両方と衝突
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

        // 他は全て衝突させる
        return true;
    }
}
