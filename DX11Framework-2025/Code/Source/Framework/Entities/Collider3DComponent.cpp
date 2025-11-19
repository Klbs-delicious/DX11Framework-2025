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
	{
	}

	void Collider3DComponent::Initialize()
	{
		this->transform = this->Owner()->transform;

		// 形状を生成する
		this->BuildShape();
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

	void Collider3DComponent::BuildShape()
	{
		DX::Vector3 scale = this->Owner()->transform->GetWorldScale();  // 追加

		switch (this->shapeType)
		{
		case ColliderShapeType::Box:
		{
			DX::Vector3 size = this->boxHalfExtent * scale;

			JPH::BoxShapeSettings settings(JPH::Vec3(size.x, size.y, size.z));
			this->shape = settings.Create().Get();
			break;
		}

		case ColliderShapeType::Sphere:
		{
			float radius = this->sphereRadius * scale.x;
			JPH::SphereShapeSettings settings(radius);
			this->shape = settings.Create().Get();
			break;
		}

		case ColliderShapeType::Capsule:
		{
			float r = this->capsuleRadius * scale.x;
			float h = this->capsuleHalfHeight * scale.y;

			JPH::CapsuleShapeSettings settings(h, r);
			this->shape = settings.Create().Get();
			break;
		}
		case ColliderShapeType::Mesh:
			// 未実装
			this->shape = nullptr;
			break;

		default:
			this->shape = nullptr;
			break;
		}
	}

	JPH::ShapeRefC Collider3DComponent::GetShape() const
	{
		return this->shape;
	}
}