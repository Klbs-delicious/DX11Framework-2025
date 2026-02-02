/** @file	AnimationImporter.cpp
 *  @brief	Assimpを利用したアニメーションデータ読み込み（新AnimationData型対応）
 *  @date	2026/01/30
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationImporter.h"
#include "Include/Framework/Graphics/AnimationData.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
	//-----------------------------------------------------------------------------
	// Debug dump helpers
	//-----------------------------------------------------------------------------
	static void DumpChannelSourcesToText(
		const std::string& _clipName,
		const std::unordered_map<std::string, std::vector<std::string>>& _baseNameToRawNames)
	{
		std::ofstream ofs("AnimImport_Debug_ChannelSources_" + _clipName + ".txt");
		if (!ofs.is_open())
		{
			return;
		}

		ofs << "=== Channel Sources By Track (baseName) ===\n";
		ofs << "Clip Name: " << _clipName << "\n";
		ofs << "Track(baseName) Count: " << _baseNameToRawNames.size() << "\n\n";

		for (const auto& kv : _baseNameToRawNames)
		{
			const std::string& baseName = kv.first;
			const std::vector<std::string>& raws = kv.second;

			ofs << "--------------------------------------------------\n";
			ofs << "baseName: \"" << baseName << "\"\n";
			ofs << "rawName count: " << raws.size() << "\n";
			for (const auto& raw : raws)
			{
				ofs << "  rawName: \"" << raw << "\"\n";
			}
			ofs << "\n";
		}
	}

	/** @brief アニメーションの内容をテキストに出力するデバッグ関数 */
	static void DumpAnimationClipToText(
		const std::string& _clipName,
		const Graphics::Import::AnimationClip& _clip,
		const std::unordered_map<std::string, std::vector<std::string>>& _baseNameToRawNames)
	{
		std::ofstream ofs("AnimImport_Debug_" + _clip.name + ".txt");
		if (!ofs.is_open())
		{
			return;
		}

		ofs << "=== Animation Clip Debug Dump ===\n";
		ofs << "Clip Name: " << _clip.name << "\n";
		ofs << "Ticks Per Second: " << _clip.ticksPerSecond << "\n";
		ofs << "Duration (Ticks): " << _clip.durationTicks << "\n";
		ofs << "Tracks Count: " << _clip.tracks.size() << "\n\n";

		ofs << "=== Channel Sources Summary ===\n";
		ofs << "Track(baseName) Count: " << _baseNameToRawNames.size() << "\n\n";

		for (const auto& tr : _clip.tracks)
		{
			ofs << "--------------------------------------------------\n";
			ofs << "Track Node Name: " << tr.nodeName << " (Index: " << tr.nodeIndex << ")\n";

			auto it = _baseNameToRawNames.find(tr.nodeName);
			if (it != _baseNameToRawNames.end())
			{
				ofs << "  Raw Channel Names (" << it->second.size() << "):\n";
				for (const auto& raw : it->second)
				{
					ofs << "    \"" << raw << "\"\n";
				}
			}
			else
			{
				ofs << "  Raw Channel Names (0):\n";
			}

			ofs << "\n";

			ofs << "  Position Keys (" << tr.positionKeys.size() << "):\n";
			for (const auto& k : tr.positionKeys)
			{
				ofs << "    [" << std::setw(10) << k.ticksTime << "] ("
					<< k.value.x << ", " << k.value.y << ", " << k.value.z << ")\n";
			}

			ofs << "  Rotation Keys (" << tr.rotationKeys.size() << "):\n";
			for (const auto& k : tr.rotationKeys)
			{
				ofs << "    [" << std::setw(10) << k.ticksTime << "] ("
					<< k.value.x << ", " << k.value.y << ", " << k.value.z << ", " << k.value.w << ")\n";
			}

			ofs << "  Scale Keys (" << tr.scaleKeys.size() << "):\n";
			for (const auto& k : tr.scaleKeys)
			{
				ofs << "    [" << std::setw(10) << k.ticksTime << "] ("
					<< k.value.x << ", " << k.value.y << ", " << k.value.z << ")\n";
			}

			ofs << "\n";
		}

		ofs.close();

		DumpChannelSourcesToText(_clipName, _baseNameToRawNames);
	}
}

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	static bool IsFiniteDouble(double _v)
	{
		return std::isfinite(_v) != 0;
	}

	static bool IsFiniteFloat(float _v)
	{
		return std::isfinite(_v) != 0;
	}

	static double ClampNearZeroToZero(double _t, double _eps)
	{
		if (std::abs(_t) <= _eps)
		{
			return 0.0;
		}
		return _t;
	}

	static bool SanitizeKeyTime(double _t, double& _outT, double _durationHint)
	{
		if (!IsFiniteDouble(_t))
		{
			return false;
		}

		if (std::abs(_t) > 1.0e9)
		{
			return false;
		}

		_t = ClampNearZeroToZero(_t, 1.0e-6);

		if (_durationHint > 0.0)
		{
			const double minT = -1.0e-4;
			const double maxT = _durationHint + 1.0e-4;

			if (_t < minT || _t > maxT)
			{
				if (_t < -1.0 || _t > _durationHint + 1.0)
				{
					return false;
				}

				_t = std::clamp(_t, 0.0, _durationHint);
			}
		}
		else
		{
			if (_t < 0.0)
			{
				_t = 0.0;
			}
		}

		_outT = _t;
		return true;
	}

	static DX::Vector3 AiToDxVec3(const aiVector3D& _v)
	{
		return DX::Vector3(_v.x, _v.y, _v.z);
	}

	static DX::Quaternion AiToDxQuat(const aiQuaternion& _q)
	{
		return DX::Quaternion(_q.x, _q.y, _q.z, _q.w);
	}

	static DX::Quaternion NormalizeDxQuatSafe(const DX::Quaternion& _q)
	{
		DX::Quaternion q = _q;

		const float lenSq = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
		if (!IsFiniteFloat(lenSq) || lenSq <= 1.0e-12f)
		{
			return DX::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}

		const float invLen = 1.0f / std::sqrt(lenSq);
		q.x *= invLen;
		q.y *= invLen;
		q.z *= invLen;
		q.w *= invLen;
		return q;
	}

	static float DotDxQuat(const DX::Quaternion& _a, const DX::Quaternion& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z) + (_a.w * _b.w);
	}

	static DX::Quaternion NegateDxQuat(const DX::Quaternion& _q)
	{
		return DX::Quaternion(-_q.x, -_q.y, -_q.z, -_q.w);
	}

	static void SortVec3Keys(std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		std::sort(
			_keys.begin(),
			_keys.end(),
			[](const Graphics::Import::AnimKeyVec3& _a, const Graphics::Import::AnimKeyVec3& _b)
			{
				return _a.ticksTime < _b.ticksTime;
			});
	}

	static void SortQuatKeys(std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		std::sort(
			_keys.begin(),
			_keys.end(),
			[](const Graphics::Import::AnimKeyQuat& _a, const Graphics::Import::AnimKeyQuat& _b)
			{
				return _a.ticksTime < _b.ticksTime;
			});
	}

	template<typename KeyT>
	static void RemoveDuplicateTimesKeepLast(std::vector<KeyT>& _keys)
	{
		if (_keys.size() <= 1)
		{
			return;
		}

		size_t write = 0;
		for (size_t read = 0; read < _keys.size(); )
		{
			size_t next = read + 1;
			while (next < _keys.size() && _keys[next].ticksTime == _keys[read].ticksTime)
			{
				next++;
			}

			_keys[write] = _keys[next - 1];
			write++;

			read = next;
		}

		_keys.resize(write);
	}

	static std::string GetBaseNodeName(const std::string& _trackName)
	{
		// 名前を削らずにそのまま返す
		return _trackName;
	}

	//static std::string GetBaseNodeName(const std::string& _trackName)
	//{
	//	static const char* suffixes[] =
	//	{
	//		"_$AssimpFbx$_Rotation",
	//		"_$AssimpFbx$_Translation",
	//		"_$AssimpFbx$_Scaling",
	//	};

	//	for (const char* suf : suffixes)
	//	{
	//		const std::string suffix = suf;
	//		if (_trackName.size() <= suffix.size())
	//		{
	//			continue;
	//		}

	//		const size_t pos = _trackName.rfind(suffix);
	//		if (pos != std::string::npos && pos + suffix.size() == _trackName.size())
	//		{
	//			return _trackName.substr(0, pos);
	//		}
	//	}

	//	return _trackName;
	//}

	static bool EndsWith(const std::string& _s, const char* _suffix)
	{
		const std::string suf = _suffix;
		if (_s.size() < suf.size())
		{
			return false;
		}
		return _s.compare(_s.size() - suf.size(), suf.size(), suf) == 0;
	}

	static bool IsRotationOnlyChannelName(const std::string& _rawName)
	{
		return EndsWith(_rawName, "_$AssimpFbx$_Rotation");
	}

	static bool IsTranslationOnlyChannelName(const std::string& _rawName)
	{
		return EndsWith(_rawName, "_$AssimpFbx$_Translation");
	}

	static bool IsScalingOnlyChannelName(const std::string& _rawName)
	{
		return EndsWith(_rawName, "_$AssimpFbx$_Scaling");
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

	static double ComputeClipEndTicksFromKeys(const std::vector<Graphics::Import::NodeTrack>& _tracks)
	{
		double maxTick = 0.0;

		for (const auto& tr : _tracks)
		{
			maxTick = std::max(maxTick, GetMaxTickTime(tr.positionKeys));
			maxTick = std::max(maxTick, GetMaxTickTime(tr.rotationKeys));
			maxTick = std::max(maxTick, GetMaxTickTime(tr.scaleKeys));
		}

		return maxTick;
	}

	static void EnforceQuatHemisphereContinuity(std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		if (_keys.size() <= 1)
		{
			return;
		}

		DX::Quaternion prev = _keys[0].value;
		prev = NormalizeDxQuatSafe(prev);
		_keys[0].value = prev;

		for (size_t i = 1; i < _keys.size(); i++)
		{
			DX::Quaternion q = NormalizeDxQuatSafe(_keys[i].value);

			if (DotDxQuat(prev, q) < 0.0f)
			{
				q = NegateDxQuat(q);
			}

			_keys[i].value = q;
			prev = q;
		}
	}
}

