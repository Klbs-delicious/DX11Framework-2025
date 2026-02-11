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
	passedEvents{}
{
}

/// @brief 初期化処理
void AttackComponent::Initialize()
{
	// アニメーションコンポーネントを取得
	this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();

	// アニメーションクリップ管理を取得
	// Services() の実体が EngineServices を想定（既存運用に合わせる）
	this->animClipManager = this->Services()->animationClips;
}

/// @brief 終了処理
void AttackComponent::Dispose()
{
	this->animationComponent = nullptr;
	this->animClipManager = nullptr;
	this->isAttacking = false;
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

	// 次回 StartAttack で Reset(now) する前提だが、
	// 防御的に監視状態を初期化しておく
	this->clipEventWatcher.Reset(0.0f);
}

/** @brief 毎フレーム更新
 *  @param _deltaTime フレーム間の経過時間
 */
void AttackComponent::Update(float _deltaTime)
{
	if (!this->isAttacking) { return; }
	if (!this->animationComponent) { return; }
	if (!this->animClipManager) { return; }

	this->passedEvents.clear();

	// 現在再生中のクリップを取得（正規化時間と必ず同じ参照元にする）
	const Graphics::Import::AnimationClip* currentClip = this->animationComponent->GetCurrentClip();
	if (!currentClip) { return; }

	// 攻撃定義の clipKey と、現在再生中クリップが一致しているかを確認する
	// ここで一致しない場合に攻撃を即終了させるとクロスフェード等で壊れやすいので、
	// 「このフレームは監視しない」に留めて watcher を同期する
	if (currentClip->keyName != this->currentAttackDef.attackClip)
	{
		// 一致しない理由を追えるようにデバッグ出力しておく
		std::cout << "[AttackComponent] clip mismatch cur=" << currentClip->keyName << " def=" << this->currentAttackDef.attackClip << std::endl;

		// 監視状態を現在時間でリセットして同期させる
		this->clipEventWatcher.Reset(this->animationComponent->GetNormalizedTime());
		return;
	}

	// クリップのイベントテーブルを取得（無ければ nullptr）
	const Graphics::Import::ClipEventTable* eventTable = currentClip->GetEventTable();

	// 現在の正規化時間（0.0～1.0）
	const float nowNormalizedTime = this->animationComponent->GetNormalizedTime();

	// クリップイベントの更新を行う
	// （通過したイベントIDが passedEvents に格納される）
	this->clipEventWatcher.Update(eventTable, nowNormalizedTime, this->passedEvents);

	//-----------------------------------------------------------------------------
	// 通過したイベントに応じた処理
	//-----------------------------------------------------------------------------
	for (const auto& eventId : this->passedEvents)
	{
		if (eventId == Graphics::Import::ClipEventId::HitOn)
		{
			// ヒット判定ON処理
			// ここで「当たり判定成立＋回避中＋ジャスト窓＋監視対象＋未成立」を同フレームで判定する
			std::cout << "[AttackComponent] Hit On! Damage: " << this->currentAttackDef.damage << std::endl;
		}
		else if (eventId == Graphics::Import::ClipEventId::HitOff)
		{
			// ヒット判定OFF処理
			std::cout << "[AttackComponent] Hit Off!" << std::endl;
		}
	}
}