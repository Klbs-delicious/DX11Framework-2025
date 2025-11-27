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
		if (!this->staged)
		{
			return;
		}
		if (_deltaTime <= 0.0f)
		{
			return;
		}

		// 前フレーム姿勢を保存
		*(this->stagedPrev) = *(this->staged);

		// 重力の適用
		if (this->useGravity)
		{
			this->linearVelocity += this->gravity * _deltaTime;
		}

		// 速度を積分して位置を更新
		this->staged->position += this->linearVelocity * _deltaTime;

		// 回転については、現時点では外部から SetLogicalRotation で直接操作する想定
	}

	//-----------------------------------------------------------------------------
	// staged → visual
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncToVisual() const
	{
		if (!this->visualTransform || !this->staged)
		{
			return;
		}

		this->visualTransform->SetWorldPosition(this->staged->position);
		this->visualTransform->SetWorldRotation(this->staged->rotation);
		this->visualTransform->SetWorldScale(this->staged->scale);
	}

	//-----------------------------------------------------------------------------
	// visual → Jolt（Kinematic のみ）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncVisualToJolt(float _deltaTime)
	{
		if (!this->hasBody || !this->visualTransform)
		{
			return;
		}

		// Static の場合は Jolt 側を動かさない
		if (this->motionType != EMotionType::Kinematic)
		{
			return;
		}

		// staged / visual はすでに同期済みの前提
		DX::Vector3 targetPos = this->staged->position;
		DX::Quaternion targetRot = this->staged->rotation;

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.MoveKinematic(
			this->bodyID,
			RVec3(targetPos.x, targetPos.y, targetPos.z),
			Quat(targetRot.x, targetRot.y, targetRot.z, targetRot.w),
			_deltaTime
		);
	}

	//-----------------------------------------------------------------------------
	// Jolt → visual → staged（押し戻し結果を確定）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncJoltToVisual()
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

		DX::Vector3 worldPos(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);

		DX::Quaternion worldRot(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);

		// 見た目用 Transform に反映
		this->visualTransform->SetWorldPosition(worldPos);
		this->visualTransform->SetWorldRotation(worldRot);

		// ロジック側にも押し戻し結果を反映
		this->staged->position = worldPos;
		this->staged->rotation = worldRot;
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
		if (!this->staged)
		{
			return;
		}
		this->staged->position = _worldPos;
	}

	void Rigidbody3D::SetLogicalRotation(const DX::Quaternion& _worldRot)
	{
		if (!this->staged)
		{
			return;
		}
		this->staged->rotation = _worldRot;
	}

	void Rigidbody3D::TranslateWorld(const DX::Vector3& _delta)
	{
		if (!this->staged)
		{
			return;
		}
		this->staged->position += _delta;
	}

	void Rigidbody3D::TranslateLocal(const DX::Vector3& _delta)
	{
		if (!this->staged)
		{
			return;
		}

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

		_pos = this->visualTransform->GetWorldPosition();
		_rot = this->visualTransform->GetWorldRotation();
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
		if (!this->hasBody)
		{
			return;
		}

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetMotionType(
			this->bodyID,
			this->motionType,
			EActivation::Activate
		);

		// MotionQuality も合わせておく
		bodyInterface.SetMotionQuality(
			this->bodyID,
			(this->motionType == EMotionType::Kinematic)
			? EMotionQuality::LinearCast
			: EMotionQuality::Discrete
		);
	}

	void Rigidbody3D::ApplyObjectLayerToBody()
	{
		if (!this->hasBody)
		{
			return;
		}

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetObjectLayer(this->bodyID, this->objectLayer);
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

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodySettings(settings);

		Body* body = bodyInterface.CreateBody(settings);
		if (!body)
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

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}

} // namespace Framework::Physics