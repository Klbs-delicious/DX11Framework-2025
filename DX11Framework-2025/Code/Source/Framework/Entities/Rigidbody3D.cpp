/** @file   Rigidbody3D.cpp
 *  @brief  Jolt Physics 用 3D リジッドボディコンポーネント実装
 *  @date   2025/11/17
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
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Body/BodyInterface.h>
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
		, motionType(EMotionType::Dynamic)
		, collider(nullptr)
	{
	}

	Rigidbody3D::~Rigidbody3D() noexcept
	{
		this->DestroyBody();
	}

	PhysicsSystem* Rigidbody3D::GetPhysicsSystem()
	{
		return &SystemLocator::Get<PhysicsSystem>();
	}

	void Rigidbody3D::GetInitialTransform(DX::Vector3& outPosition, DX::Quaternion& outRotation) const
	{
		Transform* transform = this->Owner()->transform;
		if (transform != nullptr)
		{
			outPosition = transform->GetWorldPosition();
			outRotation = transform->GetWorldRotation();
		}
		else
		{
			outPosition = DX::Vector3::Zero;
			outRotation = DX::Quaternion::Identity;
		}
	}

	void Rigidbody3D::SetupBodyCreationSettings(BodyCreationSettings& _settings) const
	{
		DX::Vector3 pos;
		DX::Quaternion rot;
		this->GetInitialTransform(pos, rot);

		RVec3 joltPos(pos.x, pos.y, pos.z);
		Quat  joltRot(rot.x, rot.y, rot.z, rot.w);

		_settings.mPosition = joltPos;
		_settings.mRotation = joltRot;
		_settings.mMotionType = this->motionType;

		// コライダーから形状を取得して設定する
		if (this->collider != nullptr)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}

		// 質量の上書き設定（5.4.0 で存在する列挙のみ使用）
		_settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		_settings.mMassPropertiesOverride.mMass = this->mass;
	}

	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody)
		{
			return;
		}

		// コライダーが未設定なら同一 GameObject から取得を試みる
		if (this->collider == nullptr)
		{
			this->collider = this->Owner()->GetComponent<Collider3DComponent>();
		}

		if (this->collider == nullptr)
		{
			// 形状がないので生成できない
			return;
		}

		PhysicsSystem* physics = this->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return;
		}

		BodyInterface& bodyInterface = physics->GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodyCreationSettings(settings);

		Body* body = bodyInterface.CreateBody(settings);
		if (body == nullptr)
		{
			return;
		}

		bodyInterface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody)
		{
			return;
		}

		PhysicsSystem* physics = this->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return;
		}

		BodyInterface& bodyInterface = physics->GetBodyInterface();

		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->bodyID = BodyID();
		this->hasBody = false;
	}

	void Rigidbody3D::SetMass(float _mass)
	{
		this->mass = _mass;

		// すでに Body がある場合は作り直す
		if (this->hasBody)
		{
			this->DestroyBody();
			this->InitializeBody();
		}
	}

	void Rigidbody3D::SetMotionType(EMotionType _motionType)
	{
		this->motionType = _motionType;

		if (!this->hasBody)
		{
			return;
		}

		PhysicsSystem* physics = this->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return;
		}

		BodyInterface& bodyInterface = physics->GetBodyInterface();
		bodyInterface.SetMotionType(this->bodyID, this->motionType, EActivation::Activate);
	}

	void Rigidbody3D::SetLinearVelocity(const DX::Vector3& _velocity)
	{
		if (!this->hasBody)
		{
			return;
		}

		PhysicsSystem* physics = this->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return;
		}

		BodyInterface& bodyInterface = physics->GetBodyInterface();
		Vec3 v(_velocity.x, _velocity.y, _velocity.z);
		bodyInterface.SetLinearVelocity(this->bodyID, v);
	}

	DX::Vector3 Rigidbody3D::GetLinearVelocity() const
	{
		if (!this->hasBody)
		{
			return DX::Vector3::Zero;
		}

		PhysicsSystem* physics = const_cast<Rigidbody3D*>(this)->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return DX::Vector3::Zero;
		}

		const BodyInterface& bodyInterface = physics->GetBodyInterface();
		Vec3 v = bodyInterface.GetLinearVelocity(this->bodyID);
		return DX::Vector3(v.GetX(), v.GetY(), v.GetZ());
	}

	void Rigidbody3D::AddForce(const DX::Vector3& _force)
	{
		if (!this->hasBody)
		{
			return;
		}

		PhysicsSystem* physics = this->GetPhysicsSystem();
		if (physics == nullptr)
		{
			return;
		}

		BodyInterface& bodyInterface = physics->GetBodyInterface();
		Vec3 f(_force.x, _force.y, _force.z);
		bodyInterface.AddForce(this->bodyID, f, EActivation::Activate);
	}
}