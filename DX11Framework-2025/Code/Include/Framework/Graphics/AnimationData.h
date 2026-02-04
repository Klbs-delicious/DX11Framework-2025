//-----------------------------------------------------------------------------
// AnimationData.h（AnimationClip に焼き込み関数を追加）
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Utils/CommonTypes.h"

#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	struct SkeletonCache;

	/** @struct AnimKeyVec3
	 *  @brief 3Dベクトルアニメーションキー
	 */
	struct AnimKeyVec3
	{
		double ticksTime = 0.0; // ticks
		DX::Vector3 value{};
		AnimKeyVec3() = default;
		AnimKeyVec3(double _time, const DX::Vector3& _v) : ticksTime(_time), value(_v) {}
	};

	/** @struct AnimKeyQuat
	 *  @brief クォータニオンアニメーションキー
	 */
	struct AnimKeyQuat
	{
		double ticksTime = 0.0; // ticks
		DX::Quaternion value{};
		AnimKeyQuat() = default;
		AnimKeyQuat(double _time, const DX::Quaternion& _q) : ticksTime(_time), value(_q) {}
	};

	/** @struct NodeTrack
	 *  @brief ノードごとのアニメーショントラックデータ
	 */
	struct NodeTrack
	{
		int nodeIndex = -1;          ///< 対応ノード index（焼き込み後に確定）
		std::string nodeName = "";   ///< デバッグ用に残す（不要ならBake後に捨てても良い）

		std::vector<AnimKeyVec3> positionKeys{};
		std::vector<AnimKeyQuat> rotationKeys{};
		std::vector<AnimKeyVec3> scaleKeys{};

		bool hasPosition = false;    ///< Bake時に確定
		bool hasRotation = false;    ///< Bake時に確定
		bool hasScale = false;       ///< Bake時に確定
	};

	/** @struct AnimationClip
	 *  @brief アニメーションクリップデータ
	 */
	struct AnimationClip
	{
		std::string name{};
		double durationTicks = 0.0;     ///< Bake時に「全キー最大時刻」で確定
		double ticksPerSecond = 0.0;    ///< 0にならないようインポータで補正

		std::vector<NodeTrack> tracks{};

		/** @brief ノード index を焼き込む
		 *  @param _skeletonCache
		 */
		void BakeNodeIndices(const SkeletonCache& _skeletonCache);

		/** @brief 焼き込み済みか取得
		 *  @return 焼き込み済みの場合 true
		 */
		bool IsBaked() const { return this->isBaked; }

	private:
		bool isBaked = false;
	};
} // namespace Graphics::Import

//-----------------------------------------------------------------------------
// Namespace : Graphics::Debug::Output
//-----------------------------------------------------------------------------
namespace Graphics::Debug::Output
{
	/** @brief AnimationClip をテキストにダンプする
	 *  @param _filePath 出力先パス
	 *  @param _clip ダンプ対象
	 */
	void DumpToText(const std::string& _filePath, const Graphics::Import::AnimationClip& _clip);
}