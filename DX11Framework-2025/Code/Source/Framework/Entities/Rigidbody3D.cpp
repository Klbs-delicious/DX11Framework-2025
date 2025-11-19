/** @file   Rigidbody3D.cpp
 *  @brief  Jolt Physics 用 3D リジッドボディコンポーネント実装
 *  @date   2025/11/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/PhysicsSystem.h"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>

namespace Framework::Physics
{
	using namespace JPH;

	//-----------------------------------------------------------------------------
	// Rigidbody3D Class
	//-----------------------------------------------------------------------------

	Rigidbody3D::Rigidbody3D(GameObject* _owner, bool _isActive)
		: Component(_owner, _isActive)
		, bodyID()
		, hasBody(false)
		, mass(1.0f)
		, gravityScale(1.0f)
		, friction(0.5f)
		, restitution(0.5f)
		, isTrigger(false)
		, motionType(EMotionType::Dynamic)
		, transform(nullptr)
		, collider(nullptr)
		, physicsSystem(SystemLocator::Get<PhysicsSystem>())
	{
	}

	Rigidbody3D::~Rigidbody3D() noexcept
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// Initialize
	//-----------------------------------------------------------------------------

	void Rigidbody3D::Initialize()
	{
		this->transform = this->Owner()->GetComponent<Transform>();
		if (!this->collider)
		{
			this->collider = this->Owner()->GetComponent<Collider3DComponent>();
		}
		if (!this->collider)
		{
			this->collider = this->Owner()->AddComponent<Collider3DComponent>();
		}

		this->InitializeBody();
	}

	//-----------------------------------------------------------------------------
	// Body Setup
	//-----------------------------------------------------------------------------

	void Rigidbody3D::GetInitialTransform(DX::Vector3& _outPosition, DX::Quaternion& _outRotation) const
	{
		if (this->transform)
		{
			_outPosition = this->transform->GetWorldPosition();
			_outRotation = this->transform->GetWorldRotation();
		}
		else
		{
			_outPosition = DX::Vector3::Zero;
			_outRotation = DX::Quaternion::Identity;
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

		// ObjectLayer
		switch (this->motionType)
		{
		case EMotionType::Static:    _settings.mObjectLayer = PhysicsLayer::Static; break;
		case EMotionType::Dynamic:   _settings.mObjectLayer = PhysicsLayer::Dynamic; break;
		case EMotionType::Kinematic: _settings.mObjectLayer = PhysicsLayer::Kinematic; break;
		}

		// Shape
		if (this->collider)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}

		// Mass 設定（手動）
		_settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		_settings.mMassPropertiesOverride.mMass = this->mass;

		// Trigger 初期化
		_settings.mIsSensor = this->isTrigger;
	}

	//-----------------------------------------------------------------------------
	// Body Create / Destroy
	//-----------------------------------------------------------------------------

	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody) { return; }

		BodyInterface& iface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodyCreationSettings(settings);

		Body* body = iface.CreateBody(settings);
		if (!body) { return; }

		iface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;

		// friction / restitution / gravity の最終反映
		iface.SetFriction(this->bodyID, this->friction);
		iface.SetRestitution(this->bodyID, this->restitution);
		iface.SetGravityFactor(this->bodyID, this->gravityScale);
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) { return; }

		BodyInterface& iface = this->physicsSystem.GetBodyInterface();
		iface.RemoveBody(this->bodyID);
		iface.DestroyBody(this->bodyID);

		this->bodyID = BodyID();
		this->hasBody = false;
	}

	//-----------------------------------------------------------------------------
	// Sync Transform
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SyncTransform()
	{
		if (!this->hasBody) { return; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return; }

		const Body& body = lock.GetBody();
		RVec3 jPos = body.GetPosition();
		Quat  jRot = body.GetRotation();

		if (this->transform)
		{
			this->transform->SetWorldPosition(DX::Vector3(
				(float)jPos.GetX(), (float)jPos.GetY(), (float)jPos.GetZ()));
			this->transform->SetWorldRotation(DX::Quaternion(
				(float)jRot.GetX(), (float)jRot.GetY(), (float)jRot.GetZ(), (float)jRot.GetW()));
		}
	}

	//-----------------------------------------------------------------------------
	// Update
	//-----------------------------------------------------------------------------

	void Rigidbody3D::Update(float _deltaTime)
	{
		if (!this->hasBody) { return; }
		this->SyncTransform();
	}

	//-----------------------------------------------------------------------------
	// Force / Velocity
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetLinearVelocity(const DX::Vector3& _velocity)
	{
		if (!this->hasBody) { return; }

		Vec3 v(_velocity.x, _velocity.y, _velocity.z);
		this->physicsSystem.GetBodyInterface().SetLinearVelocity(this->bodyID, v);
	}

	DX::Vector3 Rigidbody3D::GetLinearVelocity() const
	{
		if (!this->hasBody) { return DX::Vector3::Zero; }

		Vec3 v = this->physicsSystem.GetBodyInterface().GetLinearVelocity(this->bodyID);
		return DX::Vector3(v.GetX(), v.GetY(), v.GetZ());
	}

	void Rigidbody3D::AddForce(const DX::Vector3& _force)
	{
		if (!this->hasBody) { return; }

		Vec3 f(_force.x, _force.y, _force.z);
		this->physicsSystem.GetBodyInterface().AddForce(this->bodyID, f, EActivation::Activate);
	}

	void Rigidbody3D::AddImpulse(const DX::Vector3& impulse)
	{
		if (!this->hasBody) { return; }

		Vec3 v(impulse.x, impulse.y, impulse.z);
		this->physicsSystem.GetBodyInterface().AddImpulse(this->bodyID, v);
	}

	//-----------------------------------------------------------------------------
	// Friction
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetFriction(float _friction)
	{
		this->friction = _friction;

		if (!this->bodyID.IsInvalid())
		{
			this->physicsSystem.GetBodyInterface().SetFriction(this->bodyID, _friction);
		}
	}

	//-----------------------------------------------------------------------------
	// Restitution
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetRestitution(float _restitution)
	{
		this->restitution = _restitution;

		if (!this->bodyID.IsInvalid())
		{
			this->physicsSystem.GetBodyInterface().SetRestitution(this->bodyID, _restitution);
		}
	}

	//-----------------------------------------------------------------------------
	// Trigger / Sensor
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetIsTrigger(bool _isTrigger)
	{
		this->isTrigger = _isTrigger;

		if (!this->hasBody) { return; }

		auto& bodyLockInterface = this->physicsSystem.GetBodyLockInterface();
		BodyLockWrite lock(bodyLockInterface, this->bodyID);
		if (lock.Succeeded())
		{
			lock.GetBody().SetIsSensor(_isTrigger);
		}
	}

	//-----------------------------------------------------------------------------
	// Mass（再生成）
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetMass(float _mass)
	{
		this->mass = _mass;

		if (this->hasBody)
		{
			this->DestroyBody();
			this->InitializeBody();
		}
	}

	//-----------------------------------------------------------------------------
	// MotionType
	//-----------------------------------------------------------------------------

	void Rigidbody3D::SetMotionType(EMotionType _motionType)
	{
		this->motionType = _motionType;

		if (!this->hasBody) { return; }

		this->physicsSystem.GetBodyInterface()
			.SetMotionType(this->bodyID, this->motionType, EActivation::Activate);
	}

} // namespace Framework::Physics