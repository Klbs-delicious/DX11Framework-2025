/**@file NormalBehavior.cpp
 * @date 2026/06/22
 */
#include "Include/Game/Entities/NormalBehavior.h"

#include "Include/Framework/Entities/GameObjectManager.h"
#include "Include/Framework/Core/SystemLocator.h"

#include <iostream>

 //-----------------------------------------------------------------------------

NormalBehavior::NormalBehavior(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
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
}

void NormalBehavior::Dispose()
{
	this->target = nullptr;
}

EnemyDecision NormalBehavior::GetDecision() const
{
	EnemyDecision decision;
	if (!this->target)
	{
		decision.type = EnemyDecisionType::None;
		return decision;
	}

	// プレイヤーに近かったら攻撃
	const DX::Vector3 toTarget = target->GetTransform()->GetWorldPosition() - this->Owner()->GetTransform()->GetWorldPosition();
	const float distance = toTarget.Length();

	if (distance < this->attackRange)
	{
		decision.type = EnemyDecisionType::Attack;
		return decision;
	}

	// TODO: 追跡不可の場合待機
	//if (!this->CanChase())
	//{
	//	decision.type = EnemyDecisionType::Stay;
	//	return decision;
	//}

	// プレイヤーから離れていたら近づく
	decision.type = EnemyDecisionType::Chase;
	DX::Vector3 dir = toTarget;
	dir.Normalize();					
	decision.direction = dir;
	return decision;
}

bool NormalBehavior::CanChase() const
{
	// TODO: 周りに追跡者が大勢いれば追跡不可とする
	return true;
}
