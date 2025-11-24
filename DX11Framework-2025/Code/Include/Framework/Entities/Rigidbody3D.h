/** @file   Rigidbody3D.h
 *  @brief  TimeScale 対応 3D リジッドボディコンポーネント（物理は実時間）
 *  @date   2025/11/26
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/TimeScaleComponent.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Framework::Physics
{
	class PhysicsSystem;

	/** @class Rigidbody3D
	 *  @brief Jolt Physics を使用した 3D 剛体コンポーネント
	 *  @details
	 *          - 物理計算は Jolt が実時間で実施する
	 *          - Transform へは TimeScale を掛けた scaledVelocity を適用する
	 *          - 個別 TimeScaleComponent に完全対応
	 */
	class Rigidbody3D final : public Component
	{
	public:
		/** @brief コンストラクタ
		 *  @param GameObject* _owner 所有オブジェクト
		 *  @param bool _active 有効/無効
		 */
		Rigidbody3D(GameObject* _owner, bool _active = true);

		/// @brief デストラクタ
		~Rigidbody3D() noexcept override;

		/// @brief 初期化
		void Initialize() override;

		/// @brief 破棄処理
		void Dispose() override;

		/// @brief Body の生成
		void InitializeBody();

		/// @brief Body の破棄
		void DestroyBody();

		/** @brief 物理結果を Transform に適用する（TimeScaleVelocity 使用）
		 *  @param float _fixedDelta 固定デルタ
		 */
		void ApplyPhysicsResults(float _fixedDelta);

		//-------------------------------------------------------------------------
		// パラメータ設定
		//-------------------------------------------------------------------------
		/** @brief 質量設定
		 *  @param float _mass 質量
		 */
		void SetMass(float _mass);

		/** @brief MotionType 設定
		 *  @param JPH::EMotionType _type モーションタイプ
		 */
		void SetMotionType(JPH::EMotionType _type);

		/** @brief 摩擦係数設定
		 *  @param float _f 摩擦係数
		 */
		void SetFriction(float _f);

		/** @brief 反発係数設定
		 *  @param float _r 反発係数
		 */
		void SetRestitution(float _r);

		/** @brief 重力倍率設定
		 *  @param float _scale 重力倍率
		 */
		void SetGravityScale(float _scale);

		/// @brief 質量取得
		float GetMass() const { return this->mass; }

		/// @brief 摩擦係数取得
		float GetFriction() const { return this->friction; }

		/// @brief 反発係数取得
		float GetRestitution() const { return this->restitution; }

		/// @brief 重力倍率取得
		float GetGravityScale() const { return this->gravityScale; }

		/// @brief MotionType 取得
		JPH::EMotionType GetMotionType() const { return this->motionType; }

		//-------------------------------------------------------------------------
		// 速度・力
		//-------------------------------------------------------------------------
		/** @brief 線形速度設定
		 *  @param const DX::Vector3& _vel 設定速度
		 */
		void SetLinearVelocity(const DX::Vector3& _vel);

		/// @brief 線形速度取得（realVelocity）
		DX::Vector3 GetLinearVelocity() const;

		/** @brief 力を加える
		 *  @param const DX::Vector3& _force 加える力
		 */
		void AddForce(const DX::Vector3& _force);

		/** @brief インパルスを加える
		 *  @param const DX::Vector3& _impulse 加えるインパルス
		 */
		void AddImpulse(const DX::Vector3& _impulse);

		//-------------------------------------------------------------------------
		// 状態
		//-------------------------------------------------------------------------
		/// @brief Body を保持しているか
		bool HasBody() const { return this->hasBody; }

		/// @brief BodyID を取得
		const JPH::BodyID& GetBodyID() const { return this->bodyID; }

	private:
		void GetInitialTransform(DX::Vector3& _outPos, DX::Quaternion& _outRot) const;
		void SetupBodyCreationSettings(JPH::BodyCreationSettings& _settings) const;

	private:
		//-------------------------------------------------------------
		// Jolt Body
		//-------------------------------------------------------------
		JPH::BodyID      bodyID;      ///< BodyID
		bool             hasBody;     ///< Body を持つか

		//-------------------------------------------------------------
		// パラメータ
		//-------------------------------------------------------------
		float            mass;         ///< 質量
		float            friction;     ///< 摩擦
		float            restitution;  ///< 反発
		float            gravityScale; ///< 重力倍率
		JPH::EMotionType motionType;   ///< MotionType

		//-------------------------------------------------------------
		// 必須参照
		//-------------------------------------------------------------
		Transform* transform;      ///< Transform
		TimeScaleComponent* timeScaleComp;  ///< TimeScale
		Collider3DComponent* collider;      ///< コライダー
		PhysicsSystem& physicsSystem; ///< 物理システム

		//-------------------------------------------------------------
		// TimeScale 用速度
		//-------------------------------------------------------------
		DX::Vector3 scaledVelocity; ///< realVelocity * TimeScale
	};
} // namespace Framework::Physics
