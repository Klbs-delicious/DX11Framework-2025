/** @file   AttackComponent.cpp
 *  @brief  攻撃状態と攻撃判定を管理するコンポーネント
 *  @date   2026/02/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

#include <iostream>

//-----------------------------------------------------------------------------
// AttackComponent class
//-----------------------------------------------------------------------------

AttackComponent::AttackComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	animClipManager(nullptr),
	animationComponent(nullptr),
	isAttacking(false),
	currentAttackDef{},
	clipEventWatcher{},
	passedEvents{},
	attackObj(nullptr),
	dodgeComponent(nullptr),
	timeScaleSystem(nullptr)
{
}

void AttackComponent::Initialize()
{
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->animClipManager = this->Services()->animationClips;

	// TimeScaleSystem を取得
	this->timeScaleSystem = &SystemLocator::Get<TimeScaleSystem>();

	this->isAttacking = false;
}

void AttackComponent::Dispose()
{
	this->animationComponent = nullptr;
	this->animClipManager = nullptr;
	this->attackObj = nullptr;
	this->dodgeComponent = nullptr;
	this->timeScaleSystem = nullptr;

	this->isAttacking = false;
	this->passedEvents.clear();
}

void AttackComponent::StartAttack(AttackDef _attackDef)
{
	this->currentAttackDef = _attackDef;
	this->isAttacking = true;

	//------------------------------
	// 監視状態を初期化
	//------------------------------
	if (this->animationComponent)
	{
		this->clipEventWatcher.Reset(this->animationComponent->GetNormalizedTime());
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
	if (!this->isAttacking) { return; }
	if (!this->animationComponent) { return; }
	if (!this->animClipManager) { return; }

	//------------------------------
	// 現在のクリップのイベントを監視して、通過したイベントIDを列挙する
	//------------------------------
	this->passedEvents.clear();
	const Graphics::Import::AnimationClip* currentClip = this->animationComponent->GetCurrentClip();
	if (!currentClip) { return; }

	if (currentClip->keyName != this->currentAttackDef.attackClip)
	{
		// 攻撃定義のクリップと再生中のクリップが違うなら、イベント監視をリセットして無視する
		this->clipEventWatcher.Reset(this->animationComponent->GetNormalizedTime());
		return;
	}

	//------------------------------
	// 攻撃定義のクリップと再生中のクリップが同じなら、イベント監視を行う
	//------------------------------
	const Graphics::Import::ClipEventTable* eventTable = currentClip->GetEventTable();
	const float nowNormalizedTime = this->animationComponent->GetNormalizedTime();
	this->clipEventWatcher.Update(eventTable, nowNormalizedTime, this->passedEvents);

	for (const auto& eventId : this->passedEvents)
	{
		// HitOnイベントでジャスト回避判定を行う
		if (eventId != Graphics::Import::ClipEventId::HitOn) { continue; }
		if (!this->attackObj) { continue; }

		DodgeComponent* dodge = this->dodgeComponent;
		if (!dodge)
		{
			// 攻撃対象のオブジェクトからDodgeComponentを取得してみる
			// （OnTriggerEnterで取得できていない場合に備えて）
			dodge = this->attackObj->GetComponent<DodgeComponent>();
			this->dodgeComponent = dodge;
		}
		if (!dodge) { continue; }

		// ------------------------------
		// ジャスト回避判定
		// ------------------------------
		const bool isDodging = dodge->IsDodging();
		const bool timingValid = dodge->IsDodgeTimingValid();
		const bool just = (isDodging && timingValid);

		if (!just) { continue; }

		// ------------------------------
		// Just回避成立
		// ------------------------------
		if (this->timeScaleSystem)
		{
			this->timeScaleSystem->RequestEvent(TimeScaleEventId::TestDodge);
			//std::cout << "[JustDodge] group=" << this->attackObj->TimeScale()->GetGroupName()
			//	<< " scale=" << this->attackObj->TimeScale()->GetFinalScale() << "\n";
		}
	}
}

void AttackComponent::OnTriggerEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	(void)_self;

	if (!_other) { return; }
	if (!_other->Owner()) { return; }

	// 攻撃対象のオブジェクトとDodgeComponentを取得して保持する
	this->attackObj = _other->Owner();
	this->dodgeComponent = this->attackObj->GetComponent<DodgeComponent>();
}

void AttackComponent::OnTriggerExit(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	(void)_self;

	if (!_other) { return; }
	if (!_other->Owner()) { return; }

	if (this->attackObj == _other->Owner())
	{
		// 攻撃対象のオブジェクトが範囲から出たら、攻撃対象とDodgeComponentの参照をクリアする
		this->attackObj = nullptr;
		this->dodgeComponent = nullptr;
	}
}