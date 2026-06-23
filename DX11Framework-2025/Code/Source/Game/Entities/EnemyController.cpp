/**@file EnemyController.cpp
 * @date 2026/06/23
 */
#include "Include/Game/Entities/EnemyController.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>	

 //-----------------------------------------------------------------------------
EnemyController::EnemyController(GameObject* _owner, bool _isActive) : 
	Component(_owner, _isActive),
	inputAdapter(nullptr),
	attackComponent(nullptr),
	moveComponent(nullptr),
	walkSpeed(2.0f)
{
}

void EnemyController::Initialize()
{
	this->inputAdapter = this->Owner()->GetComponent<InputAdapterAI>();
	if (!this->inputAdapter)
	{
		std::cout << "[EnemyController] InputAdapterAI コンポーネントが見つかりません\n";
		return;
	}

	this->attackComponent = this->Owner()->GetComponent<AttackComponent>();
	if (!this->attackComponent)
	{
		std::cout << "[EnemyController] AttackComponent コンポーネントが見つかりません\n";
		return;
	}

	this->moveComponent = this->Owner()->GetComponent<MoveComponent>();
	if (!this->moveComponent)
	{
		std::cout << "[EnemyController] MoveComponent コンポーネントが見つかりません\n";
		return;
	}

	// 移動速度
	this->moveComponent->SetMoveParams(this->walkSpeed, 15.0f);
}

void EnemyController::Dispose()
{
	this->inputAdapter = nullptr;
}

void EnemyController::Update(float _deltaTime)
{
	if (!this->inputAdapter) { return; }

	// 入力の取得
	InputCommand command = this->inputAdapter->GetCommand();

	if (command.type == InputType::Attack)
	{
		// 攻撃コマンド
		if (this->attackComponent && !this->attackComponent->IsAttacking())
		{
			this->moveComponent->ClearMoveIntent();

			// TODO: 攻撃定義は敵の種類や状況に応じて変える
			AttackDef attackDef = {
				.attackClip = "Punch",				// 攻撃アニメーションのクリップ名（仮）
				.attackType = AttackType::Melee,	// 近接攻撃
				.damage = 10.0f						// ダメージ量（仮）
			};
			this->attackComponent->StartAttack(attackDef);
		}
	}
	else if(command.type == InputType::Move)
	{
		// 移動コマンド
		if (this->moveComponent)
		{
			this->moveComponent->SetMoveIntentWorld(command.direction * this->walkSpeed);
		}
	}
	else
	{
		// それ以外のコマンドは移動停止として扱う
		if (this->moveComponent)
		{
			this->moveComponent->ClearMoveIntent();
		}
	}
}