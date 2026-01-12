#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>

namespace Framework::Physics
{
	class PhysicsSystem;

    /** @class  PhysicsContactListener
     *  @brief  Rigidbody間の接触イベントを受け取る
	 */
    class PhysicsContactListener : public JPH::ContactListener
    {
    public:
        PhysicsContactListener(PhysicsSystem& _physicsSystem) : physicsSystem(_physicsSystem) {}

		/** @brief  ぶつかった瞬間に呼ばれるコールバック
         *  @param _bodyA       ぶつかった剛体A
         *  @param _bodyB       ぶつかった剛体B
         *  @param _manifold    接触マニフォールド
         *  @param _settings    接触設定
         */
        void OnContactAdded(const JPH::Body& _bodyA,
            const JPH::Body& _bodyB,
            const JPH::ContactManifold& _manifold,
            JPH::ContactSettings& _settings) override;

		/** @brief  毎フレーム触れている状態で呼ばれるコールバック
         *  @param _bodyA       ぶつかった剛体A
         *  @param _bodyB       ぶつかった剛体B
         *  @param _manifold    接触マニフォールド
         *  @param _settings    接触設定
         */
        void OnContactPersisted(const JPH::Body& _bodyA,
            const JPH::Body& _bodyB,
            const JPH::ContactManifold& _manifold,
            JPH::ContactSettings& _settings) override;

		/** @brief 触れなくなった瞬間に呼ばれるコールバック
		 *  @param _subshapePair 触れなくなったペア
         */
        void OnContactRemoved(const JPH::SubShapeIDPair& _subshapePair) override;

    private:
		PhysicsSystem& physicsSystem;   ///< 物理システム
    };
}