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

	/** @class Rigidbody3D
	 *  @brief TimeScale 対応・自前移動＋Jolt 押し戻しのための 3D ボディコンポーネント
	 *  @details
	 *          - Transform 主導で移動量を決定し、Jolt には「押し戻し判定のみ」を担当させる
	 *          - TimeScale は GameObject 側で適用済みの deltaTime を受け取り、自前で積分する
	 *          - 押し戻し結果は visualTransform と stagedTransform の両方に反映し、見た目とロジックをそろえる
	 *          - MotionType は Static / Kinematic のみを扱う
	 */
	class Rigidbody3D final : public Component
	{
	public:
		/** @brief コンストラクタ
		 *  @param _owner このコンポーネントが所属する GameObject
		 *  @param _active コンポーネントの有効 / 無効
		 */
		Rigidbody3D(GameObject* _owner, bool _active = true);

		/// @brief デストラクタ
		~Rigidbody3D() noexcept override;

		/// @brief 初期化処理
		void Initialize() override;

		/// @brief 終了処理
		void Dispose() override;

		/// @brief Body を生成する（多重生成は行わない）
		void InitializeBody();

		/// @brief Body を破棄する
		void DestroyBody();

		/** @brief ロジック側の姿勢を更新する（TimeScale 適用済み）
		 *  @param _deltaTime 経過時間（秒）
		 *  @details
		 *          - 線形速度と重力を積分して stagedTransform.position を更新する
		 *          - GameObjectManager::BeginPhysics から毎固定フレーム呼び出される想定
		 */
		void UpdateLogical(float _deltaTime);

		/// @brief stagedTransform から visualTransform に姿勢を反映する
		void SyncToVisual() const;

		/** @brief visualTransform から Jolt の Body へ姿勢を反映する
		 *  @param _deltaTime 物理ステップ時間（固定フレーム時間）
		 *  @details
		 *          - MotionType が Kinematic の場合のみ MoveKinematic() を呼ぶ
		 *          - Static の場合は何も行わない（静的ボディは動かさない）
		 */
		void SyncVisualToJolt(float _deltaTime);

		/** @brief Jolt の押し戻し結果を visual / staged 双方へ反映する
		 *  @details
		 *          - Body の位置・回転を Transform に書き込み
		 *          - 同じ値を stagedTransform にも書き戻してロジック座標をそろえる
		 */
		void SyncJoltToVisual();

		//-------------------------------------------------------------------------
		// ロジック座標（stagedTransform）アクセス
		//-------------------------------------------------------------------------

		/// @brief ロジック側ワールド位置を取得する
		DX::Vector3 GetLogicalPosition() const;

		/// @brief ロジック側ワールド回転を取得する
		DX::Quaternion GetLogicalRotation() const;

		/** @brief ロジック側ワールド位置を設定する
		 *  @param _worldPos 新しいワールド位置
		 */
		void SetLogicalPosition(const DX::Vector3& _worldPos);

		/** @brief ロジック側ワールド回転を設定する
		 *  @param _worldRot 新しいワールド回転
		 */
		void SetLogicalRotation(const DX::Quaternion& _worldRot);

		/** @brief ワールド座標系で平行移動する
		 *  @param _delta 移動量（ワールド座標）
		 */
		void TranslateWorld(const DX::Vector3& _delta);

		/** @brief ローカル座標系で平行移動する
		 *  @param _delta 移動量（ローカル座標）
		 *  @details
		 *          - 現在のロジック回転を基準に、前方・右・上方向へ変換して移動する
		 */
		void TranslateLocal(const DX::Vector3& _delta);

		//-------------------------------------------------------------------------
		// 線形速度・重力
		//-------------------------------------------------------------------------

		/// @brief 線形速度を取得する
		DX::Vector3 GetLinearVelocity() const { return this->linearVelocity; }

		/** @brief 線形速度を設定する
		 *  @param _velocity 新しい線形速度
		 */
		void SetLinearVelocity(const DX::Vector3& _velocity);

		/** @brief 線形速度を加算する
		 *  @param _deltaVelocity 追加する速度
		 */
		void AddLinearVelocity(const DX::Vector3& _deltaVelocity);

		/// @brief 重力を適用するかどうかを設定する
		void SetUseGravity(bool _enable) { this->useGravity = _enable; }

		/// @brief 重力が有効かどうかを取得する
		bool IsUsingGravity() const { return this->useGravity; }

		/// @brief 重力加速度ベクトルを設定する
		void SetGravity(const DX::Vector3& _gravity) { this->gravity = _gravity; }

		/// @brief 重力加速度ベクトルを取得する
		DX::Vector3 GetGravity() const { return this->gravity; }

		//-------------------------------------------------------------------------
		// MotionType / ObjectLayer 設定
		//-------------------------------------------------------------------------

		/// @brief MotionType を Static に設定する（Body 生成済みなら即反映）
		void SetMotionTypeStatic();

		/// @brief MotionType を Kinematic に設定する（Body 生成済みなら即反映）
		void SetMotionTypeKinematic();

		/// @brief 現在の MotionType を取得する
		JPH::EMotionType GetMotionType() const { return this->motionType; }

		/// @brief ObjectLayer を Static に設定する（Body 生成済みなら即反映）
		void SetObjectLayerStatic();

		/// @brief ObjectLayer を Kinematic に設定する（Body 生成済みなら即反映）
		void SetObjectLayerKinematic();

		/// @brief 現在の ObjectLayer を取得する
		JPH::ObjectLayer GetObjectLayer() const { return this->objectLayer; }

		//-------------------------------------------------------------------------
		// Body 情報
		//-------------------------------------------------------------------------

		/// @brief Body を保持しているかどうか
		bool HasBody() const { return this->hasBody; }

		/// @brief BodyID を取得する
		const JPH::BodyID& GetBodyID() const { return this->bodyID; }

		/** @brief Body のワールド Transform を取得する
		 *  @param outPos 位置の出力
		 *  @param outRot 回転の出力
		 *  @return 取得に成功したかどうか（Body 未生成時は false）
		 */
		bool GetBodyTransform(DX::Vector3& outPos, DX::Quaternion& outRot) const;

	private:
		/** @brief 初期 Transform を取得する
		 *  @param _pos 初期座標の出力
		 *  @param _rot 初期回転の出力
		 */
		void GetInitialTransform(DX::Vector3& _pos, DX::Quaternion& _rot) const;

		/** @brief BodyCreationSettings を初期化する
		 *  @param _settings 生成設定（motionType / layer / shape 等）
		 */
		void SetupBodySettings(JPH::BodyCreationSettings& _settings) const;

		/// @brief 既存 Body に対して MotionType の変更を反映する
		void ApplyMotionTypeToBody();

		/// @brief 既存 Body に対して ObjectLayer の変更を反映する
		void ApplyObjectLayerToBody();

	private:
		JPH::BodyID           bodyID;       ///< 剛体 ID
		bool                  hasBody;      ///< Body を保持しているかどうか

		JPH::EMotionType      motionType;   ///< MotionType（Static / Kinematic のみ）
		JPH::ObjectLayer      objectLayer;  ///< ObjectLayer（PhysicsLayer::Static / Kinematic）

		std::unique_ptr<StagedTransform> staged;     ///< ロジック用ワールド姿勢
		std::unique_ptr<StagedTransform> stagedPrev; ///< 前フレームのロジック姿勢

		Transform* visualTransform;       ///< 見た目用 Transform
		PhysicsSystem& physicsSystem;         ///< 物理システム参照
		Collider3DComponent* collider;              ///< コライダーコンポーネント

		DX::Vector3           linearVelocity;        ///< 線形速度
		DX::Vector3           gravity;               ///< 重力加速度
		bool                  useGravity;            ///< 重力を適用するかどうか
	};

} // namespace Framework::Physics