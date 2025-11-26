/** @file   Rigidbody3D.h
 *  @brief  Transform を基準とした衝突判定専用ボディコンポーネント
 *  @date   2025/11/26
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include "Include/Framework/Physics/StagedTransform.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Framework::Physics
{
	class PhysicsSystem;

	/** @class Rigidbody3D
	 *  @brief Transform 主導の Kinematic 衝突専用ボディ
	 *  @details
	 *          - 移動・回転などの運動はすべて Transform（StagedTransform）が決定
	 *          - Jolt は MoveKinematic 相当の衝突判定と押し戻しのみ担当
	 *          - 毎フレーム visual → Jolt、そして押し戻し結果を visual に戻す
	 */
	class Rigidbody3D final : public Component
	{
	public:
		/** @brief コンストラクタ
		 *  @param _owner このコンポーネントが所属するオブジェクト
		 *  @param _active 有効/無効状態
		 */
		Rigidbody3D(GameObject* _owner, bool _active = true);

		/// @brief デストラクタ
		~Rigidbody3D() noexcept override;

		/// @brief 初期化
		void Initialize() override;

		/// @brief 終了処理
		void Dispose() override;

		/// @brief Body を生成する（多重生成は行わない）
		void InitializeBody();

		/// @brief Body を破棄する
		void DestroyBody();

		/** @brief 論理変換（StagedTransform）を更新する
		 *  @param _deltaTime 経過時間（秒）
		 */
		void UpdateLogical(float _deltaTime);

		/// @brief StagedTransform → visualTransform へ反映する
		void SyncToVisual() const;

		/// @brief visualTransform → Jolt Body へ反映する
		void SyncVisualToJolt() const;

		/// @brief Jolt の押し戻し結果 → visualTransform へ反映する
		void SyncJoltToVisual() const;

		/// @brief Body を保持しているか
		bool HasBody() const { return this->hasBody; }

		/// @brief BodyID を取得する
		const JPH::BodyID& GetBodyID() const { return this->bodyID; }

	private:
		/** @brief 初期 Transform を取得する
		 *  @param _pos 初期座標の出力
		 *  @param _rot 初期回転の出力
		 */
		void GetInitialTransform(DX::Vector3& _pos, DX::Quaternion& _rot) const;

		/** @brief BodyCreationSettings を初期化する
		 *  @param _settings 生成設定（motionType/shape など）
		 */
		void SetupBodySettings(JPH::BodyCreationSettings& _settings) const;

	private:
		JPH::BodyID bodyID;          ///< 剛体 ID
		bool        hasBody;         ///< Body を保持しているか

		std::unique_ptr<StagedTransform> stagedTransform; ///< 論理姿勢
		Transform* visualTransform;  ///< 見た目の Transform

		PhysicsSystem& physicsSystem; ///< 物理システム
		Collider3DComponent* collider;      ///< 衝突形状
	};

} // namespace Framework::Physics