//-----------------------------------------------------------------------------
// Quaternion probe (Hips rotationKeys[0] only once)
//-----------------------------------------------------------------------------
namespace
{
	static void DumpMatrix4x4_4x4(std::ostream& _os, const char* _label, const DirectX::XMFLOAT4X4& _m)
	{
		_os << _label << "\n";
		_os << std::fixed << std::setprecision(6);
		_os << "  [r0] " << _m._11 << ", " << _m._12 << ", " << _m._13 << ", " << _m._14 << "\n";
		_os << "  [r1] " << _m._21 << ", " << _m._22 << ", " << _m._23 << ", " << _m._24 << "\n";
		_os << "  [r2] " << _m._31 << ", " << _m._32 << ", " << _m._33 << ", " << _m._34 << "\n";
		_os << "  [r3] " << _m._41 << ", " << _m._42 << ", " << _m._43 << ", " << _m._44 << "\n";
	}

	static void DumpQuatProbe_HipsOnce(
		const std::string& _clipName,
		double _ticksTime,
		const aiQuaternion& _aiQ,
		const DX::Quaternion& _dxQ)
	{
		static bool sDumped = false;
		if (sDumped)
		{
			return;
		}
		sDumped = true;

		std::ofstream ofs("AnimImport_QuatProbe_Hips.txt", std::ios::app);
		if (!ofs.is_open())
		{
			return;
		}

		ofs << "\n============================================================\n";
		ofs << "[QuatProbe_HipsRotationKey0]\n";
		ofs << "clipName=\"" << _clipName << "\"\n";
		ofs << "ticksTime=" << std::fixed << std::setprecision(6) << _ticksTime << "\n";

		ofs << "aiQuaternion (w,x,y,z) = ("
			<< _aiQ.w << ", " << _aiQ.x << ", " << _aiQ.y << ", " << _aiQ.z << ")\n";

		ofs << "DX::Quaternion (x,y,z,w) = ("
			<< _dxQ.x << ", " << _dxQ.y << ", " << _dxQ.z << ", " << _dxQ.w << ")\n";

		const double lenSq =
			static_cast<double>(_dxQ.x) * static_cast<double>(_dxQ.x) +
			static_cast<double>(_dxQ.y) * static_cast<double>(_dxQ.y) +
			static_cast<double>(_dxQ.z) * static_cast<double>(_dxQ.z) +
			static_cast<double>(_dxQ.w) * static_cast<double>(_dxQ.w);

		const double len = (lenSq > 0.0) ? std::sqrt(lenSq) : 0.0;
		ofs << "DX quat length = " << len << " (lenSq=" << lenSq << ")\n";

		const DirectX::XMVECTOR qv = DirectX::XMVectorSet(_dxQ.x, _dxQ.y, _dxQ.z, _dxQ.w);
		const DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(qv);

		DirectX::XMFLOAT4X4 rf{};
		DirectX::XMStoreFloat4x4(&rf, R);

		DumpMatrix4x4_4x4(ofs, "RotationMatrixFromDXQuat:", rf);

		ofs << "============================================================\n";
	}
}

