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
	dodgeTimingRemaining(0.0f),
	defaultDodgeDuration(0.35f),
	currentDodgeDuration(0.35f),
	justDodgeWindowDuration(0.15f),
	timeSystem(SystemLocator::Get<ITimeProvider>())
{
}

/// @brief 初期化処理
void DodgeComponent::Initialize()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;

	this->currentDodgeDuration = this->defaultDodgeDuration;
}

/// @brief 解放処理
void DodgeComponent::Dispose()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;

	this->currentDodgeDuration = this->defaultDodgeDuration;
}

/** @brief 更新処理
 *  @param _deltaTime 経過時間（秒）
 */
void DodgeComponent::Update(float _deltaTime)
{
	(void)_deltaTime;

	if (!this->isDodging)
	{
		return;
	}

	const float rawDelta = this->timeSystem.RawDelta();

	//--------------------------------------------------------------------------
	// ジャスト回避判定猶予ウィンドウの更新（rawDelta で減算）
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
	// 回避タイマーの更新（rawDelta で進行）
	//--------------------------------------------------------------------------
	this->dodgeTimer += rawDelta;
	if (this->dodgeTimer >= this->currentDodgeDuration)
	{
		this->EndDodge();
	}
}

/** @brief 回避状態を開始する
 *  @param _duration 回避持続時間（秒）
 */
void DodgeComponent::StartDodge(float _duration)
{
	this->isDodging = true;
	this->dodgeTimer = 0.0f;

	if (_duration > 0.0f)
	{
		this->currentDodgeDuration = _duration;
	}
	else
	{
		this->currentDodgeDuration = this->defaultDodgeDuration;
	}

	this->dodgeTimingRemaining = this->justDodgeWindowDuration;
}

/// @brief 回避状態を終了する
void DodgeComponent::EndDodge()
{
	this->isDodging = false;
	this->dodgeTimer = 0.0f;
	this->dodgeTimingRemaining = 0.0f;

	this->currentDodgeDuration = this->defaultDodgeDuration;
}