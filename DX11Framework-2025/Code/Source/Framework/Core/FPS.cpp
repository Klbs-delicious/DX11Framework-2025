/** @file   FPS.cpp
 *  @brief  FPS制御の実装
 *  @date   2025/11/13
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/FPS.h"
#include <thread>

//-----------------------------------------------------------------------------
// FPS Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _targetFps 目標フレームレート
 */
FPS::FPS(uint64_t _targetFps)
    : frameInterval(std::chrono::microseconds(1'000'000 / _targetFps)),
    nextTime(std::chrono::steady_clock::now() + this->frameInterval),
    lastTime(std::chrono::steady_clock::now()),      // 初期化
    deltaMicrosec(0)
{}

/// @brief 次フレームまで待機し、ΔTime を更新する
void FPS::Tick()
{
    auto now = std::chrono::steady_clock::now();

    // まだ次フレーム時刻前なら待機
    if (now < this->nextTime)
    {
        std::this_thread::sleep_until(this->nextTime);
    }

    // 待機後の実時間
    now = std::chrono::steady_clock::now();

    // ΔTime（前フレームからの経過時間）を更新
    this->deltaMicrosec =
        std::chrono::duration_cast<std::chrono::microseconds>(now - this->lastTime).count();
    this->lastTime = now;

    // 遅れていた場合は補正（遅延累積を防ぐ）
    if (now > this->nextTime + this->frameInterval)
    {
        this->nextTime = now + this->frameInterval;
    }
    else
    {
        this->nextTime += this->frameInterval;
    }
}

/// @brief 経過時間（秒）を取得
float FPS::DeltaSec() const
{
    return static_cast<float>(this->deltaMicrosec) * 1e-6f;
}

/// @brief 経過時間（マイクロ秒）を取得
uint64_t FPS::DeltaMicrosec() const
{
    return this->deltaMicrosec;
}
/// @brief 次フレームの理想時刻を現在時刻から再設定
void FPS::ResetTime()
{
    this->nextTime = std::chrono::steady_clock::now() + this->frameInterval;
}

/// @brief  FPS値を返す
float FPS::GetFPS() const
{
    return 1.0f / DeltaSec();
}