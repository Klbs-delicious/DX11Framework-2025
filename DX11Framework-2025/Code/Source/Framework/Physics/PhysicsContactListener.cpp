/** @file   PhysicsContactListener.cpp
 *  @brief  Rigidbody間の接触イベントを受け取る Jolt ContactListenerを実装する
 *  @date   2025/12/05
 */

#include "Include/Framework/Physics/PhysicsContactListener.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/Body/Body.h>
#include <iostream>

using namespace JPH;

namespace Framework::Physics
{
    void PhysicsContactListener::OnContactAdded(const Body& _bodyA,
        const Body& _bodyB,
        const ContactManifold& _manifold,
        ContactSettings& _settings)
    {
		// 接触ペアを物理システムに登録する
        this->physicsSystem.AddContactPair(_bodyA.GetID(), _bodyB.GetID());
    }

    void PhysicsContactListener::OnContactPersisted(const Body& _bodyA,
        const Body& _bodyB,
        const ContactManifold& _manifold,
        ContactSettings& _settings)
    {
        // 接触ペアを物理システムに登録する
        this->physicsSystem.AddContactPair(_bodyA.GetID(), _bodyB.GetID());
    }

    void PhysicsContactListener::OnContactRemoved(const SubShapeIDPair& _pair)
    {
		// 現状は特に何もしない
    }
}