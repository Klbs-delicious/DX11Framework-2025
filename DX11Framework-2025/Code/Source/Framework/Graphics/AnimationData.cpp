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

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	void AnimationClip::BakeNodeIndices(const SkeletonCache& _skeletonCache)
	{
		// 既に同じスケルトンで焼き込み済みならスキップ
		if (this->bakesSkeletonID == _skeletonCache.skeletonID){ return; }		
		if (_skeletonCache.nodes.empty()) { return; }

		// tracks 側の状態を初期化（前回の焼き込み結果を残さない）
		for (auto& tr : this->tracks)
		{
			tr.nodeIndex = -1;
			tr.hasPosition = false;
			tr.hasRotation = false;
			tr.hasScale = false;
		}

		// ノード名 -> ノードIndex の辞書を作る（補助ノードは登録しない）
		std::unordered_map<std::string, int> nodeNameToIndex;
		nodeNameToIndex.reserve(_skeletonCache.nodes.size());

		for (int nodeIndex = 0; nodeIndex < static_cast<int>(_skeletonCache.nodes.size()); ++nodeIndex)
		{
			const std::string& nodeName = _skeletonCache.nodes[nodeIndex].name;

			// 【重要】補助ノードは絶対に紐付けない（辞書にも入れない）
			if (nodeName.find("_$AssimpFbx$_") != std::string::npos)
			{
				continue;
			}

			// 同名が来たら後勝ちになる（同名が存在する構造は基本的に異常）
			nodeNameToIndex[nodeName] = nodeIndex;
		}

		// トラックごとにノード index を解決して焼き込む
		for (auto& tr : this->tracks)
		{
			auto it = nodeNameToIndex.find(tr.nodeName);
			if (it == nodeNameToIndex.end())
			{
				continue;
			}

			tr.nodeIndex = it->second;
			tr.hasPosition = (!tr.positionKeys.empty());
			tr.hasRotation = (!tr.rotationKeys.empty());
			tr.hasScale = (!tr.scaleKeys.empty());
		}

		// 焼き込み済み
		this->bakesSkeletonID = _skeletonCache.skeletonID;
	}
} // namespace Graphics::Import