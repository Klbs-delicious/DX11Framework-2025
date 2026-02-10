/** @file   ITimeProvider.h
 *  @brief　TimeSystem の読み取り専用インタフェース
 *  @date   2026/02/10
 */
#pragma once

 /** @class  ITimeProvider
  *  @brief  時間情報の読み取り専用インタフェース
  *  @details
  *		- TimeSystem の情報を外部から読み取るためのインタフェース
  */
class ITimeProvider
{
public:
	virtual ~ITimeProvider() = default;

	/// @brief TimeScale 適用前のデルタ（秒）
	virtual float RawDelta() const = 0;

	/// @brief 固定ステップ幅（秒）
	virtual float FixedDelta() const = 0;
};
