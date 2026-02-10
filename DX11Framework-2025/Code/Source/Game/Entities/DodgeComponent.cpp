/** @file   DodgeComponent.cpp
 *  @brief  回避（Dodge）状態の管理を行うコンポーネント
 *  @date   2026/02/10
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/DodgeComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/SystemLocator.h"

//-----------------------------------------------------------------------------
// DodgeComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _isActive コンポーネントの有効/無効
 */
DodgeComponent::DodgeComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	isDodging(false),
	dodgeTimer(0.0f),
	dodgeDuration(0.35f),
	dodgeTimingRemaining(0.0f),
	dodgeTimingDuration(0.15f),
	timeSystem(SystemLocator::Get<ITimeProvider>())
{
}

/// @brief 初期化処理
void DodgeComponent::Initialize()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;
}

/// @brief 解放処理
void DodgeComponent::Dispose()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;
}

/** @brief 更新処理
 *  @param _deltaTime 経過時間（秒）
 */
void DodgeComponent::Update(float _deltaTime)
{
	(void)_deltaTime;

	if (!this->isDodging){ return; }

	const float rawDelta = this->timeSystem.RawDelta();

	//--------------------------------------------------------------------------
	// 判定猶予ウィンドウの更新
	//--------------------------------------------------------------------------
	if (this->dodgeTimingRemaining > 0.0f)
	{
		this->dodgeTimingRemaining -= rawDelta;
		if (this->dodgeTimingRemaining < 0.0f)
		{
			this->dodgeTimingRemaining = 0.0f;
		}
	}

	//--------------------------------------------------------------------------
	// 回避タイマーの更新
	//--------------------------------------------------------------------------
	this->dodgeTimer += rawDelta;
	if (this->dodgeTimer >= this->dodgeDuration)
	{
		this->EndDodge();
	}
}

/** @brief 回避状態を開始する
 *  @param _duration 回避持続時間（秒）
 */
void DodgeComponent::StartDodge(float _duration)
{
	const float duration = (_duration > 0.0f) ? _duration : this->dodgeDuration;

	this->isDodging = true;
	this->dodgeTimer = 0.0f;
	this->dodgeDuration = duration;

	this->dodgeTimingRemaining = this->dodgeTimingDuration;
}

/// @brief 回避状態を終了する
void DodgeComponent::EndDodge()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;
}
