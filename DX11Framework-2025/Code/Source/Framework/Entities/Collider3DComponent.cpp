/** @file   Collider3DComponent.cpp
 *  @brief  Collider3DComponent の実装
 *  @date   2025/11/17
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Collider3DComponent.h"
#include "Include/Framework/Entities/GameObject.h"

// Jolt Physics Shape includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include <algorithm>
#include <cmath>

namespace Framework::Physics
{
	//-----------------------------------------------------------------------------
	// Collider3DComponent Class
	//-----------------------------------------------------------------------------

	Collider3DComponent::Collider3DComponent(GameObject* _owner, bool _active)
		: Component(_owner, _active)
		, shapeType(ColliderShapeType::Box)
		, shape(nullptr)
		, transform(nullptr)
		, boxHalfExtent(0.5f, 0.5f, 0.5f)
		, sphereRadius(0.5f)
		, capsuleRadius(0.5f)
		, capsuleHalfHeight(0.5f)
		, centerOffset(DX::Vector3::Zero)
		, isTrigger(false)
		, shapeSettings(nullptr)
	{}

	void Collider3DComponent::Initialize()
	{
		this->transform = this->Owner()->transform;

		// 現状では Shape 設定の構築と Shape の生成は Rigidbody3D 側で行う（生成順序が不定のため）
		//// 形状設定を構築してから Shape を生成する
		//this->BuildShapeSettings();
		//this->shape = this->CreateShape();
	}

	void Collider3DComponent::Dispose()
	{
		this->shape = nullptr;
		this->transform = nullptr;
	}

	void Collider3DComponent::SetShape(ColliderShapeType _shapeType)
	{
		this->shapeType = _shapeType;
	}

	void Collider3DComponent::SetBoxHalfExtent(const DX::Vector3& _half)
	{
		this->boxHalfExtent = _half;
	}

	void Collider3DComponent::SetSphereRadius(float _radius)
	{
		this->sphereRadius = _radius;
	}

	void Collider3DComponent::SetCapsule(float _radius, float _halfHeight)
	{
		this->capsuleRadius = _radius;
		this->capsuleHalfHeight = _halfHeight;
	}

	void Collider3DComponent::SetCenterOffset(const DX::Vector3& _offset)
	{
		this->centerOffset = _offset;
	}

	void Collider3DComponent::SetisTrigger(bool _isTrigger)
	{
		this->isTrigger = _isTrigger;
	}

	void Collider3DComponent::BuildShapeSettings()
	{
		DX::Vector3 scale = this->Owner()->transform->GetWorldScale();

		const float uniformScale = std::max({ std::fabs(scale.x), std::fabs(scale.y), std::fabs(scale.z) });

		switch (this->shapeType)
		{
		case ColliderShapeType::Box:
		{
			DX::Vector3 size = this->boxHalfExtent * scale;

			this->shapeSettings = new JPH::BoxShapeSettings(JPH::Vec3(size.x, size.y, size.z));
			break;
		}

		case ColliderShapeType::Sphere:
		{
			float radius = this->sphereRadius * uniformScale;

			this->shapeSettings = new JPH::SphereShapeSettings(radius);
			break;
		}

		case ColliderShapeType::Capsule:
		{
			float radius = this->capsuleRadius * std::fabs(scale.x);
			float height = this->capsuleHalfHeight * std::fabs(scale.y);

			this->shapeSettings = new JPH::CapsuleShapeSettings(height, radius);
			break;
		}

		case ColliderShapeType::Mesh:
			this->shapeSettings = nullptr;
			break;

		default:
			this->shapeSettings = nullptr;
			break;
		}
	}

	void Collider3DComponent::CreateShape()
	{
		if (!this->shapeSettings){ return; }

		// Shapeを生成する
		this->shape = this->shapeSettings->Create().Get();
	}

	JPH::ShapeRefC Collider3DComponent::GetShape() const
	{
		return this->shape;
	}

	ColliderShapeType& Collider3DComponent::GetShapeType()
	{
		return this->shapeType;
	}

	DX::Vector3& Collider3DComponent::GetBoxHalfExtent()
	{
		return this->boxHalfExtent;
	}
	float& Collider3DComponent::GetSphereRadius()
	{
		return this->sphereRadius;
	}

	float& Collider3DComponent::GetCapsuleRadius()
	{
		return this->capsuleRadius;
	}

	float& Collider3DComponent::GetCapsuleHalfHeight()
	{
		return this->capsuleHalfHeight;
	}

	DX::Vector3& Collider3DComponent::GetCenterOffset()
	{
		return this->centerOffset;
	}

	bool Collider3DComponent::IsTrigger() const
	{
		return this->isTrigger;
	}

	JPH::Ref<JPH::ShapeSettings> Collider3DComponent::GetShapeSettings() const
	{
		return this->shapeSettings;
	}
}