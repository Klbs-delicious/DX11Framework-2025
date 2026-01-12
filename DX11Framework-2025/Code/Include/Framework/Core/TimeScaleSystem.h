/** @file   TimeScaleSystem.h
 *  @brief  時間スケール管理システム
 *  @date   2025/11/14
 */
#pragma once

#include "Include/Framework/Utils/NonCopyable.h"
#include <cstdint>
#include <array>

 /// @brief 時間スケールのレイヤー種別
enum class TimeScaleLayer : uint8_t
{
	Default = 0,	///< 通常のゲーム進行
	UI,				///< UI 表示など
	Effect,			///< 演出・エフェクト
	Max				///< レイヤー最大数
};

/** @class  TimeScaleSystem
 *  @brief  グローバルおよびレイヤー単位の時間スケールを管理する
 *  @details
 *          - 全体スケール（Global）
 *          - レイヤーごとのスケール（Layer）
 *          の二段階で時間係数を管理する
 */
class TimeScaleSystem : private NonCopyable
{
public:
	/// @brief コンストラクタ
	TimeScaleSystem();

	/// @brief デストラクタ
	~TimeScaleSystem() = default;

	/** @brief グローバル TimeScale を設定する
	 *  @param _scale 設定するスケール値
	 */
	void SetGlobalScale(float _scale);

	/// @brief グローバル TimeScale を取得する
	[[nodiscard]] float GlobalScale() const;

	/** @brief レイヤーの TimeScale を設定する
	 *  @param _layer 対象レイヤー
	 *  @param _scale 設定するスケール値
	 */
	void SetLayerScale(TimeScaleLayer _layer, float _scale);

	/** @brief レイヤーの TimeScale を取得する
	 *  @param _layer 対象レイヤー
	 *  @return 指定レイヤーのスケール値
	 */
	[[nodiscard]] float LayerScale(TimeScaleLayer _layer) const;

	/** @brief 合成済み TimeScale（Global × Layer）を取得する
	 *  @param _layer 対象レイヤー
	 *  @return 合成されたスケール値
	 */
	[[nodiscard]] float CombinedScale(TimeScaleLayer _layer) const;

private:
	float globalScale;															///< 全体に適用される時間スケール
	std::array<float, static_cast<size_t>(TimeScaleLayer::Max)> layerScales;	///< レイヤーごとの時間スケール
};