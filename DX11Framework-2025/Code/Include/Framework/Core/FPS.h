/** @file   FPS.h
 *  @brief  FPS制御（絶対時刻方式＋遅延補正つき）
 *  @date   2025/11/13
 */
#pragma once

#include <chrono>

 /** @class  FPS
  *  @brief  フレームレート制御クラス（sleep_until方式）
  *  @details
  *          - 絶対時刻方式のため累積誤差が発生しにくい
  *          - 重いフレームで遅れた場合は補正して追いつく
  *          - Tick() 内で自動的にδ時間（ΔTime）を計測する
  */
class FPS
{
public:
    /** @brief コンストラクタ
     *  @param _targetFps 目標フレームレート
     */
    explicit FPS(uint64_t _targetFps);

    /// @brief 次フレームまで待機し、ΔTime を更新する
    void Tick();

    /// @brief 経過時間（秒）を取得
    float DeltaSec() const;

    /// @brief 経過時間（マイクロ秒）を取得
    uint64_t DeltaMicrosec() const;

    /// @brief 次フレームの理想時刻を現在時刻から再設定
    void ResetTime();

    /// @brief  FPS値を返す
    float GetFPS()const;

private:
    const std::chrono::steady_clock::duration frameInterval; ///< 1フレームの理想間隔
    std::chrono::steady_clock::time_point     nextTime;      ///< 次フレームの理想時刻
    std::chrono::steady_clock::time_point     lastTime;      ///< 前フレーム計測時刻
    uint64_t                                  deltaMicrosec; ///< 実フレーム間隔(μs)
};