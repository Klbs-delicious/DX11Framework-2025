/** @file   Rigidbody3D.h
 *  @brief  TimeScale 対応・自前移動＋Jolt 押し戻し用 Rigidbody3D コンポーネント
 *  @date   2025/11/28
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include "Include/Framework/Physics/StagedTransform.h"
#include "Include/Framework/Physics/PhysicsLayers.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <memory>

namespace Framework::Physics
{
	class PhysicsSystem;
	enum class ContactType;

	/** @class Rigidbody3D
	 *  @brief Jolt Physics ベースの 3D リジッドボディコンポーネント
	 *  @details
	 *      - Transform とは別に StagedTransform を持ち、自前移動＋押し戻しを行う
	 *      - Kinematic Body として Jolt に同期する
	 */
	class Rigidbody3D : public Component
	{
	public:
		/// @brief コンストラクタ
		Rigidbody3D(GameObject* _owner, bool _active = true);

		/// @brief デストラクタ
		~Rigidbody3D() noexcept override;

		/// @brief 初期化
		void Initialize() override;

		/// @brief 破棄
		void Dispose() override;

        /** @brief 物理シミュレーションステップ
         *  @param _deltaTime 経過時間
		 */
        void StepPhysics(float _deltaTime);

		/** @brief 自前移動（TimeScale 適用済み）
		 *  @param _deltaTime 経過時間
		 */
		void UpdateLogical(float _deltaTime);

		/// @brief staged → visual の同期
		void SyncToVisual() const;

		/** @brief visual → Jolt（Kinematic Body）へ同期
		 *  @param _deltaTime 経過時間
		 */
		void SyncVisualToJolt(float _deltaTime);

		/// @brief Jolt → visual/staged へ姿勢を反映
		void SyncJoltToVisual();

		/// @brief 貫通解決
		void ResolvePenetration();

		/** @brief CastShape による押し戻し解決
		 *  @param _deltaTime 経過時間
		 */
		void ResolveCastShape(float _deltaTime);

		/// @brief 論理位置取得
		DX::Vector3 GetLogicalPosition() const;

		/// @brief 論理回転取得
		DX::Quaternion GetLogicalRotation() const;

		/** @brief 論理位置の設定
		 *  @param _worldPos ワールド位置
		 */
		void SetLogicalPosition(const DX::Vector3& _worldPos);

		/** @brief 論理回転の設定
		 *  @param _worldRot ワールド回転
		 */
		void SetLogicalRotation(const DX::Quaternion& _worldRot);

		/** @brief ワールド座標系で移動
		 *  @param _delta 移動量
		 */
		void TranslateWorld(const DX::Vector3& _delta);

		/** @brief ローカル座標系で移動
		 *  @param _delta 移動量
		 */
		void TranslateLocal(const DX::Vector3& _delta);

		/** @brief 線形速度の設定
		 *  @param _velocity 速度
		 */
		void SetLinearVelocity(const DX::Vector3& _velocity);

		/** @brief 線形速度の加算
		 *  @param _deltaVelocity 追加速度
		 */
		void AddLinearVelocity(const DX::Vector3& _deltaVelocity);

		DX::Vector3 GetLinearVelocity() const;

		/// @brief 重力の有効/無効設定
		void SetUseGravity(bool _use) { this->useGravity = _use; }

		/// @brief 重力が有効か
		bool IsUsingGravity() const { return this->useGravity; }

		/** @brief 重力ベクトルを設定
		 *  @param _gravity 重力ベクトル
		 */
		void SetGravity(const DX::Vector3& _gravity) { this->gravity = _gravity; }

		/// @brief 現在の重力ベクトルを取得
		DX::Vector3 GetGravity() const { return this->gravity; }

		/// @brief 接地しているか（RayCast による）
		bool IsGrounded() const { return this->isGrounded; }

		/// @brief MotionType を Static に設定
		void SetMotionTypeStatic();

		/// @brief MotionType を Kinematic に設定
		void SetMotionTypeKinematic();

		/** @brief ObjectLayer を設定
		 *  @param _layer 設定するレイヤー
		 */
		void SetObjectLayer(JPH::ObjectLayer _layer);

		/** @brief Jolt Body の姿勢を取得
		 *  @param _outPos ワールド位置
		 *  @param _outRot ワールド回転
		 *  @return 成功なら true
		 */
		bool GetBodyTransform(DX::Vector3& _outPos, DX::Quaternion& _outRot) const;

		/** @brief 接触イベントのディスパッチ
		 *  @param _type イベント種別
		 *  @param _selfCollider 自分側のコライダー
		 *  @param _otherColl 相手側のコライダー
		 */
		void DispatchContactEvent(const ContactType& _type, Collider3DComponent* _selfCollider, Collider3DComponent* _otherColl);

		static constexpr int SolveIterations = 3;   ///< 貫通解決の反復回数

	private:
		/** @brief 初期姿勢の取得
		 *  @param _pos 初期位置（COM）
		 *  @param _rot 初期回転
		 */
		void GetInitialTransform(DX::Vector3& _pos, DX::Quaternion& _rot) const;

		/** @brief BodyCreationSettings へ値をセット
		 *  @param _settings 設定
		 */
		void SetupBodySettings(JPH::BodyCreationSettings& _settings) const;

		/// @brief MotionType を Body に適用
		void ApplyMotionTypeToBody();

		/// @brief ObjectLayer を Body に適用
		void ApplyObjectLayerToBody();

		/// @brief Body を生成
		void InitializeBody();

		/// @brief Body を破棄
		void DestroyBody();

		/// @brief 接地判定（RayCast）
		void CheckGrounded();

		/// @brief 複数ColliderのcenterOffsetを回転適用して平均COMオフセットを計算
		DX::Vector3 ComputeCombinedOffset(const DX::Quaternion& _rot) const;

		/** @brief 自身の階層下から Body に属する Collider を収集する
		 *  @details 子階層も含めて取得し、別 Rigidbody を持つ子の Collider は除外する
		 */
		void CollectColliders();

	private:
		JPH::BodyID bodyID;                         ///< Jolt の BodyID
		bool hasBody;                               ///< Body を保持しているか

		JPH::EMotionType motionType;                ///< 動作モード（Static / Kinematic）
		JPH::ObjectLayer objectLayer;               ///< コリジョンレイヤ

		std::unique_ptr<StagedTransform> staged;        ///< 論理姿勢（更新中の位置）
		std::unique_ptr<StagedTransform> stagedPrev;    ///< 前フレームの論理姿勢

		Transform* visualTransform;                     ///< Transform（見た目用）
		PhysicsSystem& physicsSystem;                   ///< Jolt 物理システム
		std::vector<Collider3DComponent*> colliders;    ///< 自身の階層下に存在するコライダー形状

		DX::Vector3 linearVelocity;                 ///< 線形速度
		DX::Vector3 gravity;                        ///< 重力ベクトル
		bool useGravity;                            ///< 重力使用フラグ

		bool isGrounded;                            ///< 接地フラグ
	};
} // namespace Framework::Physics