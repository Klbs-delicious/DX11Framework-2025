#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <assimp/scene.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "Include/Framework/Graphics/AnimationData.h"
//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------

namespace Graphics::Import
{
	/** @struct AnimKeyVec3
	 *  @brief アニメーションのベクトル3キー
	 */
	struct AnimKeyVec3
	{
		double time = 0.0;
		aiVector3D value{};
	};

	/** @struct AnimKeyQuat
	 *  @brief アニメーションのクォータニオンキー
	 */
	struct AnimKeyQuat
	{
		double time = 0.0;
		aiQuaternion value{};
	};

	/** @struct NodeTrack
	 *  @brief ノードごとのアニメーショントラック
	 */
	struct NodeTrack
	{
		std::string nodeName{};						///< ノード名
		std::vector<AnimKeyVec3> positionKeys{};	///< 位置キー
		std::vector<AnimKeyQuat> rotationKeys{};	///< 回転キー
		std::vector<AnimKeyVec3> scaleKeys{};		///< スケールキー
	};

	/** @struct AnimationClip
	 *  @brief アニメーションクリップ
	 */
	struct AnimationClip
	{
		std::string name{};										///< クリップ名
		double durationTicks = 0.0;								///< クリップの長さ（ティック単位）
		double ticksPerSecond = 0.0;							///< 1秒あたりのティック数
		std::unordered_map<std::string, NodeTrack> tracks{};	/// ノード名からノードトラックへの辞書
	};
} // namespace Graphics::Import
