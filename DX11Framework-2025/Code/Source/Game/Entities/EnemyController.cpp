/** @file EnemyController.cpp
 *  @date 2026/06/23
 */
#include "Include/Game/Entities/EnemyController.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

EnemyController::EnemyController(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	inputAdapter(nullptr),
	attackComponent(nullptr),
	moveComponent(nullptr),
	animationComponent(nullptr),
	animStateMachine(nullptr),
	walkSpeed(2.0f)
{
}

void EnemyController::Initialize()
{
	this->inputAdapter = this->Owner()->GetComponent<InputAdapterAI>();
	this->attackComponent = this->Owner()->GetComponent<AttackComponent>();
	this->moveComponent = this->Owner()->GetComponent<MoveComponent>();
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->animStateMachine =
		this->Owner()->GetComponent<AnimationStateMachine<EnemyAnimState>>();

	if (!this->inputAdapter)
	{
		std::cout << "[EnemyController] InputAdapterAI component not found.\n";
	}
	if (!this->attackComponent)
	{
		std::cout << "[EnemyController] AttackComponent not found.\n";
	}
	if (!this->moveComponent)
	{
		std::cout << "[EnemyController] MoveComponent not found.\n";
	}

	if (this->moveComponent)
	{
		this->moveComponent->SetMoveParams(this->walkSpeed, 15.0f);
	}

	if (this->animStateMachine)
	{
		using State = EnemyAnimState;
		using Rule = TransitionRule<State>;

		// 遷移ルールを追加する
		this->animStateMachine->AddTransition(Rule{ State::Idle, State::Walk, 0.15f, 0, false });
		this->animStateMachine->AddTransition(Rule{ State::Walk, State::Idle, 0.15f, 0, false });
		this->animStateMachine->AddTransition(Rule{ State::Idle, State::Attack, 0.10f, 1, false });
		this->animStateMachine->AddTransition(Rule{ State::Walk, State::Attack, 0.10f, 1, false });
		this->animStateMachine->AddTransition(Rule{ State::Attack, State::Idle, 0.15f, 0, false });
		this->animStateMachine->AddTransition(Rule{ State::Attack, State::Walk, 0.15f, 0, false });
	}
}

void EnemyController::Dispose()
{
	this->inputAdapter = nullptr;
	this->attackComponent = nullptr;
	this->moveComponent = nullptr;
	this->animationComponent = nullptr;
	this->animStateMachine = nullptr;
}

void EnemyController::Update(float _deltaTime)
{
	if (!this->inputAdapter) { return; }
	const InputCommand command = this->inputAdapter->GetCommand();

	if (command.type == InputType::Attack)
	{
		if (this->moveComponent)
		{
			// 攻撃中は移動指示をクリアする
			this->moveComponent->ClearMoveIntent();
		}

		// 攻撃中でなければ攻撃を開始する
		if (this->attackComponent && !this->attackComponent->IsAttacking())
		{
			this->RequestAnimationIfChanged(EnemyAnimState::Attack);
			if (this->animationComponent)
			{
				this->animationComponent->Restart();
			}

			AttackDef attackDef = {
				.attackClip = "Punch",
				.attackType = AttackType::Melee,
				.damage = 10.0f
			};
			this->attackComponent->StartAttack(attackDef);
		}
		return;
	}

	if (command.type == InputType::Move)
	{
		if (this->attackComponent && this->attackComponent->IsAttacking())
		{
			// 攻撃中は移動指示を無視する
			if (this->moveComponent) { this->moveComponent->ClearMoveIntent(); }
			return;
		}

		this->RequestAnimationIfChanged(EnemyAnimState::Walk);
		if (this->moveComponent)
		{
			this->moveComponent->SetMoveIntentWorld(command.direction);
		}
		return;
	}

	if (this->moveComponent)
	{
		// 移動指示がない場合は移動をクリアする
		this->moveComponent->ClearMoveIntent();
	}

	if (!this->attackComponent || !this->attackComponent->IsAttacking())
	{
		// 攻撃中でなければアイドル状態に戻す
		this->RequestAnimationIfChanged(EnemyAnimState::Idle);
	}
}

void EnemyController::RequestAnimationIfChanged(EnemyAnimState _state)
{
	if (!this->animStateMachine) { return; }
	if (this->animStateMachine->GetCurrentState() == _state) { return; }

	this->animStateMachine->RequestAnimation(_state);
}
