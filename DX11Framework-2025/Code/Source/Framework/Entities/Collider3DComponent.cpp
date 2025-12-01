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

	void Collider3DComponent::SetCenterOffset(const DX::Vector3& _offset)
	{
		this->centerOffset = _offset;
	}

	void Collider3DComponent::BuildShape()
	{
		DX::Vector3 scale = this->Owner()->transform->GetWorldScale();

		// 球に使う統一スケール（非一様スケールの取り扱い方）
		const float uniformScale = std::max({ std::fabs(scale.x), std::fabs(scale.y), std::fabs(scale.z) });

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
			// 球は等方スケール扱い：ワールドの最大成分を採用して半径を決定
			float radius = this->sphereRadius * uniformScale;
			JPH::SphereShapeSettings settings(radius);
			this->shape = settings.Create().Get();
			break;
		}

		case ColliderShapeType::Capsule:
		{
			// 半径は X 成分基準、円柱半高さは Y 成分基準（既存方針を維持）
			float r = this->capsuleRadius * std::fabs(scale.x);
			float h = this->capsuleHalfHeight * std::fabs(scale.y);

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
}