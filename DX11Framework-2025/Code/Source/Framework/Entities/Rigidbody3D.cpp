/** @file   Rigidbody3D.cpp
 *  @brief  TimeScale 対応 3D リジッドボディ（物理は実時間）
 *  @date   2025/11/26
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>

#include <algorithm>

namespace Framework::Physics
{
	using namespace JPH;

	//-----------------------------------------------------------------------------
	// Constructor / Destructor
	//-----------------------------------------------------------------------------
	Rigidbody3D::Rigidbody3D(GameObject* _owner, bool _active)
		: Component(_owner, _active)
		, bodyID()
		, hasBody(false)
		, mass(1.0f)
		, friction(0.5f)
		, restitution(0.5f)
		, gravityScale(1.0f)
		, motionType(EMotionType::Dynamic)
		, transform(nullptr)
		, timeScaleComp(nullptr)
		, collider(nullptr)
		, physicsSystem(SystemLocator::Get<PhysicsSystem>())
		, scaledVelocity(DX::Vector3::Zero)
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
		this->transform = this->Owner()->GetComponent<Transform>();
		this->timeScaleComp = this->Owner()->GetComponent<TimeScaleComponent>();
		this->collider = this->Owner()->GetComponent<Collider3DComponent>();

		if (!this->collider)
		{
			this->collider = this->Owner()->AddComponent<Collider3DComponent>();
		}

		this->InitializeBody();
	}

	void Rigidbody3D::Dispose()
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// Body Setup
	//-----------------------------------------------------------------------------
	void Rigidbody3D::GetInitialTransform(DX::Vector3& _outPos, DX::Quaternion& _outRot) const
	{
		if (this->transform)
		{
			_outPos = this->transform->GetWorldPosition();
			_outRot = this->transform->GetWorldRotation();
		}
		else
		{
			_outPos = DX::Vector3::Zero;
			_outRot = DX::Quaternion::Identity;
		}
	}

	void Rigidbody3D::SetupBodyCreationSettings(BodyCreationSettings& _settings) const
	{
		DX::Vector3 pos;
		DX::Quaternion rot;
		this->GetInitialTransform(pos, rot);

		_settings.mPosition = RVec3(pos.x, pos.y, pos.z);
		_settings.mRotation = Quat(rot.x, rot.y, rot.z, rot.w);
		_settings.mMotionType = this->motionType;

		if (this->collider)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}

		_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		_settings.mMassPropertiesOverride.mMass = this->mass;
	}

	//-----------------------------------------------------------------------------
	// Body create / destroy
	//-----------------------------------------------------------------------------
	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody) return;

		auto& iface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodyCreationSettings(settings);

		Body* body = iface.CreateBody(settings);
		if (!body) return;

		iface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;

		iface.SetFriction(this->bodyID, this->friction);
		iface.SetRestitution(this->bodyID, this->restitution);
		iface.SetGravityFactor(this->bodyID, this->gravityScale);
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) return;

		auto& iface = this->physicsSystem.GetBodyInterface();
		iface.RemoveBody(this->bodyID);
		iface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}

	//-----------------------------------------------------------------------------
	// ApplyPhysicsResults（TimeScaleVelocity の中核処理）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ApplyPhysicsResults(float _fixedDelta)
	{
		if (!this->hasBody || !this->transform) return;

		//---------------------------------------------------------
		// 1) Jolt の本物の速度（realVelocity）を取得
		//---------------------------------------------------------
		Vec3 jVel = this->physicsSystem.GetBodyInterface().GetLinearVelocity(this->bodyID);
		DX::Vector3 realVel(jVel.GetX(), jVel.GetY(), jVel.GetZ());

		//---------------------------------------------------------
		// 2) TimeScale を取得
		//---------------------------------------------------------
		float scale = 1.0f;
		if (this->timeScaleComp)
		{
			scale = this->timeScaleComp->GetFinalScale();
		}

		//---------------------------------------------------------
		// 3) scaledVelocity = realVelocity * TimeScale
		//---------------------------------------------------------
		this->scaledVelocity = realVel * scale;

		//---------------------------------------------------------
		// 4) Transform に見た目の移動を適用
		//---------------------------------------------------------
		DX::Vector3 move = this->scaledVelocity * _fixedDelta;

		DX::Vector3 newPos = this->transform->GetWorldPosition() + move;
		this->transform->SetWorldPosition(newPos);

		//---------------------------------------------------------
		// 5) 動的剛体の場合、Jolt へ scaledVelocity を戻す（速度の連続性）
		//---------------------------------------------------------
		if (this->motionType == EMotionType::Dynamic)
		{
			JPH::Vec3 newJoltVel(move.x / _fixedDelta, move.y / _fixedDelta, move.z / _fixedDelta);
			this->physicsSystem.GetBodyInterface().SetLinearVelocity(this->bodyID, newJoltVel);
		}
	}

	//-----------------------------------------------------------------------------
	// Velocity / Force
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SetLinearVelocity(const DX::Vector3& _vel)
	{
		if (!this->hasBody) return;
		JPH::Vec3 v(_vel.x, _vel.y, _vel.z);
		this->physicsSystem.GetBodyInterface().SetLinearVelocity(this->bodyID, v);
	}

	DX::Vector3 Rigidbody3D::GetLinearVelocity() const
	{
		if (!this->hasBody) return DX::Vector3::Zero;

		JPH::Vec3 v = this->physicsSystem.GetBodyInterface().GetLinearVelocity(this->bodyID);
		return DX::Vector3(v.GetX(), v.GetY(), v.GetZ());
	}

	void Rigidbody3D::AddForce(const DX::Vector3& _force)
	{
		if (!this->hasBody) return;
		JPH::Vec3 f(_force.x, _force.y, _force.z);
		this->physicsSystem.GetBodyInterface().AddForce(this->bodyID, f);
	}

	void Rigidbody3D::AddImpulse(const DX::Vector3& _impulse)
	{
		if (!this->hasBody) return;
		JPH::Vec3 i(_impulse.x, _impulse.y, _impulse.z);
		this->physicsSystem.GetBodyInterface().AddImpulse(this->bodyID, i);
	}

} // namespace Framework::Physics
