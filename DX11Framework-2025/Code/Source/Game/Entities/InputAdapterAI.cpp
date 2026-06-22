/**@file InputAdapterAI.cpp
 * @date 2026/06/23
 */
#include "Include/Game/Entities/InputAdapterAI.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

 //-----------------------------------------------------------------------------

InputAdapterAI::InputAdapterAI(GameObject* _owner, bool _isActive):
	Component(_owner, _isActive),
	aiBehavior(nullptr)
{
}

void InputAdapterAI::Initialize()
{
	// AIの行動を取得するコンポーネントを取得
	this->aiBehavior = this->Owner()->GetComponent<IAIBehavior>();
	if (!this->aiBehavior)
	{
		std::cout << "[InputAdapterAI] IAIBehavior コンポーネントが見つかりません\n";
	}
}

void InputAdapterAI::Dispose()
{
	this->aiBehavior = nullptr;
}

InputCommand InputAdapterAI::GetCommand() const
{
	InputCommand command;
	command.type = InputType::None;

	if(!this->aiBehavior)
	{
		return command;
	}

	// AIの行動を取得
	EnemyDecision decision = this->aiBehavior->GetDecision();

	// 攻撃
	if (decision.type == EnemyDecisionType::Attack)
	{
		command.type = InputType::Attack;
		return command;
	}

	// 追跡
	if (decision.type == EnemyDecisionType::Chase)
	{
		command.type = InputType::Move;
		command.direction = decision.direction;
		return command;
	}

	// 待機状態は入力なしとして扱う

	return command;
}