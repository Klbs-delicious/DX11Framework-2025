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
		std::string nodeName{};
		std::vector<AnimKeyVec3> positionKeys{};
		std::vector<AnimKeyQuat> rotationKeys{};
		std::vector<AnimKeyVec3> scaleKeys{};
	};

	/** @struct AnimationClip
	 *  @brief アニメーションクリップ
	 */
	struct AnimationClip
	{
		std::string name{};
		double durationTicks = 0.0;
		double ticksPerSecond = 0.0;
		std::unordered_map<std::string, NodeTrack> tracks{};
	};
} // namespace Graphics::Import
