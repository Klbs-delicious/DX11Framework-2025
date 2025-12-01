/** @file   Collider3DComponent.h
 *  @brief  3Dの当たり判定を扱うコライダーコンポーネント
 *  @date   2025/11/17
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/Transform.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace Framework::Physics
{
	/** @enum ColliderShapeType
	 *  @brief コライダー形状タイプ
	 */
	enum class ColliderShapeType
	{
		Box,
		Sphere,
		Capsule,
		Mesh,
		Max
	};

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

		/// @brief 設定値に基づいて形状を生成する
		void BuildShape();

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

	private:
		ColliderShapeType shapeType;      ///< コライダーの種類
		JPH::ShapeRefC shape;             ///< Jolt 形状の参照
		Transform* transform;             ///< 対象オブジェクトの変換

		// 設定値（BuildShape 用）
		DX::Vector3 boxHalfExtent;	///< ボックス形状の半分の大きさ
		float sphereRadius;			///< 球形状の半径
		float capsuleRadius;		///< カプセル形状の半径
		float capsuleHalfHeight;	///< カプセル形状の半分の高さ
		DX::Vector3 centerOffset;	///< 形状の中心オフセット
	};
}