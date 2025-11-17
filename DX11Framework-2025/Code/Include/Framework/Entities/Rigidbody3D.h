/** @file   Rigidbody3D.h
 *  @brief  Jolt Physics 用 3D リジッドボディコンポーネント
 *  @date   2025/11/17
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Framework::Physics
{
	class PhysicsSystem;
	class Collider3DComponent;

	/** @class  Rigidbody3D
	 *  @brief  Jolt の Body をラップする 3D 物理コンポーネント
	 */
	class Rigidbody3D final : public Component
	{
	public:
		/** @brief コンストラクタ
		 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
		 *  @param bool _isActive コンポーネントの有効 / 無効
		 */
		Rigidbody3D(GameObject* _owner, bool _isActive = true);

		/// @brief デストラクタ（Jolt の Body を破棄する）
		~Rigidbody3D() noexcept override;

		/// @brief 物理ボディを生成する（まだ生成されていない場合のみ）
		void InitializeBody();

		/// @brief 物理ボディを明示的に破棄する
		void DestroyBody();

		/** @brief 質量を設定する
		 *  @param float _mass 質量
		 *
		 *  すでに Body が存在する場合は、一度破棄して再生成する。
		 */
		void SetMass(float _mass);

		/// @brief 設定されている質量を取得する
		float GetMass() const { return this->mass; }

		/** @brief 運動タイプを設定する
		 *  @param JPH::EMotionType _motionType Static / Kinematic / Dynamic
		 */
		void SetMotionType(JPH::EMotionType _motionType);

		/// @brief 運動タイプを取得する
		JPH::EMotionType GetMotionType() const { return this->motionType; }

		/** @brief 速度を設定する（Dynamic / Kinematic のみ有効）
		 *  @param const DX::Vector3& _velocity 中心の線形速度（メートル / 秒）
		 */
		void SetLinearVelocity(const DX::Vector3& _velocity);

		/// @brief 現在の線形速度を取得する
		DX::Vector3 GetLinearVelocity() const;

		/** @brief 力を加える（Dynamic のみ有効）
		 *  @param const DX::Vector3& _force 加える力（ニュートン）
		 */
		void AddForce(const DX::Vector3& _force);

		/// @brief Jolt の BodyID を取得する（有効でない場合は無効 ID）
		const JPH::BodyID& GetBodyID() const { return this->bodyID; }

		/// @brief Body が生成済みかどうか
		bool HasBody() const { return this->hasBody; }

	private:
		/// @brief エンジン側 PhysicsSystem を取得するヘルパ
		PhysicsSystem* GetPhysicsSystem();

		/// @brief Transform から Jolt 用の位置・回転を取得するヘルパ
		void GetInitialTransform(DX::Vector3& outPosition, DX::Quaternion& outRotation) const;

		/// @brief Body 生成時の共通設定を行う
		void SetupBodyCreationSettings(JPH::BodyCreationSettings& _settings) const;

	private:
		JPH::BodyID			bodyID;		///< Jolt のボディ ID（無効の場合もある）
		bool				hasBody;	///< 現在 Body が生成されているか
		float				mass;		///< 質量（kg）
		JPH::EMotionType	motionType;	///< 運動タイプ

		Collider3DComponent* collider;	///< 形状を提供するコライダー
	};
}