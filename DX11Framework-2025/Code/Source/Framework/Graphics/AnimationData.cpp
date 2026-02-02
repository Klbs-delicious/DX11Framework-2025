/** @file	AnimationData.cpp
 *  @brief	AnimationClip の nodeIndex 焼き込み処理
 *  @date	2026/01/28
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationData.h"
#include "Include/Framework/Graphics/ModelData.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <unordered_map>

//-----------------------------------------------------------------------------
// Namespace : Graphics::Debug::Output
//-----------------------------------------------------------------------------
namespace Graphics::Debug::Output
{
	namespace
	{
		template<typename KeyT>
		static bool IsSortedByTickTime(const std::vector<KeyT>& _keys)
		{
			if (_keys.size() <= 1) { return true; }

			for (size_t i = 1; i < _keys.size(); ++i)
			{
				if (_keys[i - 1].ticksTime > _keys[i].ticksTime)
				{
					return false;
				}
			}
			return true;
		}

		template<typename KeyT>
		static bool FindFirstInversion(
			const std::vector<KeyT>& _keys,
			size_t& _outIndex,
			double& _outPrev,
			double& _outCurr)
		{
			if (_keys.size() <= 1) { return false; }

			for (size_t i = 1; i < _keys.size(); ++i)
			{
				const double prev = _keys[i - 1].ticksTime;
				const double curr = _keys[i].ticksTime;

				if (prev > curr)
				{
					_outIndex = i;
					_outPrev = prev;
					_outCurr = curr;
					return true;
				}
			}
			return false;
		}

		template<typename KeyT>
		static double GetMaxTickTime(const std::vector<KeyT>& _keys)
		{
			double maxTick = 0.0;
			for (const auto& k : _keys)
			{
				maxTick = std::max(maxTick, k.ticksTime);
			}
			return maxTick;
		}
	}

	void DumpToText(const std::string& _filePath, const Graphics::Import::AnimationClip& _clip)
	{
		std::ofstream ofs(_filePath);
		if (!ofs.is_open())
		{
			return;
		}

		ofs << std::fixed << std::setprecision(6);

		ofs << "[AnimationClip]\n";
		ofs << "name = " << _clip.name << "\n";
		ofs << "durationTicks = " << _clip.durationTicks << "\n";
		ofs << "ticksPerSecond = " << _clip.ticksPerSecond << "\n\n";

		for (const auto& tr : _clip.tracks)
		{
			ofs << "[NodeTrack]\n";
			ofs << "nodeName = " << tr.nodeName << "\n";
			ofs << "nodeIndex = " << tr.nodeIndex << "\n\n";

			{
				const bool isSorted = IsSortedByTickTime(tr.positionKeys);
				size_t invIndex = 0;
				double invPrev = 0.0;
				double invCurr = 0.0;
				const bool hasInv = FindFirstInversion(tr.positionKeys, invIndex, invPrev, invCurr);

				ofs << "  PositionKeys (count=" << tr.positionKeys.size() << ")\n";
				ofs << "    isSorted = " << (isSorted ? "true" : "false") << "\n";
				if (hasInv)
				{
					ofs << "    firstInversionIndex = " << invIndex << " (prev=" << invPrev << ", curr=" << invCurr << ")\n";
				}

				for (size_t i = 0; i < tr.positionKeys.size(); ++i)
				{
					ofs << "    [" << i << "] t=" << tr.positionKeys[i].ticksTime << "\n";
				}
				ofs << "\n";
			}

			{
				const bool isSorted = IsSortedByTickTime(tr.rotationKeys);
				size_t invIndex = 0;
				double invPrev = 0.0;
				double invCurr = 0.0;
				const bool hasInv = FindFirstInversion(tr.rotationKeys, invIndex, invPrev, invCurr);

				ofs << "  RotationKeys (count=" << tr.rotationKeys.size() << ")\n";
				ofs << "    isSorted = " << (isSorted ? "true" : "false") << "\n";
				if (hasInv)
				{
					ofs << "    firstInversionIndex = " << invIndex << " (prev=" << invPrev << ", curr=" << invCurr << ")\n";
				}

				for (size_t i = 0; i < tr.rotationKeys.size(); ++i)
				{
					ofs << "    [" << i << "] t=" << tr.rotationKeys[i].ticksTime << "\n";
				}
				ofs << "\n";
			}

			{
				const bool isSorted = IsSortedByTickTime(tr.scaleKeys);
				size_t invIndex = 0;
				double invPrev = 0.0;
				double invCurr = 0.0;
				const bool hasInv = FindFirstInversion(tr.scaleKeys, invIndex, invPrev, invCurr);

				ofs << "  ScaleKeys (count=" << tr.scaleKeys.size() << ")\n";
				ofs << "    isSorted = " << (isSorted ? "true" : "false") << "\n";
				if (hasInv)
				{
					ofs << "    firstInversionIndex = " << invIndex << " (prev=" << invPrev << ", curr=" << invCurr << ")\n";
				}

				for (size_t i = 0; i < tr.scaleKeys.size(); ++i)
				{
					ofs << "    [" << i << "] t=" << tr.scaleKeys[i].ticksTime << "\n";
				}
				ofs << "\n";
			}

			ofs << "\n";
		}

		ofs.close();
	}
}

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	/** @brief アニメーションクリップの終了時刻をキーから計算
	 *  @param _tracks ノードトラック配列
	 *  @return 終了時刻（ticks）
	 */
	static double ComputeClipEndTicksFromKeys(const std::vector<Graphics::Import::NodeTrack>& _tracks)
	{
		double maxTick = 0.0;

		for (const auto& tr : _tracks)
		{
			maxTick = std::max(maxTick, Graphics::Debug::Output::GetMaxTickTime(tr.positionKeys));
			maxTick = std::max(maxTick, Graphics::Debug::Output::GetMaxTickTime(tr.rotationKeys));
			maxTick = std::max(maxTick, Graphics::Debug::Output::GetMaxTickTime(tr.scaleKeys));
		}

		return maxTick;
	}

	namespace
	{
		static int ResolveNodeIndexFromName(const std::string& _name, const std::unordered_map<std::string, int>& _nodeNameToIndex)
		{
			// 1. 完全一致
			auto it = _nodeNameToIndex.find(_name);
			if (it != _nodeNameToIndex.end())
			{
				return it->second;
			}

			// 2. FBX特有のサフィックスを除去して試行
			static const std::vector<std::string> suffixes = {
				"_$AssimpFbx$_Translation",
				"_$AssimpFbx$_Rotation",
				"_$AssimpFbx$_Scaling",
				"_$AssimpFbx$_PreRotation"
			};

			for (const auto& s : suffixes)
			{
				size_t pos = _name.find(s);
				if (pos != std::string::npos)
				{
					std::string baseName = _name.substr(0, pos);
					auto itBase = _nodeNameToIndex.find(baseName);
					if (itBase != _nodeNameToIndex.end())
					{
						return itBase->second;
					}
				}
			}

			// 名前解決に失敗した場合のみログを出力
			char buf[512];
			sprintf_s(buf, "[AnimationImporter] Failed to resolve node index for name: %s\n", _name.c_str());
			OutputDebugStringA(buf);

			return -1;
		}
	}
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	void AnimationClip::BakeNodeIndices(const SkeletonCache& _skeletonCache)
	{
		if (this->isBaked) { return; }
		if (_skeletonCache.nodes.empty()) { return; }

		// スケルトン側の全ノードをスキャン
		for (int i = 0; i < (int)_skeletonCache.nodes.size(); ++i) {
			const std::string& nodeName = _skeletonCache.nodes[i].name;

			for (auto& tr : this->tracks) {
				// スケルトン側の全ノードをスキャン
				for (int i = 0; i < (int)_skeletonCache.nodes.size(); ++i) {
					const std::string& nodeName = _skeletonCache.nodes[i].name;

					// 【重要】_$AssimpFbx$_ を含む中間ノードには絶対に関連付けない
					if (nodeName.find("_$AssimpFbx$_") != std::string::npos) {
						continue;
					}

					// 本体のボーン名と完全一致する場合のみ紐付ける
					if (tr.nodeName == nodeName) {
						tr.nodeIndex = i;
						tr.hasPosition = (tr.positionKeys.size() > 0);
						tr.hasRotation = (tr.rotationKeys.size() > 0);
						tr.hasScale = (tr.scaleKeys.size() > 0);
						break;
					}
				}
			}
		}
		this->isBaked = true;
	}
} // namespace Graphics::Import