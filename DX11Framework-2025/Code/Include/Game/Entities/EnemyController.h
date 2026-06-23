/**@file EnemyController.h
 * @date 2026/06/23
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Game/Entities/InputAdapterAI.h"
#include "Include/Game/Entities/MoveComponent.h"
#include "Include/Game/Entities/AttackComponent.h"

 //-----------------------------------------------------------------------------

/** @brief 敵の行動を制御するコンポーネント
 *  @details
 *  - IAIBehavior と InputAdapterAI を組み合わせて、敵の行動を決定、入力コマンドに変換して CharacterController に渡す役割を持つ
 *  - 敵の状態や環境に応じて適切な行動を選択するためのロジックを実装する
 */
class EnemyController : public Component, public IUpdatable
{
public:
	EnemyController(GameObject* _owner, bool _isActive = true);
	~EnemyController() override = default;

	void Initialize() override;
	void Dispose() override;

	void Update(float _deltaTime) override;

private:
	InputAdapterAI*		inputAdapter;		///< AIの入力を中継するコンポーネント
	AttackComponent*	attackComponent;	///< 攻撃処理
	MoveComponent*		moveComponent;		///< 移動処理

	float				walkSpeed;			///< 歩行速度
};