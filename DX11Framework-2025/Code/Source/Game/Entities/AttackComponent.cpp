/** @file   AttackComponent.cpp
 *  @brief  攻撃状態と攻撃判定を管理するコンポーネント
 *  @date   2026/02/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include <iostream>

//-----------------------------------------------------------------------------
// AttackComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _isActive コンポーネントの有効/無効
 */
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
	timeScaleGroup(nullptr),
	slowDuration(0.5f),
	slowRemainingRawSec(0.0f),
	isSlowing(false),
	timeProvider(nullptr)
{
}

/// @brief 初期化処理
void AttackComponent::Initialize()
{
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
	this->animClipManager = this->Services()->animationClips;

	this->timeScaleGroup = this->Owner()->GetComponent<TimeScaleGroup>();

	// rawDelta を取る
	this->timeProvider = &SystemLocator::Get<ITimeProvider>();

	this->isAttacking = false;
	this->isSlowing = false;
	this->slowRemainingRawSec = 0.0f;
}

/// @brief 終了処理
void AttackComponent::Dispose()
{
	// スローを終了して復帰（安全側）
	if (this->timeScaleGroup)
	{
		this->timeScaleGroup->SetGroupScale("DodgeSlow", 1.0f);
	}

	this->animationComponent = nullptr;
	this->animClipManager = nullptr;

	this->attackObj = nullptr;
	this->dodgeComponent = nullptr;

	this->timeScaleGroup = nullptr;
	this->timeProvider = nullptr;

	this->isAttacking = false;
	this->isSlowing = false;
	this->slowRemainingRawSec = 0.0f;

	this->passedEvents.clear();
}

/** @brief 攻撃開始
 *  @param _attackDef 攻撃定義
 */
void AttackComponent::StartAttack(AttackDef _attackDef)
{
	this->currentAttackDef = _attackDef;
	this->isAttacking = true;

	// 攻撃開始時点の正規化時間を基準としてセットする
	if (this->animationComponent)
	{
		this->clipEventWatcher.Reset(this->animationComponent->GetNormalizedTime());
	}
	else
	{
		this->clipEventWatcher.Reset(0.0f);
	}
}

/// @brief 攻撃終了
void AttackComponent::EndAttack()
{
	this->isAttacking = false;

	// 防御的に監視状態を初期化しておく
	this->clipEventWatcher.Reset(0.0f);
}

void AttackComponent::OnTriggerEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	(void)_self;

	if (!_other) { return; }
	if (!_other->Owner()) { return; }

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
		this->attackObj = nullptr;
		this->dodgeComponent = nullptr;
	}
}

/** @brief 毎フレーム更新
 *  @param _deltaTime フレーム間の経過時間
 */
void AttackComponent::Update(float _deltaTime)
{
	(void)_deltaTime;

	//-----------------------------------------------------------------------------
	// 攻撃処理（攻撃中のみ）
	// 先に「今フレームの JustSuccess」を確定させてから、
	// 後段でスロー残り時間を減算する（ログ順も直感通りになる）
	//-----------------------------------------------------------------------------
	if (this->isAttacking && this->animationComponent && this->animClipManager)
	{
		this->passedEvents.clear();

		const Graphics::Import::AnimationClip* currentClip = this->animationComponent->GetCurrentClip();
		if (currentClip)
		{
			if (currentClip->keyName != this->currentAttackDef.attackClip)
			{
				std::cout << "[AttackComponent] clip mismatch cur=" << currentClip->keyName
					<< " def=" << this->currentAttackDef.attackClip << "\n";

				this->clipEventWatcher.Reset(this->animationComponent->GetNormalizedTime());
			}
			else
			{
				const Graphics::Import::ClipEventTable* eventTable = currentClip->GetEventTable();
				const float nowNormalizedTime = this->animationComponent->GetNormalizedTime();
				this->clipEventWatcher.Update(eventTable, nowNormalizedTime, this->passedEvents);

				for (const auto& eventId : this->passedEvents)
				{
					if (eventId != Graphics::Import::ClipEventId::HitOn) { continue; }

					std::cout << "-------------------------------------------------------------------------------\n";
					std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
						<< "Hit On! Damage: " << this->currentAttackDef.damage << "\n";

					if (!this->attackObj)
					{
						std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
							<< "Hit On! Object: (null)\n";
						continue;
					}

					std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
						<< "Hit On! Object: " << this->attackObj->GetName() << "\n";

					DodgeComponent* dodge = this->dodgeComponent;
					if (!dodge)
					{
						dodge = this->attackObj->GetComponent<DodgeComponent>();
						this->dodgeComponent = dodge;
					}

					if (!dodge)
					{
						std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
							<< "Target has no DodgeComponent.\n";
						continue;
					}

					const bool isDodging = dodge->IsDodging();
					const bool timingValid = dodge->IsDodgeTimingValid();
					const bool just = (isDodging && timingValid);

					std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
						<< "Target has DodgeComponent. IsDodging: " << isDodging << "\n";
					std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
						<< "Target Dodge Timing Valid: " << timingValid << "\n";

					if (!just) { continue; }

					std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
						<< "JustSuccess! remain(before set)=" << this->slowRemainingRawSec
						<< " duration=" << this->slowDuration << "\n";

					// 再成立は常に延長（上書き）
					this->slowRemainingRawSec = this->slowDuration;

					// 開始時だけ適用（延長で毎回 Set しない）
					if (!this->isSlowing)
					{
						this->isSlowing = true;

						if (this->timeScaleGroup)
						{
							this->timeScaleGroup->SetGroupScale("DodgeSlow", 0.3f);
						}
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	// スロー更新（rawDeltaで残り時間を減らす）
	// 攻撃状態に依存させない：攻撃が止まってもスローが正しく終わる
	//-----------------------------------------------------------------------------
	if (!this->isSlowing) { return; }
	if (!this->timeProvider) { return; }

	const float rawDt = this->timeProvider->RawDelta();

	std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
		<< "SlowTick rawDt=" << rawDt
		<< " remaining(before)=" << this->slowRemainingRawSec
		<< " duration=" << this->slowDuration << "\n";

	this->slowRemainingRawSec -= rawDt;

	std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
		<< "SlowTick remaining(after)=" << this->slowRemainingRawSec << "\n";

	if (this->slowRemainingRawSec <= 0.0f)
	{
		this->slowRemainingRawSec = 0.0f;  
		this->isSlowing = false;

		if (this->timeScaleGroup)
		{
			this->timeScaleGroup->SetGroupScale("DodgeSlow", 1.0f);
		}

		std::cout << "[AttackComponent][" << this->Owner()->GetName() << "][" << this << "] "
			<< "Slow effect ended.\n";
	}
}