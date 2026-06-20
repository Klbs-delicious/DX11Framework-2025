/**@file AnimationStateMachine.h
 * @date 2026/06/20
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/AnimationComponent.h"

#include<list>
#include <optional>

/** @struct TransitionRule
 * @brief アニメーション状態遷移のルールを定義する構造体
 * @tparam TState 状態を表す型（通常は enum class）
*/
template<typename TState>
struct TransitionRule
{
	TState  fromState;          ///< 遷移元の状態
	TState  toState;            ///< 遷移先の状態
	float   fadeDuration;       ///< 遷移時のフェード時間（秒）
	int     priority;           ///< 遷移の優先度（数値が大きいほど優先度が高い）
	bool    requiresExitTime;   ///< true:遷移元の状態が終了している必要がある
};

/** @class AnimationStateMachine
 * @brief アニメーション状態遷移を管理するコンポーネント
 * @tparam TState 状態を表す型（通常は enum class）
 */
template<typename TState>
class AnimationStateMachine : public Component, public IUpdatable
{
public:
    AnimationStateMachine(GameObject* _owner, bool _isActive = true):
		Component(_owner, _isActive),
		currentState(TState{}),
		animationComponent(nullptr)
	{
	}
    ~AnimationStateMachine() override = default;

	void Initialize() override
	{
		this->animationComponent = this->Owner()->GetComponent<AnimationComponent>();
		if (!this->animationComponent)
		{
			std::cout << "[AnimationStateMachine] AnimationComponent not found on owner.\n";
		}
	}

	void Dispose() override
	{
		this->animationComponent = nullptr;
		this->transitionRules.clear();
		this->pendingTransition.reset();
	}

	void Update(float _deltaTime) override
	{
		// 保留中の遷移がない場合は何もしない
		if (!this->pendingTransition.has_value())
		{
			return;
		}

		// アニメーション再生中は遷移を待機する
		if (this->animationComponent && this->animationComponent->IsPlaying())
		{
			return;
		}

		// 遷移を実行する
		TransitionTo(*this->pendingTransition);
		this->pendingTransition.reset();
	}

	/** @brief アニメーションの状態遷移を実行する
	 *  @param _rule 遷移ルール
	 */
	void TransitionTo(const TransitionRule<TState>& _rule)
	{
		this->animationComponent->RequestState<TState>(
			_rule.toState,
			_rule.fadeDuration);
		this->currentState = _rule.toState;
	}

	/** @brief 遷移ルールを追加する
	 *  @param _rule 追加する遷移ルール
	 */
	void AddTransition(const TransitionRule<TState>& _rule)
	{
		this->transitionRules.push_back(_rule);
	}

	/** @brief アニメーションの遷移リクエストを行う
	 *  @param _nextState 遷移先の状態
	 */
	void RequestAnimation(TState _nextState)
	{
		if (!this->animationComponent)
		{
			std::cout << "[AnimationStateMachine] AnimationComponent が見つかりません。遷移要求を無視します。:" << static_cast<int>(_nextState) << ".\n";
			return;
		}

		// 遷移ルールを検索する
		const auto* rule = this->FindTransitionRule(this->currentState, _nextState);
		if (!rule)
		{
			std::cout << "[AnimationStateMachine] この状態への遷移ルールが見つかりません。:" << static_cast<int>(_nextState) << ".\n";
			return;
		}

		// 要求された遷移が現在の状態と同じ場合は無視する
		if (this->currentState == _nextState)
		{
			std::cout << "[AnimationStateMachine] すでにこの状態です。遷移要求を無視します。:" << static_cast<int>(_nextState) << ".\n";
			return;
		}

		// 要求された遷移が現在の状態からのものでない場合は無視する
		if (rule->toState != _nextState)
		{
			std::cout << "[AnimationStateMachine] 現在の状態からの遷移ルールではありません。遷移要求を無視します。:" << static_cast<int>(_nextState) << ".\n";
			return;
		}

		// 遷移元の状態が終了している必要がある場合は、遷移完了まで待機する
		if (rule->requiresExitTime)
		{
			std::cout << "[AnimationStateMachine] 遷移元の状態が終了するまで待機します。:" << static_cast<int>(_nextState) << ".\n";
			this->pendingTransition = *rule;
		}
		else
		{
			std::cout << "[AnimationStateMachine] すぐに遷移します。:" << static_cast<int>(_nextState) << ".\n";
			this->pendingTransition.reset();
			this->TransitionTo(*rule);
		}
	}

	private:
		/** @brief 指定された状態に対応する遷移ルールを検索する
		 *  @param _currentState 現在の状態
		 *  @param _nextState 遷移先の状態
		 *  @return 指定された状態に対応する遷移ルールへのポインタ（見つからなければ nullptr）
		 */
		const TransitionRule<TState>* FindTransitionRule(TState _currentState, TState _nextState) const
		{
			for (const auto& rule : this->transitionRules)
			{
				if (rule.fromState == _currentState && rule.toState == _nextState)
				{
					return &rule;
				}
			}
			return nullptr;
		}

		/** @brief 遷移元の状態が終了しているかを判定する
		 *  @param _state 判定する状態
		 *  @return 遷移元の状態が終了していれば true
		 */
		const bool RequiresExitTime(TState _state) const
		{
			const auto* rule = this->FindTransitionRule(_state);
			if (!rule)
			{
				return false;
			}
			return rule->requiresExitTime;
		}

	private:
		std::list<TransitionRule<TState>>		transitionRules;	///< 遷移ルールのリスト
		std::optional<TransitionRule<TState>>	pendingTransition;  ///< 保留中の遷移ルール
		TState									currentState;		///< 現在の状態
		AnimationComponent* animationComponent;	///< アニメーションコンポーネントへの参照
	};