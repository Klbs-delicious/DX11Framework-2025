/** @file   SkinningDebug.cpp
 *  @brief  SkeletonCache のスキニング基準情報ダンプ
 *  @date   2026/02/01
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Tests/SkinningDebug.h"
#include "Include/Framework/Graphics/ModelData.h" 
#include "Include/Framework/Graphics/AnimationData.h" 

#include <DirectXMath.h>

#include <cmath>
#include <iostream>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	static_assert(sizeof(DX::Matrix4x4) == sizeof(DirectX::XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが一致しません。");


	void LogHeader(const char* _title)
	{
		std::cout << "\n";
		std::cout << "============================================================\n";
		std::cout << _title << "\n";
		std::cout << "============================================================\n";
	}

	void PrintMatrix4x4(const char* _label, const DX::Matrix4x4& _m)
	{
		std::cout << _label << "\n";
		std::cout
			<< "  [r0] " << _m.m[0][0] << ", " << _m.m[0][1] << ", " << _m.m[0][2] << ", " << _m.m[0][3] << "\n"
			<< "  [r1] " << _m.m[1][0] << ", " << _m.m[1][1] << ", " << _m.m[1][2] << ", " << _m.m[1][3] << "\n"
			<< "  [r2] " << _m.m[2][0] << ", " << _m.m[2][1] << ", " << _m.m[2][2] << ", " << _m.m[2][3] << "\n"
			<< "  [r3] " << _m.m[3][0] << ", " << _m.m[3][1] << ", " << _m.m[3][2] << ", " << _m.m[3][3] << "\n";
	}

	float MaxAbsDiffIdentity(const DX::Matrix4x4& _m)
	{
		const float id[16] =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		const float* p = reinterpret_cast<const float*>(&_m);

		float maxAbs = 0.0f;
		for (int i = 0; i < 16; i++)
		{
			const float d = std::fabs(p[i] - id[i]);
			if (d > maxAbs)
			{
				maxAbs = d;
			}
		}
		return maxAbs;
	}

	static void DumpMatrix(std::ostream& _os, const char* _name, const DX::Matrix4x4& _m)
	{
		_os << "[" << _name << "]\n";
		_os << std::fixed << std::setprecision(6);
		_os << "  " << _m._11 << ", " << _m._12 << ", " << _m._13 << ", " << _m._14 << "\n";
		_os << "  " << _m._21 << ", " << _m._22 << ", " << _m._23 << ", " << _m._24 << "\n";
		_os << "  " << _m._31 << ", " << _m._32 << ", " << _m._33 << ", " << _m._34 << "\n";
		_os << "  " << _m._41 << ", " << _m._42 << ", " << _m._43 << ", " << _m._44 << "\n\n";
	}

	void DumpSkeletonBasisToText(const char* _filename, const Graphics::Import::SkeletonCache& _cache)
	{
		std::ofstream ofs(_filename);
		if (!ofs.is_open())
		{
			return;
		}

		ofs << "=== Skeleton Basis Dump ===\n";
		ofs << "nodeCount=" << _cache.nodes.size() << "\n";
		ofs << "boneOffsetCount=" << _cache.boneOffset.size() << "\n";
		ofs << "meshRootNodeIndex=" << _cache.meshRootNodeIndex << "\n\n";

		DumpMatrix(ofs, "globalInverse", _cache.globalInverse);

		ofs << "--- boneOffset ---\n";
		for (size_t i = 0; i < _cache.boneOffset.size(); i++)
		{
			std::string name = "boneOffset[" + std::to_string(i) + "]";
			DumpMatrix(ofs, name.c_str(), _cache.boneOffset[i]);
		}

		ofs << "--- nodes bindLocal ---\n";
		for (size_t i = 0; i < _cache.nodes.size(); i++)
		{
			std::string name =
				"node[" + std::to_string(i) + "] " +
				_cache.nodes[i].name +
				" boneIndex=" + std::to_string(_cache.nodes[i].boneIndex);

			DumpMatrix(ofs, name.c_str(), _cache.nodes[i].bindLocalMatrix);
		}
	}

	static DirectX::XMFLOAT4X4 ToXMFLOAT4X4(const DX::Matrix4x4& _m)
	{
		DirectX::XMFLOAT4X4 out{};
		std::memcpy(&out, &_m, sizeof(out));
		return out;
	}

	static void WriteMatrix(std::ofstream& _ofs, const char* _label, const DX::Matrix4x4& _m)
	{
		const DirectX::XMFLOAT4X4 f = ToXMFLOAT4X4(_m);

		_ofs << _label << "\n";
		_ofs << "  [r0] " << f._11 << ", " << f._12 << ", " << f._13 << ", " << f._14 << "\n";
		_ofs << "  [r1] " << f._21 << ", " << f._22 << ", " << f._23 << ", " << f._24 << "\n";
		_ofs << "  [r2] " << f._31 << ", " << f._32 << ", " << f._33 << ", " << f._34 << "\n";
		_ofs << "  [r3] " << f._41 << ", " << f._42 << ", " << f._43 << ", " << f._44 << "\n";
	}

	static bool IsIdentityApprox(const DX::Matrix4x4& _m, float _eps = 1.0e-5f)
	{
		const DirectX::XMFLOAT4X4 f = ToXMFLOAT4X4(_m);

		const float diagErr =
			std::fabsf(f._11 - 1.0f) +
			std::fabsf(f._22 - 1.0f) +
			std::fabsf(f._33 - 1.0f) +
			std::fabsf(f._44 - 1.0f);

		const float offSum =
			std::fabsf(f._12) + std::fabsf(f._13) + std::fabsf(f._14) +
			std::fabsf(f._21) + std::fabsf(f._23) + std::fabsf(f._24) +
			std::fabsf(f._31) + std::fabsf(f._32) + std::fabsf(f._34) +
			std::fabsf(f._41) + std::fabsf(f._42) + std::fabsf(f._43);

		return (diagErr + offSum) <= _eps;
	}

	static void WriteVectorInt(std::ofstream& _ofs, const char* _label, const std::vector<int>& _v, size_t _maxCount = 64)
	{
		_ofs << _label << " (count=" << _v.size() << ")\n";
		const size_t n = std::min(_v.size(), _maxCount);
		for (size_t i = 0; i < n; i++)
		{
			_ofs << "  [" << i << "] " << _v[i] << "\n";
		}
		if (_v.size() > _maxCount)
		{
			_ofs << "  ... (truncated)\n";
		}
	}

	static void WriteBoneOffsetSummary(std::ofstream& _ofs, const std::vector<DX::Matrix4x4>& _boneOffset, size_t _maxCount = 12)
	{
		_ofs << "boneOffset (count=" << _boneOffset.size() << ")\n";
		const size_t n = std::min(_boneOffset.size(), _maxCount);
		for (size_t i = 0; i < n; i++)
		{
			const DirectX::XMFLOAT4X4 f = ToXMFLOAT4X4(_boneOffset[i]);
			_ofs << "  [" << i << "] "
				<< "t=(" << f._41 << ", " << f._42 << ", " << f._43 << ") "
				<< "diag=(" << f._11 << ", " << f._22 << ", " << f._33 << ", " << f._44 << ")"
				<< "\n";
		}
		if (_boneOffset.size() > _maxCount)
		{
			_ofs << "  ... (truncated)\n";
		}
	}

	static bool OpenAppend(std::ofstream& _ofs, const char* _filePath)
	{
		if (!_filePath) { return false; }

		_ofs.open(_filePath, std::ios::app);
		return _ofs.is_open();
	}

	static void DumpMatrix4x4_RowCol(
		std::ofstream& _ofs,
		const char* _label,
		const DX::Matrix4x4& _m)
	{
		_ofs << _label << "\n";
		_ofs << "  [r0] " << _m._11 << ", " << _m._12 << ", " << _m._13 << ", " << _m._14 << "\n";
		_ofs << "  [r1] " << _m._21 << ", " << _m._22 << ", " << _m._23 << ", " << _m._24 << "\n";
		_ofs << "  [r2] " << _m._31 << ", " << _m._32 << ", " << _m._33 << ", " << _m._34 << "\n";
		_ofs << "  [r3] " << _m._41 << ", " << _m._42 << ", " << _m._43 << ", " << _m._44 << "\n";

		// 行列の見方を間違えやすいので translation を併記する
		_ofs << "  col4(translation) = (" << _m._14 << ", " << _m._24 << ", " << _m._34 << ")\n";
	}
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Debug::Output
//-----------------------------------------------------------------------------
namespace Graphics::Debug::Output
{
	void DumpSkinningBasisToText(
		const std::string& _filePath,
		const Graphics::Import::SkeletonCache& _skeletonCache)
	{
		std::ofstream ofs(_filePath);
		if (!ofs.is_open())
		{
			return;
		}

		ofs << std::fixed << std::setprecision(6);

		ofs << "============================================================\n";
		ofs << "[SkinningBasisDump]\n";
		ofs << "============================================================\n";

		ofs << "nodeCount = " << _skeletonCache.nodes.size() << "\n";
		ofs << "meshRootNodeIndex = " << _skeletonCache.meshRootNodeIndex << "\n";
		ofs << "orderCount = " << _skeletonCache.order.size() << "\n";
		ofs << "boneCount = " << _skeletonCache.boneOffset.size() << "\n";
		ofs << "boneIndexToNodeIndexCount = " << _skeletonCache.boneIndexToNodeIndex.size() << "\n\n";

		::WriteMatrix(ofs, "[A] globalInverse (inverse(bindGlobal(meshRoot)) という想定値)", _skeletonCache.globalInverse);
		ofs << "  isIdentityApprox = " << (::IsIdentityApprox(_skeletonCache.globalInverse) ? "true" : "false") << "\n\n";

		::WriteVectorInt(ofs, "[B] order (parent first)", _skeletonCache.order, 96);
		ofs << "\n";

		::WriteVectorInt(ofs, "[C] boneIndexToNodeIndex", _skeletonCache.boneIndexToNodeIndex, 96);
		ofs << "\n";

		::WriteBoneOffsetSummary(ofs, _skeletonCache.boneOffset, 16);
		ofs << "\n";

		ofs.close();
	}

	float MaxAbsDiffIdentity(const DX::Matrix4x4& _m)
	{
		const float id[16] =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};

		const float* p = reinterpret_cast<const float*>(&_m);

		float maxAbs = 0.0f;
		for (int i = 0; i < 16; i++)
		{
			const float d = std::fabs(p[i] - id[i]);
			if (d > maxAbs)
			{
				maxAbs = d;
			}
		}
		return maxAbs;
	}

	void DumpBindPoseGlobalCheckOnce(
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const Graphics::Import::Pose& _pose)
	{
		const int root = _skeletonCache.meshRootNodeIndex;
		if (root < 0) { return; }
		if (static_cast<size_t>(root) >= _pose.globalMatrices.size()) { return; }

		const DX::Matrix4x4 check =
			_pose.globalMatrices[static_cast<size_t>(root)] *
			_skeletonCache.globalInverse;

		const float err = MaxAbsDiffIdentity(check);

		//std::cout << "[SkinningDebug] BindPose Global Check\n";
		//std::cout << "  meshRootIndex = " << root << "\n";
		//std::cout << "  meshRootName  = \"" << _skeletonCache.nodes[root].name << "\"\n";
		//std::cout << "  maxAbsError   = " << std::fixed << std::setprecision(6) << err << "\n";
	}

	void DumpBindPoseSkinCheckOnce(
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const Graphics::Import::Pose& _pose)
	{
		//std::cout << "[SkinningDebug] BindPose Skin Check\n";

		const size_t nodeCount = _pose.globalMatrices.size();
		const size_t boneCount = _skeletonCache.boneOffset.size();

		int printed = 0;
		const int maxPrint = 6;

		for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
		{
			const int boneIndex = _skeletonCache.nodes[nodeIndex].boneIndex;
			if (boneIndex < 0) { continue; }
			if (static_cast<size_t>(boneIndex) >= boneCount) { continue; }

			const DX::Matrix4x4 skin =
				_skeletonCache.boneOffset[static_cast<size_t>(boneIndex)] *
				_pose.globalMatrices[nodeIndex] *
				_skeletonCache.globalInverse;

			const float err = MaxAbsDiffIdentity(skin);

			// 大きな誤差のみ表示
			if (err > 1.0e-3f && printed < maxPrint)
			{
				std::cout
					<< "  boneIndex=" << boneIndex
					<< " node=\"" << _skeletonCache.nodes[nodeIndex].name << "\""
					<< " maxAbsError=" << std::fixed << std::setprecision(6) << err
					<< "\n";
				printed++;
			}
		}
	}

	void DumpSkeletonImportCheck(
		const char* _filePath,
		const Graphics::Import::SkeletonCache& _cache,
		size_t _maxNodeDumpCount)
	{
		std::ofstream ofs{};
		if (!OpenAppend(ofs, _filePath)) { return; }

		ofs << "=== Skeleton Import Check ===\n";
		ofs << "Node Count: " << _cache.nodes.size() << "\n";

		ofs << "\n[Global Inverse Root Matrix]\n";
		DumpMatrix4x4_RowCol(ofs, "globalInverse", _cache.globalInverse);

		if (!_cache.boneOffset.empty())
		{
			ofs << "\n[Bone Offset (First Bone)]\n";
			DumpMatrix4x4_RowCol(ofs, "boneOffset[0]", _cache.boneOffset[0]);
		}

		const size_t dumpCount = std::min(_maxNodeDumpCount, _cache.nodes.size());
		for (size_t i = 0; i < dumpCount; ++i)
		{
			ofs << "\nNode[" << i << "]: " << _cache.nodes[i].name << "\n";
			DumpMatrix4x4_RowCol(ofs, "  bindLocalMatrix", _cache.nodes[i].bindLocalMatrix);
		}
	}

	void DumpSkeletonOrderCheck(
		const char* _filePath,
		const Graphics::Import::SkeletonCache& _cache)
	{
		std::ofstream ofs{};
		if (!OpenAppend(ofs, _filePath)) { return; }

		ofs << "Node Count: " << _cache.nodes.size() << "\n";
		ofs << "Order Size: " << _cache.order.size() << "\n\n";

		std::unordered_set<int> processed{};
		bool isValid = true;

		for (size_t i = 0; i < _cache.order.size(); ++i)
		{
			const int idx = _cache.order[i];
			const auto& node = _cache.nodes[static_cast<size_t>(idx)];

			// 親が先に出ていないなら order が崩れている
			if (node.parentIndex >= 0)
			{
				if (processed.find(node.parentIndex) == processed.end())
				{
					ofs << "[ERROR] Node '" << node.name << "' idx=" << idx
						<< " parentIndex=" << node.parentIndex << "\n";
					isValid = false;
				}
			}

			processed.insert(idx);
			ofs << "Order[" << i << "]: Index " << idx << " Name: " << node.name << "\n";
		}

		if (isValid)
		{
			ofs << "\nRESULT: VALID\n";
		}
		else
		{
			ofs << "\nRESULT: INVALID\n";
		}
	}

	void DumpTrackBakeStatus(
		const char* _filePath,
		const Graphics::Import::AnimationClip& _clip,
		const Graphics::Import::SkeletonCache* _skeletonCache,
		const char* _tag)
	{
		std::ofstream ofs{};
		if (!OpenAppend(ofs, _filePath)) { return; }

		ofs << "\n============================================================\n";
		ofs << "[TrackBakeStatus] " << (_tag ? _tag : "") << "\n";
		ofs << "clipName=\"" << _clip.name << "\"\n";
		ofs << "trackCount=" << _clip.tracks.size() << "\n";

		if (_skeletonCache)
		{
			ofs << "nodeCount=" << _skeletonCache->nodes.size() << "\n";
		}

		int unresolvedCount = 0;

		for (size_t ti = 0; ti < _clip.tracks.size(); ++ti)
		{
			const auto& tr = _clip.tracks[ti];

			ofs << "track[" << ti << "] ";
			ofs << "name=\"" << tr.nodeName << "\" ";
			ofs << "nodeIndex=" << tr.nodeIndex;

			if (_skeletonCache && tr.nodeIndex >= 0 && tr.nodeIndex < static_cast<int>(_skeletonCache->nodes.size()))
			{
				const auto& node = _skeletonCache->nodes[static_cast<size_t>(tr.nodeIndex)];
				ofs << " resolvedNodeName=\"" << node.name << "\"";
			}
			else
			{
				ofs << " resolvedNodeName=\"\"";
			}

			ofs << " posKeys=" << tr.positionKeys.size();
			ofs << " rotKeys=" << tr.rotationKeys.size();
			ofs << " sclKeys=" << tr.scaleKeys.size();

			if (tr.nodeIndex < 0)
			{
				unresolvedCount++;
				ofs << " UNRESOLVED";
			}

			ofs << "\n";
		}

		ofs << "unresolvedCount=" << unresolvedCount << "\n";
		ofs << "============================================================\n";
	}

	void DumpBakeValidationOnce(
		const char* _filePath,
		const Graphics::Import::AnimationClip& _clip,
		const Graphics::Import::SkeletonCache& _skeletonCache,
		const char* _tag)
	{
		static bool dumped = false;
		if (dumped) { return; }

		std::ofstream ofs{};
		if (!OpenAppend(ofs, _filePath)) { return; }

		ofs << "\n============================================================\n";
		ofs << "[BakeValidationOnce] " << (_tag ? _tag : "") << "\n";
		ofs << "clipName=\"" << _clip.name << "\"\n";
		ofs << "trackCount=" << _clip.tracks.size() << "\n";
		ofs << "nodeCount=" << _skeletonCache.nodes.size() << "\n";

		std::unordered_map<std::string, std::vector<int>> nameToIndices{};
		nameToIndices.reserve(_skeletonCache.nodes.size());

		for (int i = 0; i < static_cast<int>(_skeletonCache.nodes.size()); ++i)
		{
			nameToIndices[_skeletonCache.nodes[static_cast<size_t>(i)].name].push_back(i);
		}

		int duplicateNameCount = 0;
		size_t duplicateNodesTotal = 0;

		for (const auto& kv : nameToIndices)
		{
			if (kv.second.size() >= 2)
			{
				duplicateNameCount++;
				duplicateNodesTotal += kv.second.size();
			}
		}

		ofs << "duplicateNameCount=" << duplicateNameCount << "\n";
		ofs << "duplicateNodesTotal=" << duplicateNodesTotal << "\n";
		ofs << "============================================================\n";

		dumped = true;
	}

	void DumpBoneMatrixCpuGpuPairOnce(
		const char* _filePath,
		int _boneIndex,
		const DX::Matrix4x4& _skinCpu,
		const DX::Matrix4x4& _skinUploaded)
	{
		static bool dumped = false;
		if (dumped) { return; }

		std::ofstream ofs{};
		if (!OpenAppend(ofs, _filePath)) { return; }

		ofs << "\n============================================================\n";
		ofs << "[BoneMatrixCpuGpuPairOnce]\n";
		ofs << "boneIndex=" << _boneIndex << "\n";

		DumpMatrix4x4_RowCol(ofs, "CPU skin (no transpose):", _skinCpu);
		ofs << "CPU translation(row4) = (" << _skinCpu._41 << ", " << _skinCpu._42 << ", " << _skinCpu._43 << ")\n";

		DumpMatrix4x4_RowCol(ofs, "Uploaded (transposed):", _skinUploaded);
		ofs << "Uploaded translation(col4) = (" << _skinUploaded._14 << ", " << _skinUploaded._24 << ", " << _skinUploaded._34 << ")\n";

		ofs << "============================================================\n";

		dumped = true;
	}
}
