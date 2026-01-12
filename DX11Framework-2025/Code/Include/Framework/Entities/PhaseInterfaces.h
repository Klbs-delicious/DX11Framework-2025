/**	@file	PhaseInterfaces.h
 *	@brief	更新、描画フェーズをまとめた抽象群
 *	@date	2025/09/16
 */
#pragma once
namespace Framework {
	namespace Physics {
		class Collider3DComponent;
	}
}

 /** @class  IUpdatable
  *  @brief  更新フェーズ抽象化クラス
  *  @details - 毎フレーム呼び出される更新処理を定義するためのインターフェース
  */
class IUpdatable
{
public:
	/// @brief デストラクタ
	virtual ~IUpdatable() = default;

	/** @brief  更新処理
	 *  @param float _deltaTime	前フレームからの経過時間（秒）
	 */
	virtual void Update(float _deltaTime) = 0;
};

class IFixedUpdatable
{
public:
	/// @brief デストラクタ
	virtual ~IFixedUpdatable() = default;

	/** @brief 固定更新処理
	 *  @param float _deltaTime	前フレームからの経過時間（秒）
	 */
	virtual void FixedUpdate(float _deltaTime) = 0;
};

/** @class	IDrawable
 *	@brief	描画フェーズ抽象化クラス
 *	@details	- 毎フレーム呼び出される描画処理を定義するためのインターフェース
 */
class IDrawable
{
public:
	/// @brief デストラクタ
	virtual ~IDrawable() = default;

	/// @brief 描画処理
	virtual void Draw() = 0;
};

/** @class	BaseColliderDispatcher3D
 *	@brief	3Dコライダーの衝突イベントディスパッチャー抽象化クラス
 *	@details	- コライダーの衝突イベントを受け取るためのインターフェース
 */
class BaseColliderDispatcher3D
{
public:
	/// @brief デストラクタ
	virtual ~BaseColliderDispatcher3D() = default;

	virtual void OnTriggerEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other){}
	virtual void OnTriggerStay(Framework::Physics::Collider3DComponent* _self,Framework::Physics::Collider3DComponent* _other){}
	virtual void OnTriggerExit(Framework::Physics::Collider3DComponent* _self,Framework::Physics::Collider3DComponent* _other){}

	virtual void OnCollisionEnter(Framework::Physics::Collider3DComponent* _self,Framework::Physics::Collider3DComponent* _other){}
	virtual void OnCollisionStay(Framework::Physics::Collider3DComponent* _self,Framework::Physics::Collider3DComponent* _other){}
	virtual void OnCollisionExit(Framework::Physics::Collider3DComponent* _self,Framework::Physics::Collider3DComponent* _other){}
};