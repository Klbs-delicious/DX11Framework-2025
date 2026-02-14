/** @file   TestEnemy.cpp
 *  @brief  一定間隔で攻撃を出し続けるテスト用コンポーネント
 *  @date   2026/02/14
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Tests/TestEnemy.h"
#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Framework/Entities/GameObject.h"

//-----------------------------------------------------------------------------
// TestEnemy class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner 所有オブジェクト
 *  @param _isActive 有効/無効
 */
TestEnemy::TestEnemy(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	attackComponent(nullptr),
	cooldownTimer(0.0f),
	cooldownDuration(2.0f),
	attackDuration(0.5f)
{
}

/// @brief 初期化
void TestEnemy::Initialize()
{
	attackComponent = this->Owner()->GetComponent<AttackComponent>();
}

/// @brief 終了処理
void TestEnemy::Dispose()
{
	attackComponent = nullptr;
}

/** @brief 毎フレーム更新
 *  @param _deltaTime 経過時間
 */
void TestEnemy::Update(float _deltaTime)
{
	if (!IsActive())
	{
		return;
	}

	if (!attackComponent)
	{
		return;
	}

	cooldownTimer += _deltaTime;

	if (cooldownTimer >= cooldownDuration)
	{
		cooldownTimer = 0.0f;

		// 毎回クールタイム終了時に攻撃開始
		AttackDef attackDef = {
			.attackClip = "Punch",				// 攻撃アニメーションのクリップ名（仮）
			.attackType = AttackType::Melee,	// 近接攻撃
			.damage = 10.0f						// ダメージ量（仮）
		};
		attackComponent->StartAttack(attackDef);
	}
}