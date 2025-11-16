/** @file   TimeSystem.h
 *  @brief  時間管理を行うシステム（可変Update・固定Fixed・TimeScale対応）
 *  @date   2025/11/14
 */
#pragma once

#include <chrono>

 /** @class  TimeSystem
  *  @brief  ゲームループ用の時間管理クラス
  *  @details
  *          - rawDeltaTime（実時間差）を計算
  *          - scaledDeltaTime（TimeScale適用）を生成
  *          - 固定ステップ方式の accumulator で FixedUpdate を制御
  *          - PhysicsSystem は fixedDeltaTime で TimeScale 非適用
  */
class TimeSystem
{
public:
	/** @brief コンストラクタ
	 *  @param uint32_t _fixedFps 固定ステップ用FPS値
	 */
	explicit TimeSystem(uint32_t _fixedFps = 60);

	/// @brief 毎フレームの rawDeltaTime を計算する
	void TickRawDelta();

	/** @brief TimeScale を適用し scaledDeltaTime を生成する
	 *  @param float _timeScale TimeScale 値
	 */
	void ApplyTimeScale(float _timeScale);

	/// @brief rawDeltaTime（秒）を返す
	[[nodiscard]] float RawDelta() const;

	/// @brief scaledDeltaTime（秒）を返す
	[[nodiscard]] float ScaledDelta() const;

	/// @brief 固定ステップ幅（秒）を返す
	[[nodiscard]] float FixedDelta() const;

	/// @brief FixedUpdate を実行すべきか判定する
	bool ShouldRunFixedStep() const;

	/// @brief FixedUpdate を1回分消費する
	void ConsumeFixedStep();

	/// @brief 累積時間をリセット
	void Reset();

private:
	std::chrono::steady_clock::time_point lastTime; ///< 前フレーム時刻
	float rawDeltaSec;                              ///< TimeScale非適用Δ時間
	float scaledDeltaSec;                           ///< TimeScale適用Δ時間
	float fixedDeltaSec;                             ///< 固定ステップΔ時間

	float accumulator;                               ///< 固定ステップ累積
};