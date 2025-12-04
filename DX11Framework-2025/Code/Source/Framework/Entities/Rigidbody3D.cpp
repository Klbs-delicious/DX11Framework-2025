/** @file   Rigidbody3D.cpp
 *  @brief  TimeScale 対応：自前移動＋Jolt 押し戻し用 Rigidbody3D 実装
 *  @date   2025/11/28
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

#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/RayCast.h>

#include <Jolt/Physics/Collision/CollideShape.h>       
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h> 


#include <algorithm>
#include <cmath>
#include <cfloat>

namespace Framework::Physics
{
	using namespace JPH;

	//-----------------------------------------------------------------------------
	// 定数
	//-----------------------------------------------------------------------------
	static constexpr float parallelEps = 1.0e-4f;		///< 法線と移動方向がほぼ平行かどうかの判定用
	static constexpr float skinWidth = 1.0e-3f;			///< 壁から少しだけ離すためのスキン幅

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
		, isGrounded(false)
	{}

	Rigidbody3D::~Rigidbody3D() noexcept
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// Initialize / Dispose
	//-----------------------------------------------------------------------------
	void Rigidbody3D::Initialize()
	{
		// 見た目用 Transform と Collider を取得
		this->visualTransform = this->Owner()->GetComponent<Transform>();
		this->collider = this->Owner()->GetComponent<Collider3DComponent>();

		// Collider が無ければ追加
		if (!this->collider)
		{
			this->collider = this->Owner()->AddComponent<Collider3DComponent>();
		}

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

		// Body を生成
		this->InitializeBody();
	}

	void Rigidbody3D::Dispose()
	{
		this->DestroyBody();
	}

	//-----------------------------------------------------------------------------
	// 物理シミュレーションステップ
	//-----------------------------------------------------------------------------
	void Rigidbody3D::StepPhysics(float _deltaTime)
	{
		// 物理更新は時間スケールを適用させ、自前の押し戻し結果を visualTransform に反映させる
		float scaledDelta = this->Owner()->TimeScale()->ApplyTimeScale(_deltaTime);
		this->UpdateLogical(scaledDelta);

		for (size_t i = 0; i < Rigidbody3D::SolveIterations; i++)
		{
			// 貫通解決を行う（既に接地している場合完全に押し戻す）
			// 現状は順番依存で解決される
			this->ResolvePenetration();
		}

		// 衝突解決（CastShape 押し戻し）
		this->ResolveCastShape(_deltaTime);

		// visual に反映させる
		this->SyncToVisual();
	}

	//-----------------------------------------------------------------------------
	// 自前移動（TimeScale 適用済み）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::UpdateLogical(float _deltaTime)
	{
		if (!this->staged) { return; }
		if (_deltaTime <= 0.0f) { return; }

		// 前フレーム姿勢を保存
		*(this->stagedPrev) = *(this->staged);

		// 重力を適用
		if (this->useGravity)
		{
			if (this->isGrounded)
			{
				// 接地中は垂直成分のみリセット
				this->linearVelocity.y = 0.0f;
			}
			else
			{
				this->linearVelocity += this->gravity * _deltaTime;
			}
		}

		// 速度を積分して位置を更新
		this->staged->position += this->linearVelocity * _deltaTime;
	}

	//-----------------------------------------------------------------------------
	// staged → visual
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncToVisual() const
	{
		if (!this->visualTransform || !this->staged) { return; }

		this->visualTransform->SetWorldPosition(this->staged->position);
		this->visualTransform->SetWorldRotation(this->staged->rotation);
		this->visualTransform->SetWorldScale(this->staged->scale);
	}

	//-----------------------------------------------------------------------------
	// visual → Jolt（Kinematic のみ）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncVisualToJolt(float _deltaTime)
	{
		if (!this->hasBody || !this->visualTransform) { return; }

		// Static などは Jolt 側を動かさない
		if (this->motionType != EMotionType::Kinematic) { return; }

		// ロジック側の姿勢を基準にする
		const DX::Vector3 targetPos = this->staged->position;
		const DX::Quaternion targetRot = this->staged->rotation;

		// centerOffset をワールドへ回して COM の位置に変換
		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), targetRot);
		}

		const DX::Vector3 comPos = targetPos + worldOffset;

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.MoveKinematic(
			this->bodyID,
			RVec3(comPos.x, comPos.y, comPos.z),
			Quat(targetRot.x, targetRot.y, targetRot.z, targetRot.w),
			_deltaTime
		);
	}

	//-----------------------------------------------------------------------------
	// Jolt → visual → staged（押し戻し結果を確定）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::SyncJoltToVisual()
	{
		if (!this->hasBody || !this->visualTransform || !this->staged) { return; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return; }

		const Body& body = lock.GetBody();

		DX::Vector3 bodyPos(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);

		DX::Quaternion bodyRot(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);

		// COM からピボット位置へ戻す
		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), bodyRot);
		}

		const DX::Vector3 pivotPos = bodyPos - worldOffset;

		// 見た目とロジックに反映
		this->visualTransform->SetWorldPosition(pivotPos);
		this->visualTransform->SetWorldRotation(bodyRot);

		this->staged->position = pivotPos;
		this->staged->rotation = bodyRot;
	}

	//-----------------------------------------------------------------------------
	// 貫通解決（最も深くめり込んでいるbody1つを押し出す）
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ResolvePenetration()
	{
		if (!this->hasBody || !this->staged || !this->collider) { return; }

		const Shape* shape = this->collider->GetShape().GetPtr();
		if (!shape){ return; }

		// 現在の姿勢から COM 位置を計算する
		const DX::Quaternion rot = this->staged->rotation;

		// centerOffset をワールドへ回して COM の位置に変換する
		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), rot);
		}
		const DX::Vector3 comPos = this->staged->position + worldOffset;

		// Jolt 型へ変換
		const RMat44 transform = RMat44::sRotationTranslation(
			Quat(rot.x, rot.y, rot.z, rot.w),
			RVec3(comPos.x, comPos.y, comPos.z)
		);

		// CollideShape 設定
		CollideShapeSettings settings;
		settings.mBackFaceMode = EBackFaceMode::IgnoreBackFaces;
		settings.mMaxSeparationDistance = 0.0f; // 純粋なめり込みだけを見る

		// 最も深くめり込んでいる面を取得できるようにする
		ClosestHitCollisionCollector<CollideShapeCollector> collector;

		// NarrowPhaseQuery から衝突判定を実行する
		const auto& npq = this->physicsSystem.GetNarrowPhaseQuery();
		const auto& broad = this->physicsSystem.GetBroadPhaseLayerFilter(this->objectLayer);
		const auto& obj = this->physicsSystem.GetObjectLayerFilter(this->objectLayer);
		IgnoreSelfBodyFilter bodyFilter(this->bodyID);

		// 衝突判定を実行する
		npq.CollideShape(
			shape,
			Vec3::sOne(),
			transform,
			settings,
			RVec3::sZero(),
			collector,
			broad,
			obj,
			bodyFilter
		);
		if (!collector.HadHit()){ return; }

		// 最も深い衝突情報を取得
		const CollideShapeResult& hit = collector.mHit;
		if (hit.mPenetrationDepth <= 0.0f){ return; }

		// 押し出し方向を正規化する
		Vec3 axis = hit.mPenetrationAxis;
		axis = axis.Normalized();

		// グローバルの上方向
		const Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

		// 法線がどの軸に一番近いかを調べる
		float absX = abs(axis.GetX());
		float absY = abs(axis.GetY());
		float absZ = abs(axis.GetZ());

		if (absY >= absX && absY >= absZ)
		{
			// 床・天井は純粋に縦押し
			axis = Vec3(0, axis.GetY() >= 0 ? 1 : -1, 0);

			// 接地スナップ（微小水平ズレ抑制）
			this->linearVelocity.x = 0;
			this->linearVelocity.z = 0;
		}

		// 自分（クエリ形状）を外に押し出すので -axis
		const Vec3 depen = -axis * hit.mPenetrationDepth;

		const DX::Vector3 depenWorld(
			depen.GetX(),
			depen.GetY(),
			depen.GetZ()
		);

		// 押し出し後の COM 位置を計算してピボット位置に戻す
		const DX::Vector3 newComPos = comPos + depenWorld;
		this->staged->position = newComPos - worldOffset;
	}

	//-----------------------------------------------------------------------------
	// CastShape 押し戻し解決
	//-----------------------------------------------------------------------------
	void Rigidbody3D::ResolveCastShape(float _deltaTime)
	{
		if (!this->hasBody || !this->staged || !this->stagedPrev || !this->collider) { return; }

		this->isGrounded = false;

		// 移動量
		const DX::Vector3 move = this->linearVelocity * _deltaTime; 

		const float moveLenSq = move.LengthSquared();
		if (moveLenSq <= 0.0f) { return; }

		// 移動方向と長さ
		const float moveLen = std::sqrt(moveLenSq);
		const DX::Vector3 moveDir = move / moveLen;

		// 前フレーム回転を使用しない
		const DX::Quaternion currRot = this->staged->rotation;

		// centerOffset をワールドへ回して COM の位置に変換する
		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), currRot);
		}

		// CastShape 開始点（COM）
		const DX::Vector3 comStart = this->stagedPrev->position + worldOffset;

		// Jolt 型へ変換
		const RVec3 jStart(comStart.x, comStart.y, comStart.z);
		const Vec3  jMove(move.x, move.y, move.z);
		const Quat  jRot(currRot.x, currRot.y, currRot.z, currRot.w);
		const RMat44 jWorld = RMat44::sRotationTranslation(jRot, jStart);

		const Shape* shape = this->collider->GetShape().GetPtr();
		if (!shape) { return; }

		RShapeCast shapeCast(shape, Vec3::sOne(), jWorld, jMove);

		// CastShape 設定
		ShapeCastSettings settings;
		settings.mReturnDeepestPoint = false;
		settings.mBackFaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
		settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;

		// ヒット収集用
		ClosestShapeCastCollector collector;
		const NarrowPhaseQuery& npq = this->physicsSystem.GetNarrowPhaseQuery();
		IgnoreSelfBodyFilter bodyFilter(this->bodyID);

		// レイヤーフィルタ取得
		const auto& broad = this->physicsSystem.GetBroadPhaseLayerFilter(this->objectLayer);
		const auto& obj = this->physicsSystem.GetObjectLayerFilter(this->objectLayer);

		// CastShape 実行
		npq.CastShape(
			shapeCast,
			settings,
			RVec3::sZero(),
			collector,
			broad,
			obj,
			bodyFilter
		);
		if (!collector.hasHit) { return; }

		// ヒット位置までの割合
		float f = std::clamp(collector.hit.mFraction, 0.0f, 1.0f);

		// スキン幅を考慮して押し戻す
		const float advance = moveLen * f - skinWidth;

		// 一次押し戻し（移動方向）
		DX::Vector3 comCorrected = comStart + moveDir * advance;

		// 衝突法線による微調整（skinWidth 分だけ離す）
		DX::Vector3 hitNormal(
			collector.hit.mPenetrationAxis.GetX(),
			collector.hit.mPenetrationAxis.GetY(),
			collector.hit.mPenetrationAxis.GetZ()
		);
		hitNormal.Normalize();
		comCorrected += hitNormal * skinWidth;

		// pivot に戻す
		DX::Vector3 pivotCorrected = comCorrected - worldOffset;

		this->staged->position = pivotCorrected;

		// 接地判定
		this->CheckGrounded();
	}

	//-----------------------------------------------------------------------------
	// 接地判定
	//-----------------------------------------------------------------------------
	void Rigidbody3D::CheckGrounded()
	{
		this->isGrounded = false;

		if (!this->collider || !this->hasBody || !this->staged) { return; }

		const float radius = this->collider->GetCapsuleRadius();
		const float halfH = this->collider->GetCapsuleHalfHeight();
		const float epsilon = 0.05f;

		// ピボット位置から足元付近にレイを飛ばす
		const DX::Vector3 worldPos = this->staged->position;
		const float rayStartY = worldPos.y - (halfH + radius * 0.5f);

		const RVec3 from(worldPos.x, rayStartY, worldPos.z);
		const RVec3 to(worldPos.x, rayStartY - (radius + epsilon), worldPos.z);

		const RRayCast ray(from, to - from);

		RayCastResult hit;
		if (this->physicsSystem.GetNarrowPhaseQuery().CastRay(ray, hit))
		{
			this->isGrounded = true;
		}
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
		if (!this->staged) { return; }
		this->staged->position = _worldPos;
	}

	void Rigidbody3D::SetLogicalRotation(const DX::Quaternion& _worldRot)
	{
		if (!this->staged) { return; }
		this->staged->rotation = _worldRot;
	}

	void Rigidbody3D::TranslateWorld(const DX::Vector3& _delta)
	{
		if (!this->staged) { return; }
		this->staged->position += _delta;
	}

	void Rigidbody3D::TranslateLocal(const DX::Vector3& _delta)
	{
		if (!this->staged) { return; }

		const DX::Quaternion& worldRot = this->staged->rotation;

		const DX::Vector3 forward = DX::Vector3::Transform(DX::Vector3::Forward, worldRot);
		const DX::Vector3 right = DX::Vector3::Transform(DX::Vector3::Right, worldRot);
		const DX::Vector3 up = DX::Vector3::Transform(DX::Vector3::Up, worldRot);

		const DX::Vector3 worldDelta =
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

	bool Rigidbody3D::GetBodyTransform(DX::Vector3& _outPos, DX::Quaternion& _outRot) const
	{
		if (!this->hasBody) { return false; }

		BodyLockRead lock(this->physicsSystem.GetBodyLockInterface(), this->bodyID);
		if (!lock.Succeeded()) { return false; }

		const Body& body = lock.GetBody();
		_outPos = DX::Vector3(
			static_cast<float>(body.GetPosition().GetX()),
			static_cast<float>(body.GetPosition().GetY()),
			static_cast<float>(body.GetPosition().GetZ())
		);
		_outRot = DX::Quaternion(
			static_cast<float>(body.GetRotation().GetX()),
			static_cast<float>(body.GetRotation().GetY()),
			static_cast<float>(body.GetRotation().GetZ()),
			static_cast<float>(body.GetRotation().GetW())
		);
		return true;
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

		_rot = this->visualTransform->GetWorldRotation();
		const DX::Vector3 pivotPos = this->visualTransform->GetWorldPosition();

		DX::Vector3 worldOffset = DX::Vector3::Zero;
		if (this->collider)
		{
			worldOffset = DX::Vector3::Transform(this->collider->GetCenterOffset(), _rot);
		}

		_pos = pivotPos + worldOffset;
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
			const ShapeRefC shape = this->collider->GetShape();
			_settings.SetShape(shape);
		}
	}

	void Rigidbody3D::ApplyMotionTypeToBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetMotionType(
			this->bodyID,
			this->motionType,
			EActivation::Activate
		);

		bodyInterface.SetMotionQuality(
			this->bodyID,
			(this->motionType == EMotionType::Kinematic)
			? EMotionQuality::LinearCast
			: EMotionQuality::Discrete
		);
	}

	void Rigidbody3D::ApplyObjectLayerToBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.SetObjectLayer(this->bodyID, this->objectLayer);
	}

	//-----------------------------------------------------------------------------
	// Body create / destroy
	//-----------------------------------------------------------------------------
	void Rigidbody3D::InitializeBody()
	{
		if (this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();

		BodyCreationSettings settings;
		this->SetupBodySettings(settings);

		Body* body = bodyInterface.CreateBody(settings);
		if (!body) { return; }

		bodyInterface.AddBody(body->GetID(), EActivation::Activate);

		this->bodyID = body->GetID();
		this->hasBody = true;
	}

	void Rigidbody3D::DestroyBody()
	{
		if (!this->hasBody) { return; }

		auto& bodyInterface = this->physicsSystem.GetBodyInterface();
		bodyInterface.RemoveBody(this->bodyID);
		bodyInterface.DestroyBody(this->bodyID);

		this->hasBody = false;
	}
} // namespace Framework::Physics