/** @file EnemyController.h
 *  @date 2026/06/23
 */
#pragma once

#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/AnimationStateMachine.h"
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Game/Entities/InputAdapterAI.h"
#include "Include/Game/Entities/MoveComponent.h"

 /** @brief 敵の行動を制御するコンポーネント
  *  @details
  *  - IAIBehavior と InputAdapterAI を組み合わせて、敵の行動を決定、入力コマンドに変換して CharacterController に渡す役割を持つ
  *  - 敵の状態や環境に応じて適切な行動を選択するためのロジックを実装する
  */
class EnemyController : public Component, public IUpdatable
{
public:
	enum class EnemyAnimState
	{
		Idle = 0,
		Walk,
		Attack,
	};

	EnemyController(GameObject* _owner, bool _isActive = true);
	~EnemyController() override = default;

	void Initialize() override;
	void Dispose() override;
	void Update(float _deltaTime) override;

private:
	/** @brief アニメーション状態が変化した場合にアニメーションをリクエストする
	 *  @param _state 新しいアニメーション状態
	 */
	void RequestAnimationIfChanged(EnemyAnimState _state);

	InputAdapterAI*							inputAdapter;		///< 敵のAI入力を取得
	AttackComponent*						attackComponent;	///< 敵の攻撃を管理
	MoveComponent*							moveComponent;		///< 敵の移動を管理
	AnimationComponent*						animationComponent;	///< 敵のアニメーションを管理
	AnimationStateMachine<EnemyAnimState>*	animStateMachine;	///< 敵のアニメーション状態を管理

	float									walkSpeed;			///< 敵の歩行速度
};