/** @file   TestEnemy.cpp
 *  @brief  テスト用エネミー挙動コンポーネント
 *  @date   2026/01/18
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Tests/TestEnemy.h"

#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Graphics/Animator.h"

//-----------------------------------------------------------------------------
// TestEnemy class
//-----------------------------------------------------------------------------

TestEnemy::TestEnemy(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	isAttacking(false),
	attackTimer(0.0f)
{
}

void TestEnemy::Initialize()
{
	this->isAttacking = false;
	this->attackTimer = 0.0f;

	// アニメーションをループ再生する
	auto anim = this->Owner()->GetComponent<AnimationComponent>();
	if (anim)
	{
		anim->Play();
	}
}

void TestEnemy::Dispose()
{
}

void TestEnemy::FixedUpdate(float _deltaTime)
{
	if (!this->IsActive()) { return; }

	(void)_deltaTime;

	// ここで行う処理例
	// - 攻撃開始条件の判定
	// - 攻撃タイマーの進行
	// - 攻撃終了判定
}