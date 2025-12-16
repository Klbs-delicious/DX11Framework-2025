/** @file   PhysicsContactListener.cpp
 *  @brief  Rigidbody間の接触イベントを受け取る Jolt ContactListenerを実装する
 *  @date   2025/12/05
 */

#include "Include/Framework/Physics/PhysicsContactListener.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>

#include <iostream>

using namespace JPH;

namespace Framework::Physics
{
    void PhysicsContactListener::OnContactAdded(const Body& _bodyA,
        const Body& _bodyB,
        const ContactManifold& _manifold,
        ContactSettings& _settings)
    {
		// 接触しているサブシェイプのインデックスを取得する
        uint32 indexA = _manifold.mSubShapeID1.GetValue();
        uint32 indexB = _manifold.mSubShapeID2.GetValue();

        // 接触ペアを物理システムに登録する
        this->physicsSystem.AddContactPair(
            { _bodyA.GetID(), indexA },
            { _bodyB.GetID(), indexB }
        );
    }

    void PhysicsContactListener::OnContactPersisted(const Body& _bodyA,
        const Body& _bodyB,
        const ContactManifold& _manifold,
        ContactSettings& _settings)
    {
        // 毎フレーム触れている接触も登録する（Stay 判定用）
        uint32 indexA = _manifold.mSubShapeID1.GetValue();
        uint32 indexB = _manifold.mSubShapeID2.GetValue();

        this->physicsSystem.AddContactPair(
            { _bodyA.GetID(), indexA },
            { _bodyB.GetID(), indexB }
        );
    }

    void PhysicsContactListener::OnContactRemoved(const SubShapeIDPair& _pair)
    {
        // Exit は PhysicsSystem::ProcessContactEvents で prev と curr の差分から判定するため、
        // ここでは明示的な処理は不要。
        // 必要なら _pair.mBody1ID / mBody2ID と _pair.mSubShapeID1 / mSubShapeID2 を使った即時通知も可能。
    }
}