/** @file   Collider3DComponent.h
 *  @brief  3D の当たり判定を扱うコライダーコンポーネント
 *  @date   2025/11/17
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <Include/Framework/Utils/CommonTypes.h>
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/Transform.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Body/BodyFilter.h>

namespace Framework::Physics
{
	//-----------------------------------------------------------------------------
	// IgnoreSelfBodyFilter
	//-----------------------------------------------------------------------------

	/** @class IgnoreSelfBodyFilter
	 *  @brief 自身の Body を無視するフィルタ
	 *  @details
	 *      - ShapeCast / RayCast 時に「自分自身と衝突しない」ためのフィルタ
	 */
	class IgnoreSelfBodyFilter : public JPH::BodyFilter
	{
	public:
		/** @brief コンストラクタ
		 *  @param _id 自身の BodyID
		 */
		IgnoreSelfBodyFilter(JPH::BodyID _id) : self(_id) {}

		/**@brief 衝突すべきかどうか
		 * @param _bodyID 判定対象の BodyID
		 * @return
		 */
		bool ShouldCollide(const JPH::BodyID& _bodyID) const override
		{
			return _bodyID != self;
		}
	public:
		JPH::BodyID self;	///< 無視したい Body の ID
	};

	//-----------------------------------------------------------------------------
	// ClosestShapeCastCollector
	//-----------------------------------------------------------------------------

	/** @class ClosestShapeCastCollector
	 *  @brief 最も近い ShapeCast ヒットだけを収集する
	 *  @details
	 *      - JPH::CastShapeCollector を継承
	 *      - AddHit をオーバーライドし、fraction が最小のものを保持する
	 */
	class ClosestShapeCastCollector : public JPH::CastShapeCollector
	{
	public:
		/// @brief ヒット時コールバック
		void AddHit(const JPH::ShapeCastResult& _result) override
		{
			if (_result.mFraction < GetEarlyOutFraction())
			{
				hit = _result;
				hasHit = true;

				// Early out fraction を更新することで、
				// より遠いヒットを無視できる（最適化）
				UpdateEarlyOutFraction(_result.mFraction);
			}
		}
	public:
		bool hasHit = false;				///< ヒットがあったか
		JPH::ShapeCastResult hit;			///< 最も近いヒット結果
	};

	//-----------------------------------------------------------------------------
	// ColliderShapeType
	//-----------------------------------------------------------------------------

	/** @enum ColliderShapeType
	 *  @brief コライダー形状の種類
	 */
	enum class ColliderShapeType
	{
		Box,
		Sphere,
		Capsule,
		Mesh,
		Max
	};

	//-----------------------------------------------------------------------------
	// Collider3DComponent
	//-----------------------------------------------------------------------------

	/** @class Collider3DComponent
	 *  @brief 3D当たり判定を管理するコンポーネント
	 *	@details
	 *		- Jolt Physics の Shape を使用して形状を管理する
	 *		- 基本的に静的データ型であり、動的に変更する場合は再度 BuildShape を呼び出す必要がある
	 */
	class Collider3DComponent : public Component
	{
	public:
		/** @brief コンストラクタ
		 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
		 *  @param bool _active コンポーネントの有効/無効
		 */
		Collider3DComponent(GameObject* _owner, bool _active = true);

		/// @brief デストラクタ
		~Collider3DComponent() noexcept override = default;

		/// @brief 初期化処理
		void Initialize() override;

		/// @brief 終了処理
		void Dispose() override;

		/** @brief コライダー形状の種類を設定
		 *  @param _shapeType 設定する形状タイプ
		 */
		void SetShape(ColliderShapeType _shapeType);

		/** @brief ボックス形状の半分の大きさを設定する
		 *  @param _half 半分の大きさ
		 */
		void SetBoxHalfExtent(const DX::Vector3& _half);

		/** @brief 球形状の半径を設定する
		 *  @param _radius 半径
		 */
		void SetSphereRadius(float _radius);

		/** @brief カプセル形状の半径と半分の高さを設定する
		 *  @param _radius 半径：デフォルト値0.5f（半径0.5f）
		 *  @param _halfHeight 半分の高さ：デフォルト値0.5f（全高1.0f）
		 *	@details
		 *		-	カプセルの高さは中央から両端までの距離の半分を指定する
		 */
		void SetCapsule(float _radius, float _halfHeight);

		/** @brief 形状の中心オフセットを設定する
		 *  @param _offset オフセット値
		 */
		void SetCenterOffset(const DX::Vector3& _offset);

		/** @brief トリガー設定を行う
		 *  @param _isTrigger トリガーにする場合は true
		 */
		void SetisTrigger(bool _isTrigger);

		/** @brief コライダーIDを設定する
		 *  @param _id 設定するID
		 */
		void SetColliderID(int _id);

		/** @brief コライダーIDを取得する
		 *  @return コライダーID
		 */
		[[nodiscard]] int GetColliderID() const;

		/// @brief 設定値からShapeSettingsを作成する
		void BuildShapeSettings();

		/** @brief	Shapeの生成
		 *	@details
		 *		-	BuildShapeSettingsで作成した設定からShapeを生成する
		 *		-	Rigidbody側で実際のShapeを作る時に使う
		 */
		void CreateShape();

		/** @brief 形状参照を取得する
		 *  @return 形状参照
		 */
		[[nodiscard]] JPH::ShapeRefC GetShape() const;

		/** @brief コライダー形状の種類を取得する
		 *  @return 形状参照
		 */
		[[nodiscard]] ColliderShapeType& GetShapeType();

		[[nodiscard]] DX::Vector3& GetBoxHalfExtent();

		[[nodiscard]] float& GetSphereRadius();
		[[nodiscard]] float& GetCapsuleRadius();
		[[nodiscard]] float& GetCapsuleHalfHeight();
		[[nodiscard]] DX::Vector3& GetCenterOffset();
		[[nodiscard]] bool IsTrigger() const;
		[[nodiscard]] JPH::Ref<JPH::ShapeSettings> GetShapeSettings() const;

	private:
		ColliderShapeType	shapeType;	///< コライダーの種類
		JPH::ShapeRefC		shape;		///< Jolt 形状の参照
		Transform*			transform;	///< 対象オブジェクトの変換

		JPH::Ref<JPH::ShapeSettings>	shapeSettings;	///< BuildShapeで作る設定キャッシュ

		// 設定値（BuildShape 用）
		DX::Vector3 boxHalfExtent;		///< ボックス形状の半分の大きさ
		float		sphereRadius;		///< 球形状の半径
		float		capsuleRadius;		///< カプセル形状の半径
		float		capsuleHalfHeight;	///< カプセル形状の半分の高さ
		DX::Vector3 centerOffset;		///< 形状の中心オフセット

		int			colliderID;			///< コライダーID（未割当は -1）
		bool		isTrigger;			///< true: 衝突判定のみ（押し出しは行わない）
	};
}
