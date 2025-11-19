/** @file   Rigidbody3D.h
 *  @brief  Jolt Physics 用 3D リジッドボディコンポーネント
 *  @date   2025/11/17
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
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
	class Rigidbody3D final : public Component, public IUpdatable
	{
	public:
		/** @brief コンストラクタ
		 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
		 *  @param bool _isActive コンポーネントの有効 / 無効
		 */
		Rigidbody3D(GameObject* _owner, bool _isActive = true);

		/// @brief デストラクタ（Jolt の Body を破棄する）
		~Rigidbody3D() noexcept override;

		/// @brief 初期化処理
		void Initialize() override;

		/// @brief 物理ボディを生成する（まだ生成されていない場合のみ）
		void InitializeBody();

		/** @brief 更新処理
		 *  @param float _deltaTime 経過時間
		 */
		void Update(float _deltaTime) override;

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

		/** @brief インパルスを加える（Dynamic のみ有効）
		 *  @param const DX::Vector3& impulse 加えるインパルス（kg・m/s）
		 */
		void AddImpulse(const DX::Vector3& impulse);

		/// @brief Jolt の BodyID を取得する（有効でない場合は無効 ID）
		const JPH::BodyID& GetBodyID() const { return this->bodyID; }

		/// @brief Body が生成済みかどうか
		bool HasBody() const { return this->hasBody; }

		/** @brief 重力の影響倍率を設定する
		 *  @param _scale 重力倍率（0 で無重力、1 で通常）
		 *  @details
		 *          動的剛体にのみ有効。静的・キネマティックでは効果なし
		 */
		void SetGravityScale(float _scale);

		/** @brief 摩擦係数を設定する
		 *  @param _friction 摩擦（一般に 0～1）
		 *  @details
		 *          衝突時のすべりやすさに影響する。動作中に変更可能
		 */
		void SetFriction(float _friction);

		/** @brief 反発係数を設定する
		 *  @param _restitution 跳ね返り係数（0～1）
		 *  @details
		 *          0 は跳ねない、1 は完全に弾む。動作中に変更可能
		 */
		void SetRestitution(float _restitution);

		/** @brief センサーかどうかを設定する
		 *  @param _isTrigger true なら接触検知のみ行い、物理反応は発生しない
		 *  @details
		 *          Static センサーは軽量。Kinematic センサーは静的物体とも接触を検知できる
		 */
		void SetIsTrigger(bool _isTrigger);

		[[nodiscard]] float GetGravityScale() const { return this->gravityScale; }
		[[nodiscard]] float GetFriction() const { return this->friction; }
		[[nodiscard]] float GetRestitution() const { return this->restitution; }
		[[nodiscard]] bool GetIsTrigger() const { return this->isTrigger; }

	private:
		/**@brief Transform から Jolt 用の位置・回転を取得するヘルパ
		 * @param outPosition
		 * @param outRotation
		 */
		void GetInitialTransform(DX::Vector3& outPosition, DX::Quaternion& outRotation) const;

		/** @brief BodyCreationSettings をセットアップするヘルパ
		 *  @param BodyCreationSettings& _settings 設定オブジェクト
		 */
		void SetupBodyCreationSettings(JPH::BodyCreationSettings& _settings) const;

		/// @brief Transform の位置・回転を物理ボディに同期する
		void SyncTransform();

	private:
		JPH::BodyID		bodyID;			///< Jolt のボディ ID（無効の場合もある）
		bool			hasBody;		///< 現在 Body が生成されているか

		float			mass;			///< 質量（kg）
		float			gravityScale;	///< 重力スケール
		float			friction;		///< 摩擦係数
		float			restitution;	///< 反発係数
		bool			isTrigger;		///< トリガーかどうか

		JPH::EMotionType		motionType;		///< 運動タイプ

		Collider3DComponent*	collider;		///< 形状を提供するコライダー
		Transform*				transform;		///< 所属するオブジェクトの Transform
		PhysicsSystem&			physicsSystem;;	///< 物理システムの参照
	};
}