/** @file   AttackComponent.cpp
 *  @brief  攻撃状態と攻撃判定を管理するコンポーネント
 *  @date   2026/02/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Game/Entities/CharacterController.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

//-----------------------------------------------------------------------------
// AttackComponent
//-----------------------------------------------------------------------------

AttackComponent::AttackComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	animClipManager(nullptr),
	animationComponent(nullptr),
	isAttacking(false),
	justTriggered(false),
	currentAttackDef{},
	clipEventWatcher{},
	passedEvents{},
	attackObj(nullptr),
	dodgeComponent(nullptr)
{
}

void AttackComponent::Initialize()
{
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->animClipManager = this->Services()->animationClips;

	this->isAttacking = false;
	this->justTriggered = false;
}

void AttackComponent::Dispose()
{
	this->animationComponent = nullptr;
	this->animClipManager = nullptr;
	this->attackObj = nullptr;
	this->dodgeComponent = nullptr;

	this->isAttacking = false;
	this->justTriggered = false;

	this->passedEvents.clear();
}

void AttackComponent::StartAttack(AttackDef _attackDef)
{
	this->currentAttackDef = _attackDef;
	this->isAttacking = true;
	this->justTriggered = false;

	if (this->animationComponent)
	{
		this->clipEventWatcher.Reset(
			this->animationComponent->GetNormalizedTime());
	}
	else
	{
		this->clipEventWatcher.Reset(0.0f);
	}
}

void AttackComponent::EndAttack()
{
	this->isAttacking = false;
	this->clipEventWatcher.Reset(0.0f);
}

void AttackComponent::Update(float _deltaTime)
{
	(void)_deltaTime;

	if (!this->isAttacking) { return; }
	if (!this->animationComponent) { return; }
	if (!this->animClipManager) { return; }

	this->passedEvents.clear();

	const Graphics::Import::AnimationClip* currentClip =
		this->animationComponent->GetCurrentClip();
	if (!currentClip) { return; }

	if (currentClip->keyName != this->currentAttackDef.attackClip)
	{
		this->clipEventWatcher.Reset(
			this->animationComponent->GetNormalizedTime());
		return;
	}

	const Graphics::Import::ClipEventTable* eventTable =
		currentClip->GetEventTable();

	const float nowNormalizedTime =
		this->animationComponent->GetNormalizedTime();

	this->clipEventWatcher.Update(
		eventTable,
		nowNormalizedTime,
		this->passedEvents);

	for (const auto& eventId : this->passedEvents)
	{
		if (eventId != Graphics::Import::ClipEventId::HitOn) { continue; }
		if (!this->attackObj) { continue; }
		if (this->justTriggered) { continue; }

		DodgeComponent* dodge = this->dodgeComponent;
		if (!dodge)
		{
			dodge = this->attackObj->GetComponent<DodgeComponent>();
			this->dodgeComponent = dodge;
		}
		if (!dodge) { continue; }

		const bool isDodging = dodge->IsDodging();
		const bool timingValid = dodge->IsDodgeTimingValid();
		const bool just = (isDodging && timingValid);

		if (!just) { continue; }

		this->justTriggered = true;

		auto controller =
			this->attackObj->GetComponent<CharacterController>();

		if (controller)
		{
			controller->OnJustDodgeSuccess(
				this->Owner(),
				this->currentAttackDef.attackType);
		}
	}

	//-----------------------------------------------------------------------------
	// 攻撃終了判定
	//-----------------------------------------------------------------------------

	// だいたい最後に近い値
	const float endThreshold = 0.98f; 
	if (nowNormalizedTime >= endThreshold)
	{
		this->EndAttack();
		return;
	}
}

void AttackComponent::OnTriggerEnter(
	Framework::Physics::Collider3DComponent* _self,
	Framework::Physics::Collider3DComponent* _other)
{
	(void)_self;

	if (!_other) { return; }
	if (!_other->Owner()) { return; }

	this->attackObj = _other->Owner();
	this->dodgeComponent =
		this->attackObj->GetComponent<DodgeComponent>();
}

void AttackComponent::OnTriggerExit(
	Framework::Physics::Collider3DComponent* _self,
	Framework::Physics::Collider3DComponent* _other)
{
	(void)_self;

	if (!_other) { return; }
	if (!_other->Owner()) { return; }

	if (this->attackObj == _other->Owner())
	{
		this->attackObj = nullptr;
		this->dodgeComponent = nullptr;
	}
}
