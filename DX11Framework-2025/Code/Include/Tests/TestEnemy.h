/** @file   TestEnemy.h
 *  @brief  一定間隔で攻撃を出し続けるテスト用コンポーネント
 *  @date   2026/02/14
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

//-----------------------------------------------------------------------------
// TestEnemy class
//-----------------------------------------------------------------------------

class AttackComponent;

/** @class  TestEnemy
 *  @brief  クールタイム毎に AttackComponent を呼び出すだけの最小構成
 */
class TestEnemy : public Component, public IUpdatable
{
public:
	/// @brief エネミーのアニメーション状態
	enum class EnemyAnimState
	{
		Idle = 0,
		Attack,
		Hit,
		Dead
	};

	/** @brief コンストラクタ
	 *  @param _owner 所有オブジェクト
	 *  @param _isActive 有効/無効
	 */
	TestEnemy(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~TestEnemy() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 毎フレーム更新
	 *  @param _deltaTime 経過時間
	 */
	void Update(float _deltaTime) override;

private:
	AttackComponent* attackComponent; ///< 所有オブジェクトのAttackComponent

	float cooldownTimer;              ///< クールタイム経過
	float cooldownDuration;           ///< 攻撃間隔

	float attackDuration;             ///< 1回の攻撃時間
};