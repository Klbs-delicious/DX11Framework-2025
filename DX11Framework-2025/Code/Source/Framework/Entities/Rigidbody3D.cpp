/** @file   Rigidbody3D.cpp
 *  @brief  Transform 主導の衝突判定専用ボディコンポーネント
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
		, stagedTransform(nullptr)
		, visualTransform(nullptr)
		, physicsSystem(SystemLocator::Get<PhysicsSystem>())
		, collider(nullptr)
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

		// ロジック側 Transform の生成と初期化
		this->stagedTransform = std::make_unique<StagedTransform>();

		if (this->visualTransform)
		{
			this->stagedTransform->position = this->visualTransform->GetWorldPosition();
			this->stagedTransform->rotation = this->visualTransform->GetWorldRotation();
			this->stagedTransform->scale = this->visualTransform->GetWorldScale();
		}
		else
		{
			this->stagedTransform->position = DX::Vector3::Zero;
			this->stagedTransform->rotation = DX::Quaternion::Identity;
			this->stagedTransform->scale = DX::Vector3::One;
		}

		// Collider が無ければ追加
		if (!this->collider)
		{
			this->collider = this->Owner()->AddComponent<Collider3DComponent>();
		}

		// Body の初期化
		this->InitializeBody();
	}

	void Rigidbody3D::Dispose()
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// StagedTransform Update
	//-----------------------------------------------------------------------------
	/** @brief 論理変換を更新する */
	void Rigidbody3D::UpdateLogical(float _deltaTime)
	{
		(void)_deltaTime;

		if (!this->stagedTransform)
		{
			return;
		}

		// TODO:
		// - TimeScale の適用
		// - 重力・移動・ジャンプなどの自前計算
		// - キャラ操作ロジックの反映
		// 現状は「値を維持するだけ」の空実装
	}

	//-----------------------------------------------------------------------------
	// Sync logical → visual
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncToVisual() const
	{
		if (!this->visualTransform || !this->stagedTransform)
		{
			return;
		}

		this->visualTransform->SetWorldPosition(this->stagedTransform->position);
		this->visualTransform->SetWorldRotation(this->stagedTransform->rotation);
		this->visualTransform->SetWorldScale(this->stagedTransform->scale);
	}

	//-----------------------------------------------------------------------------
	// Sync visual → Jolt
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncVisualToJolt() const
	{
		if (!this->hasBody || !this->visualTransform)
		{
			return;
		}

		auto& iface = this->physicsSystem.GetBodyInterface();

		DX::Vector3 pos = this->visualTransform->GetWorldPosition();
		DX::Quaternion rot = this->visualTransform->GetWorldRotation();

		iface.SetPositionAndRotation(
			this->bodyID,
			RVec3(pos.x, pos.y, pos.z),
			Quat(rot.x, rot.y, rot.z, rot.w),
			EActivation::Activate
		);
	}

	//-----------------------------------------------------------------------------
	// Sync Jolt → visual
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncJoltToVisual() const
	{
		if (!this->hasBody || !this->visualTransform)
		{
			return;
		}

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded())
		{
			return;
		}

		const Body& body = lock.GetBody();

		DX::Vector3 pos(
			(float)body.GetPosition().GetX(),
			(float)body.GetPosition().GetY(),
			(float)body.GetPosition().GetZ()
		);

		DX::Quaternion rot(
			(float)body.GetRotation().GetX(),
			(float)body.GetRotation().GetY(),
			(float)body.GetRotation().GetZ(),
			(float)body.GetRotation().GetW()
		);

		this->visualTransform->SetWorldPosition(pos);
		this->visualTransform->SetWorldRotation(rot);
	}

	//-----------------------------------------------------------------------------
	// Body Setup
	//-----------------------------------------------------------------------------
	void Rigidbody3D::GetInitialTransform(DX::Vector3& _pos, DX::Quaternion& _rot) const
	{
		if (!this->visualTransform)
		{
			_pos = DX::Vector3::Zero;
			_rot = DX::Quaternion::Identity;
			return;
		}

		_pos = this->visualTransform->GetWorldPosition();
		_rot = this->visualTransform->GetWorldRotation();
	}

	void Rigidbody3D::SetupBodySettings(BodyCreationSettings& _settings) const
	{
		DX::Vector3 pos;
		DX::Quaternion rot;
		this->GetInitialTransform(pos, rot);

		_settings.mPosition = RVec3(pos.x, pos.y, pos.z);
		_settings.mRotation = Quat(rot.x, rot.y, rot.z, rot.w);

		_settings.mMotionType = EMotionType::Kinematic;
		_settings.mMotionQuality = EMotionQuality::Discrete;

		if (this->collider)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}
	}

	//-----------------------------------------------------------------------------
	// Body create / destroy
	//-----------------------------------------------------------------------------
	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody)
		{
			return;
		}

		auto& iface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodySettings(settings);

		Body* body = iface.CreateBody(settings);
		if (!body)
		{
			return;
		}

		iface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody)
		{
			return;
		}

		auto& iface = this->physicsSystem.GetBodyInterface();
		iface.RemoveBody(this->bodyID);
		iface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}

} // namespace Framework::Physics