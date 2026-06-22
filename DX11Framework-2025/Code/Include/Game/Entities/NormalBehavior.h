/**@file NormalBehavior.h
 * @date 2026/06/22
 */
#pragma once
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Game/Entities/IAIBehavior.h"

 //-----------------------------------------------------------------------------

/** @brief 通常の敵AIの行動を定義するクラス
 */
class NormalBehavior : public IAIBehavior
{
public:
	NormalBehavior(GameObject* _owner, bool _isActive = true);
	~NormalBehavior() override = default;

	void Initialize() override;
	void Dispose() override;

	/** @brief 敵AIの行動を取得する
	 *  @return 敵AIの行動を表すEnemyDecision構造体
	 */
	virtual EnemyDecision GetDecision() const override;

private:
	/** @brief 敵AIが追跡可能かどうかを判定する
	 *  @return true: 追跡可能、false: 追跡不可能
	 */
	bool CanChase() const;

	GameObject* target;					///< ターゲットのプレイヤーオブジェクト
	EnemyDecisionType currentState;		///< 敵AIの現在の状態

	float attackRange = 2.0f;			///< 攻撃範囲
};