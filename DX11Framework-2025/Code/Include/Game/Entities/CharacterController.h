/** @file   CharacterController.h
 *  @brief  キャラクターの操作を行う
 *  @date   2025/11/11
 */
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Core/InputSystem.h"

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/AnimationComponent.h"

#include "Include/Game/Entities/AttackComponent.h"

 /** @class CharacterController
  *  @brief キャラクターの移動・操作を管理する
  */
class CharacterController : public Component, public IUpdatable
{
public:
	/** @enum	PlayerAnimState
	 *	@brief	プレイヤーのアニメーション状態
	 */
	enum class PlayerAnimState
	{
		Idle,		///< 待機
		Walk,		///< 歩行
		Run,		///< 走行
		Dodging,	///< 回避
		Jumping		///< ジャンプ
	};

	/** @enum	PlayerState
	 *	@brief	プレイヤーの状態
	 */
	enum class PlayerState
	{
		Idle,		///< 待機
		Walk,		///< 歩行
		Run,		///< 走行
		Dodging,	///< 回避
		Jumping		///< ジャンプ
	};

	/** @brief コンストラクタ
	 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param bool _active コンポーネントの有効/無効
	 */
	CharacterController(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~CharacterController() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/** @brief 更新処理
	 *  @param float _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/// @brief 状態遷移時の処理
	void StateEnter();

	/// @brief 状態更新時の処理
	void StateUpdate(float _deltaTime);

	/// @brief 状態離脱時の処理
	void StateExit();

	/**@brief  移動速度を設定
	 * @param _speed 移動速度
	 */
	void SetMoveSpeed(float _speed) { this->moveSpeed = _speed; }

	/**@brief  回転速度を設定
	 * @param _speed 回転速度
	 */
	void SetTurnSpeed(float _speed) { this->turnSpeed = _speed; }

private:
	InputSystem& inputSystem;	///< 入力処理を管理している

	AnimationComponent*					animationComponent;	///< アニメーション
	AttackComponent*					attackComponent;	///< 攻撃処理
	Transform*							cameraTransform;	///< カメラの座標系
	Framework::Physics::Rigidbody3D*	rigidbody;			///< 自身のRigidbody3Dコンポーネント

	float moveSpeed;		///< 移動速度
	float turnSpeed;		///< 回転速度

	AttackDef currentAttackDef;	///< 現在の攻撃定義

	PlayerState currentState;	///< 現在の状態
	PlayerState previousState;	///< 前の状態
};