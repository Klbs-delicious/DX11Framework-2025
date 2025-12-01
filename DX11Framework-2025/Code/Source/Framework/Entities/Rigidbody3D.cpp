/** @file   Rigidbody3D.cpp
 *  @brief  TimeScale 対応：自前移動＋Jolt 押し戻し用 Rigidbody3D 実装
 *  @date   2025/11/28
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>  
#include <Jolt/Physics/Collision/CastResult.h> 
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <cfloat>                             

namespace Framework::Physics
{
	using namespace JPH;

	//-----------------------------------------------------------------------------
	// 定数
	//-----------------------------------------------------------------------------
	static constexpr float parallelEps = 1.0e-4f;

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
		, collider(nullptr)
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
		// Transform / Collider の取得
		this->visualTransform = this->Owner()->GetComponent<Transform>();
		this->collider = this->Owner()->GetComponent<Collider3DComponent>();

		// ロジック側 Transform の生成
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

		// Collider が無ければ追加
		if (!this->collider)
		{
			this->collider = this->Owner()->AddComponent<Collider3DComponent>();
		}

		// Body を生成
		this->InitializeBody();
	}

	void Rigidbody3D::Dispose()
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// 自前移動（TimeScale 適用済み）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::UpdateLogical(float _deltaTime)
	{
		if (!this->staged) { return; }
		if (_deltaTime <= 0.0f) { return; }

		// 前フレーム姿勢を保存する
		*(this->stagedPrev) = *(this->staged);

		// 重力を適用
		if (this->useGravity)
		{
			if (this->isGrounded)
			{
				// 接地中は垂直成分のみリセット
				this->linearVelocity.y = 0.0f;
			}
			else
			{
				this->linearVelocity += this->gravity * _deltaTime;
			}
		}

		// 速度を積分して位置を更新する
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

		DX::Vector3 targetPos = this->staged->position;
		DX::Quaternion targetRot = this->staged->rotation;

		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), targetRot);
		}

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.MoveKinematic(
			this->bodyID,
			RVec3(targetPos.x + worldOffset.x, targetPos.y + worldOffset.y, targetPos.z + worldOffset.z),
			Quat(targetRot.x, targetRot.y, targetRot.z, targetRot.w),
			_deltaTime
		);
	}

	//-----------------------------------------------------------------------------
	// Jolt → visual → staged（押し戻し結果を確定）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncJoltToVisual()
	{
		if (!this->hasBody || !this->visualTransform) { return; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return; }
		const Body& body = lock.GetBody();

		DX::Vector3 bodyPos(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);

		DX::Quaternion bodyRot(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);

		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), bodyRot);
		}

		DX::Vector3 pivotPos = bodyPos - worldOffset;

		this->visualTransform->SetWorldPosition(pivotPos);
		this->visualTransform->SetWorldRotation(bodyRot);

		this->staged->position = pivotPos;
		this->staged->rotation = bodyRot;
	}

	//-----------------------------------------------------------------------------
	// CastShape 押し戻し解決
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ResolveCastShape()
	{
		if (!this->hasBody) { return; }
		if (!this->staged || !this->stagedPrev) { return; }
		if (!this->collider) { return; }

		this->isGrounded = false;

		DX::Vector3 move = this->staged->position - this->stagedPrev->position;
		float moveLenSq = move.LengthSquared();
		if (moveLenSq <= 0.0f) { return; }
		float moveLen = std::sqrt(moveLenSq);
		DX::Vector3 moveDir = move / moveLen;

		const DX::Quaternion prevRot = this->stagedPrev->rotation;

		DX::Vector3 offset = DX::Vector3::Transform(this->collider->GetCenterOffset(), prevRot);
		DX::Vector3 comStart = this->stagedPrev->position + offset;

		JPH::RVec3 jStart(comStart.x, comStart.y, comStart.z);
		JPH::Vec3  jMove(move.x, move.y, move.z);
		JPH::Quat  jRot(prevRot.x, prevRot.y, prevRot.z, prevRot.w);
		JPH::RMat44 jWorld = JPH::RMat44::sRotationTranslation(jRot, jStart);
		JPH::Vec3  jScale(1.0f, 1.0f, 1.0f);

		const JPH::Shape* shape = this->collider->GetShape().GetPtr();
		if (!shape) { return; }

		JPH::RShapeCast shapeCast(shape, jScale, jWorld, jMove);

		JPH::ShapeCastSettings settings;
		settings.mReturnDeepestPoint = false;
		settings.mBackFaceModeTriangles = JPH::EBackFaceMode::IgnoreBackFaces;
		settings.mBackFaceModeConvex = JPH::EBackFaceMode::IgnoreBackFaces;

		Framework::Physics::ClosestShapeCastCollector collector;

		const JPH::NarrowPhaseQuery& npq = this->physicsSystem.GetNarrowPhaseQuery();
		IgnoreSelfBodyFilter bodyFilter(this->bodyID);

		npq.CastShape(
			shapeCast, settings,
			JPH::RVec3::sZero(),
			collector,
			JPH::BroadPhaseLayerFilter(),
			JPH::ObjectLayerFilter(),
			bodyFilter
		);

		if (!collector.hasHit) { return; }

		float f = std::clamp(collector.hit.mFraction, 0.0f, 1.0f);

		JPH::Vec3 jMoveDir = jMove / moveLen;
		float ndot = collector.hit.mPenetrationAxis.Dot(jMoveDir);

		if (f == 0.0f && std::abs(ndot) < parallelEps)
		{
			f = 1.0f;
		}

		const float skin = 1.0e-3f;
		float advance = std::max(0.0f, moveLen * f - skin);

		DX::Vector3 corrected = this->stagedPrev->position + moveDir * advance;

		this->staged->position = corrected;

		// 接地判定も行う
		this->CheckGrounded();
	}

	//-----------------------------------------------------------------------------
	// 接地判定
	//-----------------------------------------------------------------------------
	void Rigidbody3D::CheckGrounded()
	{
		this->isGrounded = false;

		if (!this->collider || !this->hasBody) { return; }

		float radius = this->collider->GetCapsuleRadius();
		float halfH = this->collider->GetCapsuleHalfHeight();
		float epsilon = 0.05f;

		DX::Vector3 worldPos = this->staged->position;
		float rayStartY = worldPos.y - (halfH + radius * 0.5f);

		JPH::RVec3 from(worldPos.x, rayStartY, worldPos.z);
		JPH::RVec3 to(worldPos.x, rayStartY - (radius + epsilon), worldPos.z);

		JPH::RRayCast ray(from, to - from);

		JPH::RayCastResult hit;
		if (physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit))
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

		DX::Vector3 forward = DX::Vector3::Transform(DX::Vector3::Forward, worldRot);
		DX::Vector3 right = DX::Vector3::Transform(DX::Vector3::Right, worldRot);
		DX::Vector3 up = DX::Vector3::Transform(DX::Vector3::Up, worldRot);

		DX::Vector3 worldDelta =
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

	bool Rigidbody3D::GetBodyTransform(DX::Vector3& outPos, DX::Quaternion& outRot) const
	{
		if (!this->hasBody) { return false; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return false; }

		const Body& body = lock.GetBody();
		outPos = DX::Vector3(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);
		outRot = DX::Quaternion(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);
		return true;
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

		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), _rot);
		}

		_pos = pivotPos + worldOffset;
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

		if (this->collider)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
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

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodySettings(settings);

		Body* body = bodyInterface.CreateBody(settings);
		if (!body) { return; }

		bodyInterface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}

} // namespace Framework::Physics
