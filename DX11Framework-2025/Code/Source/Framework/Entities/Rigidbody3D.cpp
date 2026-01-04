/** @file   Rigidbody3D.cpp
 *  @brief  TimeScale 対応：自前移動＋Jolt 押し戻し用 Rigidbody3D 実装
 *  @date   2025/11/28
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Collider3DComponent.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/RayCast.h>

#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <iostream>

namespace Framework::Physics
{
	using namespace JPH;

	//-----------------------------------------------------------------------------
	// 自身のトリガーを無視する ShapeFilter
	//-----------------------------------------------------------------------------
	class SelfTriggerShapeFilter final : public JPH::ShapeFilter
	{
	public:
		SelfTriggerShapeFilter(PhysicsSystem& _phys, JPH::BodyID _id)
			: mPhys(&_phys), mBody(_id) {}

		bool ShouldCollide(const JPH::Shape* /*_shape*/, const JPH::SubShapeID& _subShapeID) const override
		{
			auto* collider = mPhys->GetCollider3D(mBody, _subShapeID.GetValue());
			return !(collider && collider->IsTrigger());
		}

	private:
		PhysicsSystem* mPhys;
		JPH::BodyID mBody;
	};

	//-----------------------------------------------------------------------------
	// 定数
	//-----------------------------------------------------------------------------
	static constexpr float parallelEps = 1.0e-4f;	///< 法線と移動方向がほぼ平行かどうかの判定用（今は未使用）
	static constexpr float skinWidth = 1.0e-3f;		///< 壁から少しだけ離すためのスキン幅

	//-----------------------------------------------------------------------------
	// Constructor / Destructor
	//-----------------------------------------------------------------------------
	Rigidbody3D::Rigidbody3D(GameObject* _owner, bool _active)
		: Component(_owner, _active)
		, bodyID()
		, hasBody(false)
		, motionType(EMotionType::Kinematic)
		, objectLayer(PhysicsLayer::Kinematic)
		, staged(nullptr)
		, stagedPrev(nullptr)
		, visualTransform(nullptr)
		, physicsSystem(SystemLocator::Get<PhysicsSystem>())
		, colliders()
		, linearVelocity(DX::Vector3::Zero)
		, gravity(0.0f, -9.8f, 0.0f)
		, useGravity(false)
		, isGrounded(false)
	{
	}

	Rigidbody3D::~Rigidbody3D() noexcept
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// Initialize / Dispose
	//-----------------------------------------------------------------------------
	void Rigidbody3D::Initialize()
	{
		this->visualTransform = this->Owner()->GetComponent<Transform>();

		this->colliders = this->Owner()->GetComponentsInChildren<Collider3DComponent>();

		this->staged = std::make_unique<StagedTransform>();
		this->stagedPrev = std::make_unique<StagedTransform>();

		if (this->visualTransform)
		{
			this->staged->position = this->visualTransform->GetWorldPosition();
			this->staged->rotation = this->visualTransform->GetWorldRotation();
			this->staged->scale = this->visualTransform->GetWorldScale();
		}
		else
		{
			this->staged->position = DX::Vector3::Zero;
			this->staged->rotation = DX::Quaternion::Identity;
			this->staged->scale = DX::Vector3::One;
		}

		*(this->stagedPrev) = *(this->staged);

		this->InitializeBody();
	}

	void Rigidbody3D::Dispose()
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// 物理シミュレーションステップ
	//-----------------------------------------------------------------------------
	void Rigidbody3D::StepPhysics(float _deltaTime)
	{
		// 物理更新は時間スケールを適用させ、自前の押し戻し結果を visualTransform に反映させる
		float scaledDelta = this->Owner()->TimeScale()->ApplyTimeScale(_deltaTime);
		this->UpdateLogical(scaledDelta);

		for (size_t i = 0; i < Rigidbody3D::SolveIterations; i++)
		{
			// 貫通解決を行う（既に接地している場合完全に押し戻す）
			// 現状は順番依存で解決される
			this->ResolvePenetration();
		}

		// 衝突解決（CastShape 押し戻し）
		this->ResolveCastShape(_deltaTime);

		// visual に反映させる
		this->SyncToVisual();
	}

	//-----------------------------------------------------------------------------
	// 自前移動（TimeScale 適用済み）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::UpdateLogical(float _deltaTime)
	{
		if (!this->staged) { return; }
		if (_deltaTime <= 0.0f) { return; }

		*(this->stagedPrev) = *(this->staged);

		if (this->useGravity)
		{
			if (this->isGrounded)
			{
				this->linearVelocity.y = 0.0f;
			}
			else
			{
				this->linearVelocity += this->gravity * _deltaTime;
			}
		}

		this->staged->position += this->linearVelocity * _deltaTime;
	}

	//-----------------------------------------------------------------------------
	// staged → visual
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncToVisual() const
	{
		if (!this->visualTransform || !this->staged) { return; }

		this->visualTransform->SetWorldPosition(this->staged->position);
		this->visualTransform->SetWorldRotation(this->staged->rotation);
		this->visualTransform->SetWorldScale(this->staged->scale);
	}

	//-----------------------------------------------------------------------------
	// visual → Jolt（Kinematic のみ）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncVisualToJolt(float _deltaTime)
	{
		if (!this->hasBody || !this->visualTransform) { return; }
		if (this->motionType != EMotionType::Kinematic) { return; }
		if (this->colliders.empty()) { return; }

		const DX::Vector3 pivot = this->staged->position;
		const DX::Quaternion rot = this->staged->rotation;

		DX::Vector3 combinedOffset = this->ComputeCombinedOffset(rot);
		DX::Vector3 comPos = pivot + combinedOffset;

		this->physicsSystem.GetBodyInterface().MoveKinematic(
			this->bodyID,
			RVec3(comPos.x, comPos.y, comPos.z),
			Quat(rot.x, rot.y, rot.z, rot.w),
			_deltaTime
		);
	}

	//-----------------------------------------------------------------------------
	// Jolt → visual → staged（押し戻し結果を確定）※今の方針では基本未使用
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncJoltToVisual()
	{
		if (!this->hasBody || !this->visualTransform || !this->staged) { return; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return; }

		const Body& body = lock.GetBody();

		DX::Vector3 comPos(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);

		DX::Quaternion rot(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);

		DX::Vector3 combinedOffset = this->ComputeCombinedOffset(rot);
		DX::Vector3 pivotPos = comPos - combinedOffset;

		this->visualTransform->SetWorldPosition(pivotPos);
		this->visualTransform->SetWorldRotation(rot);

		this->staged->position = pivotPos;
		this->staged->rotation = rot;
	}

	//-----------------------------------------------------------------------------
	// 複数コライダーの centerOffset を回転適用して平均を計算
	//-----------------------------------------------------------------------------
	DX::Vector3 Rigidbody3D::ComputeCombinedOffset(const DX::Quaternion& _rot) const
	{
		if (this->colliders.empty())
		{
			return DX::Vector3::Zero;
		}

		DX::Vector3 sum = DX::Vector3::Zero;

		for (auto& col : this->colliders)
		{
			const DX::Vector3 local = col->GetCenterOffset();
			sum += DX::Vector3::Transform(local, _rot);
		}

		return sum / static_cast<float>(this->colliders.size());
	}

	//-----------------------------------------------------------------------------
	// 貫通解決（最も深くめり込んでいるBody1つを押し出す）
	//   注意: BodyLockRead は shape を取るだけにして、NarrowPhaseQuery はロック外で使う
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ResolvePenetration()
	{
		if (!this->hasBody || !this->staged) { return; }
		if (this->colliders.empty()) { return; }

		const DX::Quaternion rot = this->staged->rotation;

		DX::Vector3 combinedOffset = this->ComputeCombinedOffset(rot);
		DX::Vector3 comPos = this->staged->position + combinedOffset;

		const Shape* shape = nullptr;
		{
			BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
			if (!lock.Succeeded()) { return; }

			shape = lock.GetBody().GetShape();
		} // lock はここで解除される

		if (!shape) { return; }

		RMat44 transform = RMat44::sRotationTranslation(
			Quat(rot.x, rot.y, rot.z, rot.w),
			RVec3(comPos.x, comPos.y, comPos.z)
		);

		CollideShapeSettings settings;
		settings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;
		settings.mMaxSeparationDistance = 0.0f;

		ClosestHitCollisionCollector<CollideShapeCollector> collector;
		const auto& npq = this->physicsSystem.GetNarrowPhaseQuery();
		const auto& broad = this->physicsSystem.GetBroadPhaseLayerFilter(this->objectLayer);
		const auto& obj = this->physicsSystem.GetObjectLayerFilter(this->objectLayer);
		IgnoreSelfBodyFilter bodyFilter(this->bodyID);

		//// 衝突判定を行うかどうかのフィルタ設定
		//// Triggerになっている場合には衝突判定を行わない
		SelfTriggerShapeFilter selfFilter(this->physicsSystem, this->bodyID);

		npq.CollideShape(
			shape,
			Vec3::sOne(),
			transform,
			settings,
			RVec3::sZero(),
			collector,
			broad,
			obj,
			bodyFilter,
			selfFilter
		);
		if (!collector.HadHit()) { return; }

		const auto& hit = collector.mHit;
		if (hit.mPenetrationDepth <= 0.0f) { return; }

		// まず自分側サブシェイプがトリガーなら押し戻しをしない
		{
			auto* selfCol = this->physicsSystem.GetCollider3D(this->bodyID, hit.mSubShapeID1.GetValue());
			if (selfCol && selfCol->IsTrigger()) { return; }
		}

		// ヒットした相手がトリガーなら無視する
		auto other = this->physicsSystem.GetCollider3D(hit.mBodyID2, hit.mSubShapeID2.GetValue());
		if (!other || other->IsTrigger()) { return; }

		Vec3 axis = hit.mPenetrationAxis.Normalized();

		// 速度の法線成分を除去して再貫通を抑制
		DX::Vector3 n(axis.GetX(), axis.GetY(), axis.GetZ());
		float vn = this->linearVelocity.x * n.x + this->linearVelocity.y * n.y + this->linearVelocity.z * n.z;
		if (vn > 0.0f)
		{
			this->linearVelocity -= n * vn;
		}

		DX::Vector3 depen(
			-axis.GetX() * hit.mPenetrationDepth,
			-axis.GetY() * hit.mPenetrationDepth,
			-axis.GetZ() * hit.mPenetrationDepth
		);
		DX::Vector3 newCom = comPos + depen;

		this->staged->position = newCom - combinedOffset;
	}

	//-----------------------------------------------------------------------------
	// CastShape 押し戻し解決（移動方向に対するヒットを使って押し戻す）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ResolveCastShape(float _deltaTime)
	{
		if (!this->hasBody || !this->staged || !this->stagedPrev) { return; }
		if (this->colliders.empty()) { return; }

		this->isGrounded = false;

		DX::Vector3 move = this->linearVelocity * _deltaTime;
		if (move.LengthSquared() <= 0.0f) { return; }

		float moveLen = move.Length();
		DX::Vector3 moveDir = move / moveLen;

		DX::Quaternion rot = this->staged->rotation;

		DX::Vector3 combinedOffset = this->ComputeCombinedOffset(rot);
		DX::Vector3 comStart = this->stagedPrev->position + combinedOffset;

		const Shape* shape = nullptr;
		{
			BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
			if (!lock.Succeeded()) { return; }

			shape = lock.GetBody().GetShape();
		} // lock はここで解除する

		if (!shape) { return; }

		RShapeCast cast(
			shape,
			Vec3::sOne(),
			RMat44::sRotationTranslation(
				Quat(rot.x, rot.y, rot.z, rot.w),
				RVec3(comStart.x, comStart.y, comStart.z)
			),
			Vec3(move.x, move.y, move.z)
		);

		ShapeCastSettings settings;
		settings.mReturnDeepestPoint = false;
		settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
		settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;

		ClosestShapeCastCollector col;
		const auto& npq = this->physicsSystem.GetNarrowPhaseQuery();
		const auto& broad = this->physicsSystem.GetBroadPhaseLayerFilter(this->objectLayer);
		const auto& obj = this->physicsSystem.GetObjectLayerFilter(this->objectLayer);

		//// 衝突判定を行うかどうかのフィルタ設定
		//// Triggerになっている場合には衝突判定を行わない
		SelfTriggerShapeFilter selfFilter(this->physicsSystem, this->bodyID);

		npq.CastShape(
			cast,
			settings,
			RVec3::sZero(),
			col,
			broad,
			obj,
			IgnoreSelfBodyFilter(this->bodyID),
			selfFilter
		);
		if (!col.hasHit) { return; }

		// まず自分側サブシェイプがトリガーなら押し戻しをしない
		{
			auto* selfCol = this->physicsSystem.GetCollider3D(this->bodyID, col.hit.mSubShapeID1.GetValue());
			if (selfCol && selfCol->IsTrigger()) { return; }
		}

		// ヒットした相手がトリガーなら無視する
		auto other = this->physicsSystem.GetCollider3D(col.hit.mBodyID2, col.hit.mSubShapeID2.GetValue());
		if (!other || other->IsTrigger()) { return; }

		float f = std::clamp(col.hit.mFraction, 0.0f, 1.0f);
		float adv = std::max(0.0f, moveLen * f - skinWidth);

		DX::Vector3 newCom = comStart + moveDir * adv;

		DX::Vector3 normal(
			col.hit.mPenetrationAxis.GetX(),
			col.hit.mPenetrationAxis.GetY(),
			col.hit.mPenetrationAxis.GetZ()
		);
		normal.Normalize();
		if (adv > 0.0f)
		{
			newCom += normal * skinWidth;
		}

		this->staged->position = newCom - combinedOffset;

		// 法線成分の速度を削減（面に沿わせる）
		float vn = this->linearVelocity.x * normal.x + this->linearVelocity.y * normal.y + this->linearVelocity.z * normal.z;
		if (vn > 0.0f)
		{
			this->linearVelocity -= normal * vn;
		}

		this->CheckGrounded();
	}

	//-----------------------------------------------------------------------------
	// 接地判定（足元に短いRayを落とすだけなので BodyLock は不要）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::CheckGrounded()
	{
		this->isGrounded = false;

		if (!this->hasBody || !this->staged) { return; }
		if (this->colliders.empty()) { return; }

		float minBottomOffset = FLT_MAX;
		const DX::Quaternion rot = this->staged->rotation;

		for (auto& col : this->colliders)
		{
			DX::Vector3 offset = DX::Vector3::Transform(col->GetCenterOffset(), rot);
			DX::Vector3 scale = this->staged->scale;

			float bottom = 0.0f;

			switch (col->GetShapeType())
			{
			case ColliderShapeType::Capsule:
			{
				float r = col->GetCapsuleRadius() * std::max(scale.x, std::max(scale.y, scale.z));
				float h = col->GetCapsuleHalfHeight() * scale.y;
				bottom = offset.y - (h + r);
				break;
			}
			case ColliderShapeType::Sphere:
			{
				float r = col->GetSphereRadius() * std::max(scale.x, std::max(scale.y, scale.z));
				bottom = offset.y - r;
				break;
			}
			case ColliderShapeType::Box:
			{
				DX::Vector3 ext = col->GetBoxHalfExtent() * scale;
				bottom = offset.y - ext.y;
				break;
			}
			default:
				bottom = offset.y;
				break;
			}

			minBottomOffset = std::min(minBottomOffset, bottom);
		}

		const float extra = 0.05f;
		DX::Vector3 pivotPos = this->staged->position;

		RVec3 from(pivotPos.x, pivotPos.y + minBottomOffset, pivotPos.z);
		RVec3 to(pivotPos.x, pivotPos.y + minBottomOffset - (0.1f + extra), pivotPos.z);

		RRayCast ray(from, to - from);
		RayCastResult hit;

		if (this->physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit))
		{
			this->isGrounded = true;
		}
	}

	//-----------------------------------------------------------------------------
	// ロジック座標アクセス
	//-----------------------------------------------------------------------------
	DX::Vector3 Rigidbody3D::GetLogicalPosition() const
	{
		if (!this->staged)
		{
			return DX::Vector3::Zero;
		}
		return this->staged->position;
	}

	DX::Quaternion Rigidbody3D::GetLogicalRotation() const
	{
		if (!this->staged)
		{
			return DX::Quaternion::Identity;
		}
		return this->staged->rotation;
	}

	void Rigidbody3D::SetLogicalPosition(const DX::Vector3& _worldPos)
	{
		if (!this->staged) { return; }
		this->staged->position = _worldPos;
	}

	void Rigidbody3D::SetLogicalRotation(const DX::Quaternion& _worldRot)
	{
		if (!this->staged) { return; }
		this->staged->rotation = _worldRot;
	}

	void Rigidbody3D::TranslateWorld(const DX::Vector3& _delta)
	{
		if (!this->staged) { return; }
		this->staged->position += _delta;
	}

	void Rigidbody3D::TranslateLocal(const DX::Vector3& _delta)
	{
		if (!this->staged) { return; }

		const DX::Quaternion& worldRot = this->staged->rotation;

		const DX::Vector3 forward = DX::Vector3::Transform(DX::Vector3::Forward, worldRot);
		const DX::Vector3 right = DX::Vector3::Transform(DX::Vector3::Right, worldRot);
		const DX::Vector3 up = DX::Vector3::Transform(DX::Vector3::Up, worldRot);

		const DX::Vector3 worldDelta =
			right * _delta.x +
			up * _delta.y +
			forward * _delta.z;

		this->staged->position += worldDelta;
	}

	//-----------------------------------------------------------------------------
	// 線形速度
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SetLinearVelocity(const DX::Vector3& _velocity)
	{
		this->linearVelocity = _velocity;
	}

	void Rigidbody3D::AddLinearVelocity(const DX::Vector3& _deltaVelocity)
	{
		this->linearVelocity += _deltaVelocity;
	}

	DX::Vector3 Rigidbody3D::GetLinearVelocity() const
	{
		return this->linearVelocity;
	}

	//-----------------------------------------------------------------------------
	// MotionType / ObjectLayer 設定
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SetMotionTypeStatic()
	{
		this->motionType = EMotionType::Static;
		this->ApplyMotionTypeToBody();
	}

	void Rigidbody3D::SetMotionTypeKinematic()
	{
		this->motionType = EMotionType::Kinematic;
		this->ApplyMotionTypeToBody();
	}

	void Rigidbody3D::SetObjectLayerStatic()
	{
		this->objectLayer = PhysicsLayer::Static;
		this->ApplyObjectLayerToBody();
	}

	void Rigidbody3D::SetObjectLayerKinematic()
	{
		this->objectLayer = PhysicsLayer::Kinematic;
		this->ApplyObjectLayerToBody();
	}

	bool Rigidbody3D::GetBodyTransform(DX::Vector3& _outPos, DX::Quaternion& _outRot) const
	{
		if (!this->hasBody) { return false; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return false; }

		const Body& body = lock.GetBody();
		_outPos = DX::Vector3(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);
		_outRot = DX::Quaternion(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);
		return true;
	}

	//-----------------------------------------------------------------------------
	// 衝突イベント
	//-----------------------------------------------------------------------------
	void Rigidbody3D::DispatchContactEvent(const ContactType& _type, Collider3DComponent* _selfCollider, Collider3DComponent* _otherColl)
	{
		auto owner = this->Owner();
		if (!owner) { return; }
		if (_selfCollider == nullptr || _otherColl == nullptr){ return; }

		for (auto& component : owner->GetComponents())
		{
			auto listener = dynamic_cast<BaseColliderDispatcher3D*>(component.get());
			if (!listener) { continue; }

			switch (_type)
			{
			case Framework::Physics::ContactType::Trigger_Entered:
				listener->OnTriggerEnter(_selfCollider, _otherColl);
				break;

			case Framework::Physics::ContactType::Trigger_Stayed:
				listener->OnTriggerStay(_selfCollider, _otherColl);
				break;

			case Framework::Physics::ContactType::Trigger_Exited:
				listener->OnTriggerExit(_selfCollider, _otherColl);
				break;

			case Framework::Physics::ContactType::Coll_Entered:
				listener->OnCollisionEnter(_selfCollider, _otherColl);
				break;

			case Framework::Physics::ContactType::Coll_Stayed:
				listener->OnCollisionStay(_selfCollider, _otherColl);
				break;

			case Framework::Physics::ContactType::Coll_Exited:
				listener->OnCollisionExit(_selfCollider, _otherColl);
				break;

			default:
				break;
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Body セットアップ
	//-----------------------------------------------------------------------------
	void Rigidbody3D::GetInitialTransform(DX::Vector3& _pos, DX::Quaternion& _rot) const
	{
		if (!this->visualTransform)
		{
			_pos = DX::Vector3::Zero;
			_rot = DX::Quaternion::Identity;
			return;
		}

		_rot = this->visualTransform->GetWorldRotation();
		const DX::Vector3 pivotPos = this->visualTransform->GetWorldPosition();

		DX::Vector3 combinedOffset = this->ComputeCombinedOffset(_rot);

		_pos = pivotPos + combinedOffset;
	}

	void Rigidbody3D::SetupBodySettings(BodyCreationSettings& _settings) const
	{
		DX::Vector3 initialPos;
		DX::Quaternion initialRot;
		this->GetInitialTransform(initialPos, initialRot);

		_settings.mPosition = RVec3(initialPos.x, initialPos.y, initialPos.z);
		_settings.mRotation = Quat(initialRot.x, initialRot.y, initialRot.z, initialRot.w);

		_settings.mMotionType = this->motionType;
		_settings.mMotionQuality =
			(this->motionType == EMotionType::Kinematic)
			? EMotionQuality::LinearCast
			: EMotionQuality::Discrete;

		_settings.mObjectLayer = this->objectLayer;
		_settings.mAllowSleeping = false;

		bool isSensor = false;
		for (auto& col : this->colliders)
		{
			if (col->IsTrigger())
			{
				isSensor = true;
				break;
			}
		}
		_settings.mIsSensor = isSensor;

		if (this->motionType == EMotionType::Kinematic)
		{
			_settings.mCollideKinematicVsNonDynamic = true;
		}
	}

	void Rigidbody3D::ApplyMotionTypeToBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetMotionType(
			this->bodyID,
			this->motionType,
			EActivation::Activate
		);

		bodyInterface.SetMotionQuality(
			this->bodyID,
			(this->motionType == EMotionType::Kinematic)
			? EMotionQuality::LinearCast
			: EMotionQuality::Discrete
		);
	}

	void Rigidbody3D::ApplyObjectLayerToBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetObjectLayer(this->bodyID, this->objectLayer);
	}

	//-----------------------------------------------------------------------------
	// Body create / destroy
	//-----------------------------------------------------------------------------
	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody) { return; }
		if (this->colliders.empty())
		{
			std::cout << "[Rigidbody3D] Skip InitializeBody because no colliders are attached\n";
			return;
		}

		// Collider3DComponent が持つ Shape を集めて CompoundShape を作成する
		for (auto& coll : this->colliders)
		{
			if (!coll->GetShape())
			{
				coll->BuildShapeSettings();
				coll->CreateShape();
			}
		}

		//=======================================================
		// MutableCompoundShapeSettings に追加
		//=======================================================
		Ref<MutableCompoundShapeSettings> compoundSettings = new MutableCompoundShapeSettings();

		// collider の順番 = subShapeIndex として扱う
		const size_t colliderCount = this->colliders.size();

		for (size_t i = 0; i < colliderCount; i++)
		{
			Collider3DComponent* coll = this->colliders[i];
			if (!coll || !coll->GetShape())
			{
				std::cout << "[Rigidbody3D] shape is NULL in one of colliders\n";
				return;
			}

			DX::Vector3 offset = coll->GetCenterOffset();

			compoundSettings->AddShape(
				Vec3(offset.x, offset.y, offset.z),
				Quat::sIdentity(),
				coll->GetShape()
			);
		}

		//=======================================================
		// CompoundShape を作成する
		//=======================================================
		ShapeRefC compoundShape = compoundSettings->Create().Get();

		//=======================================================
		// Body を作成しワールドに追加する
		//=======================================================
		auto& bodyInterface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodySettings(settings);
		settings.SetShape(compoundShape);

		Body* body = bodyInterface.CreateBody(settings);
		if (!body) { return; }

		bodyInterface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;

		//=======================================================
		// Rigidbody → system 登録を行う
		//=======================================================
		this->physicsSystem.RegisterRigidbody3D(this->bodyID, this);

		//=======================================================
		// SubShapeIndex（= collider の順番）を system に登録する
		//=======================================================
		// CompoundShape の実体取得（created shape は CompoundShape に変換されている）
        if (compoundShape->GetType() != EShapeType::Compound)
        {
            // 単一形状（Compound 以外）の場合は SubShape を持たないため、
            // collider の順番による登録はスキップする
            std::cout << "[Rigidbody3D] Created shape is not Compound (type=" << (int)compoundShape->GetType() << ")\n";

            // ただし、単一形状でも衝突イベントで SubShapeID は 0 が使われるため、
            // 最初のコライダーを SubShapeID=0 として登録しておく
            if (!this->colliders.empty())
            {
                Collider3DComponent* first = this->colliders[0];
                if (first)
                {
                    this->physicsSystem.RegisterCollider3D(this->bodyID, 0, first);
                }
            }
        }
        else
        {
            const CompoundShape* comp = static_cast<const CompoundShape*>(compoundShape.GetPtr());
            if (!comp)
            {
                std::cout << "[Rigidbody3D] Failed to get CompoundShape pointer\n";
                return;
            }

            const uint32 subShapeCount = comp->GetNumSubShapes();

            for (uint32 index = 0; index < subShapeCount; ++index)
            {
                // index = SubShapeID の値として扱う（Compound は連番を割り当てる想定）
                JPH::SubShapeID::Type key = static_cast<JPH::SubShapeID::Type>(index);

                Collider3DComponent* collider = this->colliders[index];
                if (!collider)
                {
                    std::cout << "[Rigidbody3D] Collider is null at index " << index << "\n";
                    continue;
                }

                this->physicsSystem.RegisterCollider3D(this->bodyID, key, collider);
            }
        }
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) { return; }

		for (SubShapeID::Type i = 0; i < static_cast<SubShapeID::Type>(this->colliders.size()); ++i)
		{
			this->physicsSystem.UnregisterCollider3D(this->bodyID, i);
		}

		this->physicsSystem.UnregisterRigidbody3D(this->bodyID);

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}
} // namespace Framework::Physics
