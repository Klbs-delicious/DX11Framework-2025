/** @file   TimeSystem.cpp
 *  @brief  TimeSystem の実装
 *  @date   2025/11/14
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Core/TimeSystem.h"

 //-----------------------------------------------------------------------------
 // TimeSystem class
 //-----------------------------------------------------------------------------

 /** @brief コンストラクタ
  *  @param uint32_t _fixedFps 固定ステップ用FPS値
  */
TimeSystem::TimeSystem(uint32_t _fixedFps)
	: lastTime(std::chrono::steady_clock::now())
	, rawDeltaSec(0.0f)
	, scaledDeltaSec(0.0f)
	, fixedDeltaSec(1.0f / static_cast<float>(_fixedFps))
	, accumulator(0.0f)
{
}

/// @brief 毎フレームの rawDeltaTime を計算する
void TimeSystem::TickRawDelta()
{
	auto now = std::chrono::steady_clock::now();
	auto delta = now - this->lastTime;
	this->lastTime = now;

	this->rawDeltaSec = std::chrono::duration<float>(delta).count();

	if (this->rawDeltaSec < 0.000001f) 
	{
		// 最小値の保証
		this->rawDeltaSec = this->fixedDeltaSec;
	}

	// Fixed用に累積する
	this->accumulator += this->rawDeltaSec;

	// 暴走防止（最大5ステップまで）
	float maxAcc = this->fixedDeltaSec * 5.0f;
	if (this->accumulator > maxAcc)
	{
		this->accumulator = maxAcc;
	}
}

/** @brief TimeScale を適用する
 *  @param float _timeScale TimeScale 値
 */
void TimeSystem::ApplyTimeScale(float _timeScale)
{
	this->scaledDeltaSec = this->rawDeltaSec * _timeScale;
}

/// @brief rawDeltaTime（秒）
float TimeSystem::RawDelta() const
{
	return this->rawDeltaSec;
}

/// @brief 固定ステップ幅（秒）
float TimeSystem::FixedDelta() const
{
	return this->fixedDeltaSec;
}

/// @brief FixedUpdate が必要か判断する
bool TimeSystem::ShouldRunFixedStep() const
{
	return (this->accumulator >= this->fixedDeltaSec);
}

/// @brief FixedUpdate を1ステップ分消費する
void TimeSystem::ConsumeFixedStep()
{
	this->accumulator -= this->fixedDeltaSec;
}

/// @brief 累積リセット
void TimeSystem::Reset()
{
	this->accumulator = 0.0f;
}