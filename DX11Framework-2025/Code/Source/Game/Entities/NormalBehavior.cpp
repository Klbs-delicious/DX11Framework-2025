/**@file NormalBehavior.cpp
 * @date 2026/06/22
 */
#include "Include/Game/Entities/NormalBehavior.h"

#include "Include/Framework/Entities/GameObjectManager.h"
#include "Include/Framework/Core/SystemLocator.h"

#include <iostream>

 //-----------------------------------------------------------------------------

NormalBehavior::NormalBehavior(GameObject* _owner, bool _isActive) :
	IAIBehavior(_owner, _isActive),
	target(nullptr),
	currentState(EnemyDecisionType::None)
{}

void NormalBehavior::Initialize()
{
	// プレイヤーの取得
	auto& gameObjectManager = SystemLocator::Get<GameObjectManager>();
	const auto& players = gameObjectManager.GetFindObjectsWithTag(GameTags::Tag::Player);
	this->target = (players.empty() ? nullptr : players.front());

	if (!this->target)
	{
		std::cout << "[NormalBehavior] ターゲットのプレイヤーが見つかりません\n";
	}

	this->Think();
}

void NormalBehavior::Dispose()
{
	this->target = nullptr;
}

void NormalBehavior::Update(float _deltaTime)
{
	this->thinkTimer += _deltaTime;

	// 一定時間ごとに行動を更新
	if (this->thinkTimer >= this->thinkDuration)
	{
		this->Think();
		this->thinkTimer = 0.0f;
	}
}

EnemyDecision NormalBehavior::GetDecision() const
{
	EnemyDecision decision;
	decision.type = EnemyDecisionType::None;

	if (!this->target)
	{
		return decision;
	}

	if (this->currentState == EnemyDecisionType::None)
	{
		return decision;
	}

	if (this->currentState == EnemyDecisionType::Attack)
	{
		decision.type = EnemyDecisionType::Attack;
		return decision;
	}

	// プレイヤーから離れていたら近づく
	if(this->currentState == EnemyDecisionType::Chase)
	{
		decision.type = EnemyDecisionType::Chase;
		DX::Vector3 dir = this->target->GetTransform()->GetWorldPosition() - this->Owner()->GetTransform()->GetWorldPosition();

		dir.Normalize();
		decision.direction = dir;
		return decision;
	}	

	// 追跡不可の場合待機
	if (this->currentState == EnemyDecisionType::Stay)
	{
		return decision;
	}

	// 現状 Stay / None は入力なし
	return decision;
}

void NormalBehavior::Think()
{
	if (!this->target)
	{
		this->currentState = EnemyDecisionType::None;
		return;
	}

	// プレイヤーに近かったら攻撃
	const DX::Vector3 toTarget = this->target->GetTransform()->GetWorldPosition() - this->Owner()->GetTransform()->GetWorldPosition();
	const float distance = toTarget.Length();

	if (distance < this->attackRange)
	{
		this->currentState = EnemyDecisionType::Attack;
		return;
	}

	// 追跡不可の場合待機
	if (!this->CanChase())
	{
		this->currentState = EnemyDecisionType::Stay;
		return;
	}

	// プレイヤーから離れていたら近づく
	this->currentState = EnemyDecisionType::Chase;
}

bool NormalBehavior::CanChase() const
{
	// TODO: 周りに追跡者が大勢いれば追跡不可とする
	return true;
}
