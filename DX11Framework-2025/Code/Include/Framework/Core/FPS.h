/**
 * @file   FPS.h
 * @brief  指定したFPSレートで処理を制御するユーティリティクラス（絶対時刻待機版）
 * @date   2025/07/04
 */
#pragma once

#include <chrono>
#include <thread>

 /**
  * @class  FPS
  * @brief  フレームタイミングを安定化するためのFPS制御クラス
  * @details
  *  - 絶対時刻待機方式（sleep_until）を採用し、累積誤差をリセット
  *  - コンストラクタで求めた frameInterval_ をもとに、
  *    次フレームの理想開始時刻 nextTime_ を更新し続ける
  */
class FPS {
public:
    /**
     * @brief  コンストラクタ
     * @param  _targetFps 目標フレームレート（FPS）
     */
    explicit FPS(uint64_t _targetFps)
        : frameInterval(std::chrono::microseconds(1'000'000 / _targetFps)),
        nextTime(std::chrono::steady_clock::now() + this->frameInterval),
        deltaMicrosec(0)
    {}

    /// @brief  次フレームの理想開始時刻を更新する
    void Tick() 
    {
        std::this_thread::sleep_until(this->nextTime);      // 次のフレームの理想開始時刻まで待機     
        this->nextTime += this->frameInterval;              // 次フレームの理想開始時刻を更新
    }

    /// @brief  実際のフレーム間隔を計測する
    void Measure() 
    {
        // 前回の計測から今回までにかかった実時間（差分）を計算
        auto now = std::chrono::steady_clock::now();
        this->deltaMicrosec = std::chrono::duration_cast<std::chrono::microseconds>(now - this->lastTime).count();

        // 現在時刻を保持
        this->lastTime = now;    
    }

    /// @brief	次フレームの理想開始時刻のリセット
    void ResetTime()
    {
        this->nextTime = std::chrono::steady_clock::now() + this->frameInterval;
    }

    /**
     * @brief  前回 Tick からの経過時間を秒で取得
     * @return 経過時間（秒）
     */
    float DeltaSec() const
    {
        return static_cast<float>(this->deltaMicrosec) * 1e-6f;
    }

    /**
     * @brief  前回 Tick からの経過時間をマイクロ秒で取得
     * @return 経過時間（マイクロ秒）
     */
    uint64_t DeltaMicrosec() const
    {
        return this->deltaMicrosec;
    }

private:
    const std::chrono::steady_clock::duration   frameInterval;  ///< 目標フレーム間隔（µs）
    std::chrono::steady_clock::time_point        nextTime;      ///< 次フレームの理想開始時刻
    std::chrono::steady_clock::time_point        lastTime;      ///< 前回のフレーム計測を行った時刻
    uint64_t                                     deltaMicrosec; ///< 実際のフレーム間隔（µs）
};