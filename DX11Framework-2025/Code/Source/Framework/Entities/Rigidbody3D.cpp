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
#include "Include/Framework/Core/SystemLocator.h"

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

	/** @brief コンストラクタ
	 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param bool _isActive コンポーネントの有効 / 無効
	 */
	Rigidbody3D::Rigidbody3D(GameObject* _owner, bool _isActive) :
		Component(_owner, _isActive),
		bodyID(),
		hasBody(false),
		mass(1.0f),
		gravityScale(1.0f),
		motionType(EMotionType::Dynamic),
		collider(nullptr),
		transform(nullptr),
		physicsSystem(SystemLocator::Get<PhysicsSystem>())
	{
	}

	/// @brief デストラクタ
	Rigidbody3D::~Rigidbody3D() noexcept
	{
		this->DestroyBody();
	}

	/// @brief 初期化処理
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

	/** @brief 重力スケールを設定する
	 *  @param float _scale 重力スケール
	 */
	void Rigidbody3D::SetGravityScale(float _scale)
	{
		this->gravityScale = _scale;

		if (this->hasBody)
		{
			auto& bodyInterface = this->physicsSystem.GetBodyInterface();
			bodyInterface.SetGravityFactor(this->bodyID, _scale);
		}
	}

	/** @brief Transform から初期位置・回転を取得する
	 */
	void Rigidbody3D::GetInitialTransform(DX::Vector3& _outPosition, DX::Quaternion& _outRotation) const
	{
		if (this->transform != nullptr)
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

	/** @brief BodyCreationSettings をセットアップする
	 */
	void Rigidbody3D::SetupBodyCreationSettings(BodyCreationSettings& _settings) const
	{
		DX::Vector3 pos;
		DX::Quaternion rot;
		this->GetInitialTransform(pos, rot);

		_settings.mPosition = RVec3(pos.x, pos.y, pos.z);
		_settings.mRotation = Quat(rot.x, rot.y, rot.z, rot.w);
		_settings.mMotionType = this->motionType;

		// 適切な ObjectLayer を設定（修正点）
		if (this->motionType == EMotionType::Static)
		{
			_settings.mObjectLayer = PhysicsLayer::Static;
		}
		else if (this->motionType == EMotionType::Dynamic)
		{
			_settings.mObjectLayer = PhysicsLayer::Dynamic;
		}
		else
		{
			_settings.mObjectLayer = PhysicsLayer::Kinematic;
		}

		if (this->collider)
		{
			JPH::ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}

		_settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		_settings.mMassPropertiesOverride.mMass = this->mass;
	}

	/// @brief 物理シミュレーション → Transform に反映
	void Rigidbody3D::SyncTransform()
	{
		if (!this->hasBody) { return; }

		BodyLockRead lock(
			this->physicsSystem.GetBodyLockInterface(),
			this->bodyID
		);
		if (!lock.Succeeded()) { return; }

		const Body& body = lock.GetBody();

		const RVec3& jPos = body.GetPosition();
		const Quat& jRot = body.GetRotation();

		DX::Vector3 pos(
			static_cast<float>(jPos.GetX()),
			static_cast<float>(jPos.GetY()),
			static_cast<float>(jPos.GetZ())
		);

		DX::Quaternion rot(
			static_cast<float>(jRot.GetX()),
			static_cast<float>(jRot.GetY()),
			static_cast<float>(jRot.GetZ()),
			static_cast<float>(jRot.GetW())
		);

		if (this->transform != nullptr)
		{
			this->transform->SetWorldPosition(pos);
			this->transform->SetWorldRotation(rot);
		}
	}

	/// @brief Body を生成
	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody) { return; }

		BodyInterface& bodyInterface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodyCreationSettings(settings);

		Body* body = bodyInterface.CreateBody(settings);
		if (!body) { return; }

		bodyInterface.AddBody(body->GetID(), EActivation::Activate);
		bodyInterface.SetGravityFactor(body->GetID(), this->gravityScale);

		this->bodyID = body->GetID();
		this->hasBody = true;
	}

	/// @brief 更新処理（Transform へ反映）
	void Rigidbody3D::Update(float _deltaTime)
	{
		if (!this->hasBody) { return; }

		this->SyncTransform();
	}

	/// @brief 物理ボディを破棄
	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) { return; }

		BodyInterface& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->bodyID = BodyID();
		this->hasBody = false;
	}

	/** @brief 質量設定（再生成）
	 */
	void Rigidbody3D::SetMass(float _mass)
	{
		this->mass = _mass;

		if (this->hasBody)
		{
			this->DestroyBody();
			this->InitializeBody();
		}
	}

	/** @brief MotionType 設定
	 */
	void Rigidbody3D::SetMotionType(EMotionType _motionType)
	{
		this->motionType = _motionType;

		if (!this->hasBody) { return; }

		BodyInterface& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetMotionType(this->bodyID, this->motionType, EActivation::Activate);
	}

	void Rigidbody3D::SetLinearVelocity(const DX::Vector3& _velocity)
	{
		if (!this->hasBody) { return; }

		BodyInterface& bodyInterface = this->physicsSystem.GetBodyInterface();
		Vec3 v(_velocity.x, _velocity.y, _velocity.z);
		bodyInterface.SetLinearVelocity(this->bodyID, v);
	}

	DX::Vector3 Rigidbody3D::GetLinearVelocity() const
	{
		if (!this->hasBody) { return DX::Vector3::Zero; }

		const BodyInterface& iface = this->physicsSystem.GetBodyInterface();
		Vec3 v = iface.GetLinearVelocity(this->bodyID);

		return DX::Vector3(v.GetX(), v.GetY(), v.GetZ());
	}

	void Rigidbody3D::AddForce(const DX::Vector3& _force)
	{
		if (!this->hasBody) { return; }

		BodyInterface& iface = this->physicsSystem.GetBodyInterface();
		Vec3 f(_force.x, _force.y, _force.z);
		iface.AddForce(this->bodyID, f, EActivation::Activate);
	}

	void Rigidbody3D::AddImpulse(const DX::Vector3& impulse)
	{
		if (!this->hasBody) { return; }

		Vec3 v(impulse.x, impulse.y, impulse.z);
		this->physicsSystem.GetBodyInterface().AddImpulse(this->bodyID, v);
	}

} // namespace Framework::Physics