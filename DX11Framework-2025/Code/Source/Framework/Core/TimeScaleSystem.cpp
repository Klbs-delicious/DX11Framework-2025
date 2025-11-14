/** @file   TimeScaleSystem.cpp
 *  @brief  TimeScaleSystem の実装
 *  @date   2025/11/14
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/TimeScaleSystem.h"

//-----------------------------------------------------------------------------
// TimeScaleSystem class
//-----------------------------------------------------------------------------

/// @brief コンストラクタ
TimeScaleSystem::TimeScaleSystem() :
	globalScale(1.0f),
	layerScales
	{}
{
	// レイヤーをすべて 1.0f（等倍）に初期化する
	for (auto& s : this->layerScales)
	{
		s = 1.0f;
	}
}

/** @brief グローバル TimeScale を設定する
 *  @param _scale 設定するスケール値
 */
void TimeScaleSystem::SetGlobalScale(float _scale)
{
	this->globalScale = _scale;
}

/// @brief グローバル TimeScale を取得する
float TimeScaleSystem::GlobalScale() const
{
	return this->globalScale;
}

/** @brief レイヤーの TimeScale を設定する
 *  @param _layer 対象レイヤー
 *  @param _scale 設定するスケール値
 */
void TimeScaleSystem::SetLayerScale(TimeScaleLayer _layer, float _scale)
{
	size_t index = static_cast<size_t>(_layer);
	this->layerScales[index] = _scale;
}

/** @brief レイヤーの TimeScale を取得する
 *  @param _layer 対象レイヤー
 *  @return 指定レイヤーのスケール値
 */
float TimeScaleSystem::LayerScale(TimeScaleLayer _layer) const
{
	size_t index = static_cast<size_t>(_layer);
	return this->layerScales[index];
}

/** @brief 合成済み TimeScale（Global × Layer）を取得する
 *  @param _layer 対象レイヤー
 *  @return 合成されたスケール値
 */
float TimeScaleSystem::CombinedScale(TimeScaleLayer _layer) const
{
	size_t index = static_cast<size_t>(_layer);
	return this->globalScale * this->layerScales[index];
}
