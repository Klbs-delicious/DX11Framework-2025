/** @file   CharacterController.h
 *  @brief  キャラクターの操作を行う
 *  @date   2025/11/11
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/InputSystem.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/AnimationComponent.h"

#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Game/Entities/MoveComponent.h"
#include "Include/Game/Entities/DodgeComponent.h"

//-----------------------------------------------------------------------------
// CharacterController class
//-----------------------------------------------------------------------------

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
		Jumping,	///< ジャンプ
		Punching,	///< パンチ
	};

	/** @enum	PlayerState
	 *	@brief	プレイヤーの状態
	 */
	enum class PlayerState
	{
		Normal,		///< 通常状態
		Attacking,	///< 攻撃中
		Dodging,	///< 回避中
		Jumping,	///< ジャンプ中
		Countering	///< 追撃中
	};

	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	CharacterController(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	~CharacterController() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/// @brief 状態遷移時の処理
	void StateEnter();

	/** @brief 状態更新時の処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void StateUpdate(float _deltaTime);

	/// @brief 状態離脱時の処理
	void StateExit();

	/** @brief ジャスト回避成功時の処理
	 *  @param _attacker 攻撃者オブジェクト
	 *  @param _type 攻撃タイプ
	 */
	void OnJustDodgeSuccess(GameObject* _attacker, AttackType _type);

	/** @brief  移動速度を設定
	 *  @param _speed 移動速度
	 */
	void SetMoveSpeed(float _speed) { this->moveSpeed = _speed; }

	/** @brief  回転速度を設定
	 *  @param _speed 回転速度
	 */
	void SetTurnSpeed(float _speed) { this->turnSpeed = _speed; }

private:
	/// @brief 回避状態の終了判定（DodgeComponent のみ）
	bool IsDodgeFinished() const;

private:
	InputSystem& inputSystem;					///< 入力処理を管理している
	TimeScaleSystem& timeScaleSystem;			///< タイムスケール処理を管理している

	AnimationComponent* animationComponent = nullptr;	///< アニメーション
	AttackComponent* attackComponent = nullptr;			///< 攻撃処理
	MoveComponent* moveComponent = nullptr;				///< 移動処理
	DodgeComponent* dodgeComponent = nullptr;			///< 回避処理

	Transform* cameraTransform = nullptr;				///< カメラの座標系

	float moveSpeed = 10.0f;							///< 移動速度
	float turnSpeed = 15.0f;							///< 回転速度

	AttackDef currentAttackDef{};						///< 現在の攻撃定義

	PlayerState currentState = PlayerState::Normal;		///< 現在の状態
	PlayerState previousState = PlayerState::Normal;	///< 前の状態

	GameObject* counterTarget = nullptr;				///< 追撃対象
	float counterStopDistance = 1.2f;					///< 追撃停止距離（到達判定）

	bool pendingCounter = false;						///< 回避終了後に追撃へ移る予約
	GameObject* pendingCounterTarget = nullptr;			///< 追撃予約ターゲット
	AttackType pendingCounterType = AttackType::Melee;	///< 追撃予約タイプ

	float counterRemainingSec = 0.0f;					///< 追撃猶予の残り時間
	float counterTimeoutSec = 0.40f;					///< 追撃猶予（適当でOK）
};