//-----------------------------------------------------------------------------
// Namespace : Graphics::Import
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	//-----------------------------------------------------------------------------
	// Constructor / Destructor
	//-----------------------------------------------------------------------------
	AnimationImporter::AnimationImporter()
	{
	}

	AnimationImporter::~AnimationImporter()
	{
	}

	//-----------------------------------------------------------------------------
	// LoadSingleClip
	//-----------------------------------------------------------------------------
	bool AnimationImporter::LoadSingleClip(const std::string& _filename, AnimationClip& _outClip) const
	{
		std::vector<AnimationClip> clips{};
		if (!this->LoadClips(_filename, clips))
		{
			return false;
		}

		if (clips.empty())
		{
			return false;
		}

		_outClip = std::move(clips[0]);
		return true;
	}

	//-----------------------------------------------------------------------------
	// LoadClips
	//-----------------------------------------------------------------------------
	bool AnimationImporter::LoadClips(const std::string& _filename, std::vector<AnimationClip>& _outClips) const
	{
		_outClips.clear();

		Assimp::Importer importer;

		// FBXの複雑なピボット構造を維持せず、座標系を整理する
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		// ファイルの単位（cmなど）に関わらず、強制的に特定のスケールに合わせる設定
		// これにより、Assimpがアニメーションキーも一括でスケーリングします
		importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);

		// フラグに aiProcess_GlobalScale を追加する
		const aiScene* scene = importer.ReadFile(_filename,
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GlobalScale |   // ← これが重要！
			aiProcess_LimitBoneWeights
		);
		if (!scene)
		{
			std::cerr << "[AnimationImporter] Assimp Error: " << importer.GetErrorString() << std::endl;
			return false;
		}

		if (!scene->HasAnimations() || scene->mNumAnimations == 0)
		{
			std::cerr << "[AnimationImporter] No animations: " << _filename << std::endl;
			return false;
		}

		_outClips.reserve(scene->mNumAnimations);

		for (unsigned int i = 0; i < scene->mNumAnimations; i++)
		{
			const aiAnimation* anim = scene->mAnimations[i];
			if (!anim)
			{
				continue;
			}

			AnimationClip clip = this->BuildClipFromAssimp(anim);
			_outClips.push_back(std::move(clip));
		}

		return !_outClips.empty();
	}
	// -----------------------------------------------------------------------------
	// BuildClipFromAssimp (ボーン構造維持版)
	// -----------------------------------------------------------------------------
	AnimationClip AnimationImporter::BuildClipFromAssimp(const aiAnimation* _anim) const
	{
		assert(_anim != nullptr);
		AnimationClip out{};
		out.name = (_anim->mName.length > 0) ? _anim->mName.C_Str() : "unknown";
		out.ticksPerSecond = _anim->mTicksPerSecond;
		if (out.ticksPerSecond <= 0.0) out.ticksPerSecond = 30.0;

		out.tracks.reserve(_anim->mNumChannels);

		for (unsigned int c = 0; c < _anim->mNumChannels; c++)
		{
			const aiNodeAnim* ch = _anim->mChannels[c];
			if (!ch) continue;

			std::string boneName = ch->mNodeName.C_Str();
			NodeTrack tr{};
			tr.nodeName = boneName;
			tr.nodeIndex = -1;

			// --- Position Keys ---
			// 【重要】Hips（またはRoot）以外のボーンがPositionアニメを持つと、
			// ボーンがバラバラになったり中心に寄ったりします。
			// 基本的に「Hips」という名前が含まれるボーンのみ移動を許可します。
			bool isRootKey = (boneName.find("Hips") != std::string::npos || boneName.find("Root") != std::string::npos);

			if (ch->mNumPositionKeys > 0 && isRootKey)
			{
				tr.positionKeys.reserve(ch->mNumPositionKeys);
				for (unsigned int k = 0; k < ch->mNumPositionKeys; k++)
				{
					const aiVectorKey& key = ch->mPositionKeys[k];
					double t = 0.0;
					if (SanitizeKeyTime(key.mTime, t, _anim->mDuration))
					{
						tr.positionKeys.emplace_back(t, AiToDxVec3(key.mValue));
					}
				}
			}
			else if (ch->mNumPositionKeys > 0)
			{
				// Hips以外はアニメーションのPositionを捨て、
				// 「静止状態（0フレーム目）」のPositionだけを維持するか、
				// あるいは完全に空にしてBindPose（初期姿勢）に任せます。
				// ここでは空にすることで、システム側の初期姿勢を活かします。
			}

			// --- Rotation Keys (回転は全てのボーンで必要) ---
			if (ch->mNumRotationKeys > 0)
			{
				tr.rotationKeys.reserve(ch->mNumRotationKeys);
				for (unsigned int k = 0; k < ch->mNumRotationKeys; k++)
				{
					const aiQuatKey& key = ch->mRotationKeys[k];
					double t = 0.0;
					if (SanitizeKeyTime(key.mTime, t, _anim->mDuration))
					{
						tr.rotationKeys.emplace_back(t, AiToDxQuat(key.mValue));
					}
				}
			}

			// --- Scaling Keys ---
			// スケールも基本的には 1.0 固定なので、通常は読み飛ばしても安全です。

			tr.hasPosition = !tr.positionKeys.empty();
			tr.hasRotation = !tr.rotationKeys.empty();
			tr.hasScale = !tr.scaleKeys.empty();
			out.tracks.push_back(std::move(tr));
		}

		out.durationTicks = ComputeClipEndTicksFromKeys(out.tracks);
		return out;
	}
} // namespace Graphics::Import
