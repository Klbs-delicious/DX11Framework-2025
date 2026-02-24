/** @file   HitComponent.cpp
 *  @brief  被弾処理（攻撃を受けた側の共通処理）
 *  @date   2026/02/17
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/HitComponent.h"

//-----------------------------------------------------------------------------
// HitComponent class
//-----------------------------------------------------------------------------

HitComponent::HitComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive)
{
}

void HitComponent::Initialize()
{
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
}

void HitComponent::OnHit(const HitInfo& _hit)
{
	const auto tag = this->Owner()->GetTag();

	if (tag == GameTags::Tag::Enemy)
	{
		std::cout << "[HitComponent] Enemy Hit damage=" << _hit.damage << " knockback=" << _hit.knockback << "\n";

		DX::Vector3 dir = _hit.hitDirWorld;
		dir.y = 0.0f;

		if (dir.LengthSquared() > 1.0e-6f)
		{
			dir.Normalize();
		}
		else
		{
			dir = DX::Vector3::UnitZ;
		}

		// 物理が効くなら速度で飛ばす
		// 効かないなら後で位置移動に差し替える
		if (this->rigidbody)
		{
			DX::Vector3 v = this->rigidbody->GetLinearVelocity();
			DX::Vector3 add = dir * _hit.knockback;
			add.y = 0.0f;

			this->rigidbody->SetLinearVelocity(DX::Vector3(add.x, v.y, add.z));
		}

		return;
	}

	if (tag == GameTags::Tag::Player)
	{
		std::cout << "[HitComponent] Player Hit damage=" << _hit.damage << "\n";
		return;
	}
}
