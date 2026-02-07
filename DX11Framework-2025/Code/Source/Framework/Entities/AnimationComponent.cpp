/** @file   AnimationComponent.cpp
 *  @brief  アニメーション更新専用コンポーネント
 *  @date   2026/01/19
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include "Include/Tests/SkinningDebug.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <DirectXMath.h>

using namespace DirectX;

static_assert(ShaderCommon::MaxBones == 128, "HLSL BoneBuffer boneMatrices[128] と C++ 側 MaxBones が不一致です。");

//-----------------------------------------------------------------------------
// Debug / Temporary settings
//-----------------------------------------------------------------------------
namespace
{
	static constexpr double ForceEndTicksEps = 1.0e-6;	///< 終端判定用
}

//-----------------------------------------------------------------------------
// Local Helpers 
//-----------------------------------------------------------------------------
namespace
{
	/** @brief トラックの終端 ticks を取得する
	 *  @param _keys トラックのキー
	 *  @return 終端 ticks
	 */
	static double GetTrackEndTickVector3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	/** @brief トラックの終端 ticks を取得する
	 *  @param _keys トラックのキー
	 *  @return 終端 ticks
	 */
	static double GetTrackEndTickQuaternion(const std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	/** @brief クリップの終端 ticks をトラックから算出する
	 *  @param _clip 対象クリップ
	 *  @return 終端 ticks
	 */
	static double ComputeClipEndTicksFromTracks(const Graphics::Import::AnimationClip& _clip)
	{
		double maxTick = 0.0;

		for (const auto& track : _clip.tracks)
		{
			maxTick = std::max(maxTick, GetTrackEndTickVector3(track.positionKeys));
			maxTick = std::max(maxTick, GetTrackEndTickQuaternion(track.rotationKeys));
			maxTick = std::max(maxTick, GetTrackEndTickVector3(track.scaleKeys));
		}

		return maxTick;
	}

	/** @brief クリップの安全な終端 ticks を取得する
	 *  @param _clip 対象クリップ
	 *  @return 終端 ticks
	 */
	static double SafeClipEndTicks(const Graphics::Import::AnimationClip* _clip)
	{
		if (!_clip) { return 0.0; }

		const double trackEnd = ComputeClipEndTicksFromTracks(*_clip);
		if (trackEnd > ForceEndTicksEps) { return trackEnd; }

		if (_clip->durationTicks > 0.0) { return _clip->durationTicks; }

		return 0.0;
	}

	/** @brief スケルトンキャッシュからバインド姿勢のグローバル行列群を構築する
	 *  @param[out] _outBindGlobal 出力先配列
	 *  @param _cache スケルトンキャッシュ
	 */
	static void BuildBindGlobalMatrices(
		std::vector<DX::Matrix4x4>& _outBindGlobal,
		const Graphics::Import::SkeletonCache& _cache)
	{
		const size_t nodeCount = _cache.nodes.size();
		_outBindGlobal.assign(nodeCount, DX::Matrix4x4::Identity);

		for (int nodeIndex : _cache.order)
		{
			const auto& node = _cache.nodes[static_cast<size_t>(nodeIndex)];

			if (node.parentIndex < 0)
			{
				_outBindGlobal[static_cast<size_t>(nodeIndex)] = node.bindLocalMatrix;
			}
			else
			{
				_outBindGlobal[static_cast<size_t>(nodeIndex)] =
					node.bindLocalMatrix * _outBindGlobal[static_cast<size_t>(node.parentIndex)];
			}
		}
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent Methods
//-----------------------------------------------------------------------------
AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive),
	isLoop(false),
	isPlaying(false),
	currentClip(nullptr),
	currentTime(0.0),
	playbackSpeed(1.0f),
	clipEndTicks(0.0),
	isSkeletonCached(false)
{
}

void AnimationComponent::Initialize()
{
	if (!this->boneCB)
	{
		auto& d3d = SystemLocator::Get<D3D11System>();
		this->boneCB = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();
		this->boneCB->Create(d3d.GetDevice());
	}

	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();
	if (!this->meshComponent)
	{
		this->meshComponent = this->Owner()->AddComponent<MeshComponent>();
	}

	this->currentTime = 0.0;
	this->playbackSpeed = 1.0f;
	this->clipEndTicks = 0.0;

	this->bindGlobalMatrices.clear();
}

void AnimationComponent::Dispose()
{
	this->currentClip = nullptr;
	this->meshComponent = nullptr;

	this->isSkeletonCached = false;

	this->boneCB.reset();
}

void AnimationComponent::SetSkeletonCache(const Graphics::Import::SkeletonCache* _cache)
{
	this->skeletonCache = _cache;

	if (!this->skeletonCache)
	{
		this->isSkeletonCached = false;
		return;
	}

	// 読み込み時の検証ログのみ許可
	if (Graphics::Debug::Config::IsImportDumpEnabled())
	{
		Graphics::Debug::Output::DumpSkeletonImportCheck(
			Graphics::Debug::Config::SkeletonImportDumpPath,
			*this->skeletonCache,
			5);

		Graphics::Debug::Output::DumpSkeletonOrderCheck(
			Graphics::Debug::Config::SkeletonOrderDumpPath,
			*this->skeletonCache);
	}

	this->isSkeletonCached = true;
	this->currentPose.Reset(*this->skeletonCache);

	this->bindGlobalMatrices.clear();
	BuildBindGlobalMatrices(this->bindGlobalMatrices, *this->skeletonCache);

	// clip が存在して未焼き込みならここで焼く
	if (this->currentClip && !this->currentClip->IsBaked())
	{
		this->currentClip->BakeNodeIndices(*this->skeletonCache);

		if (Graphics::Debug::Config::IsImportDumpEnabled())
		{
			Graphics::Debug::Output::DumpTrackBakeStatus(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				this->skeletonCache,
				"after BakeNodeIndices in SetSkeletonCache");

			Graphics::Debug::Output::DumpBakeValidationOnce(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				*this->skeletonCache,
				"after BakeNodeIndices in SetSkeletonCache");
		}
	}
	else
	{
		if (Graphics::Debug::Config::IsImportDumpEnabled() && this->currentClip)
		{
			Graphics::Debug::Output::DumpTrackBakeStatus(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				this->skeletonCache,
				"SetSkeletonCache no bake");

			Graphics::Debug::Output::DumpBakeValidationOnce(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				*this->skeletonCache,
				"SetSkeletonCache no bake");
		}
	}

	// クリップ終端はトラックから算出する
	this->clipEndTicks = SafeClipEndTicks(this->currentClip);
}

void AnimationComponent::SetAnimationClip(Graphics::Import::AnimationClip* _clip)
{
	this->currentClip = _clip;

	if (this->currentClip && this->isSkeletonCached && this->skeletonCache && !this->currentClip->IsBaked())
	{
		this->currentClip->BakeNodeIndices(*this->skeletonCache);

		if (Graphics::Debug::Config::IsImportDumpEnabled())
		{
			Graphics::Debug::Output::DumpTrackBakeStatus(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				this->skeletonCache,
				"after BakeNodeIndices in SetAnimationClip");

			Graphics::Debug::Output::DumpBakeValidationOnce(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				*this->skeletonCache,
				"after BakeNodeIndices in SetAnimationClip");
		}
	}
	else
	{
		if (Graphics::Debug::Config::IsImportDumpEnabled() && this->currentClip && this->skeletonCache)
		{
			Graphics::Debug::Output::DumpTrackBakeStatus(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				this->skeletonCache,
				"SetAnimationClip no bake");

			Graphics::Debug::Output::DumpBakeValidationOnce(
				Graphics::Debug::Config::PoseDumpFilePath,
				*this->currentClip,
				*this->skeletonCache,
				"SetAnimationClip no bake");
		}
	}

	// クリップ終端はトラックから算出する
	this->clipEndTicks = SafeClipEndTicks(this->currentClip);

	// 詳細ログは読み込み直後のみ
	if (Graphics::Debug::Config::IsImportDumpEnabled() && this->currentClip)
	{
		std::ofstream ofs(Graphics::Debug::Config::AnimDetailDumpPath);
		if (ofs.is_open())
		{
			ofs << "=== Animation Clip Detail Check ===\n";
			ofs << "Clip Name: " << this->currentClip->name << "\n";
			ofs << "DurationTicks: " << this->currentClip->durationTicks << " TicksPerSecond: " << this->currentClip->ticksPerSecond << "\n";
			ofs << "ComputedEndTicksFromTracks: " << this->clipEndTicks << "\n";
			ofs << "Track Count: " << this->currentClip->tracks.size() << "\n\n";

			for (const auto& track : this->currentClip->tracks)
			{
				if (track.nodeName.find("Hips") != std::string::npos || track.nodeIndex < 5)
				{
					ofs << "Track: " << track.nodeName << " NodeIndex: " << track.nodeIndex << "\n";

					if (!track.positionKeys.empty())
					{
						const auto& pk = track.positionKeys[0];
						ofs << "  First PosKey Time: " << pk.ticksTime
							<< " Val: (" << pk.value.x << ", " << pk.value.y << ", " << pk.value.z << ")\n";
					}
					else
					{
						ofs << "  No PosKeys\n";
					}

					if (!track.rotationKeys.empty())
					{
						const auto& rk = track.rotationKeys[0];
						ofs << "  First RotKey Time: " << rk.ticksTime
							<< " Val: (" << rk.value.x << ", " << rk.value.y << ", " << rk.value.z << ", " << rk.value.w << ")\n";
					}
					else
					{
						ofs << "  No RotKeys\n";
					}

					if (!track.scaleKeys.empty())
					{
						const auto& sk = track.scaleKeys[0];
						ofs << "  First SclKey Time: " << sk.ticksTime
							<< " Val: (" << sk.value.x << ", " << sk.value.y << ", " << sk.value.z << ")\n";
					}
					else
					{
						ofs << "  No SclKeys\n";
					}

					ofs << "\n";
				}
			}
		}
	}
}

void AnimationComponent::Play()
{
	if (!this->currentClip) { return; }
	if (!this->isSkeletonCached) { return; }

	this->isPlaying = true;
}

void AnimationComponent::Stop()
{
	this->isPlaying = false;
}

const std::string& AnimationComponent::GetCurrentAnimationName() const
{
	static const std::string empty{};
	if (!this->currentClip)
	{
		return empty;
	}
	return this->currentClip->name;
}

void AnimationComponent::FixedUpdate(float _deltaTime)
{
	if (!this->isSkeletonCached) { return; }
	if (!this->skeletonCache) { return; }
	if (this->skeletonCache->nodes.empty()) { return; }

	// 再生中だけ時間を進める
	if (this->isPlaying && this->currentClip)
	{
		const double dt = static_cast<double>(_deltaTime) * static_cast<double>(this->playbackSpeed);
		const double tps = this->currentClip->ticksPerSecond;

		if (tps > 0.0)
		{
			double nextTime = this->currentTime + dt;

			// トラックから算出した終端で回す
			const double endTicks = (this->clipEndTicks > ForceEndTicksEps) ? this->clipEndTicks : this->currentClip->durationTicks;
			const double endSec = (endTicks > ForceEndTicksEps) ? (endTicks / tps) : 0.0;

			if (endSec > 0.0)
			{
				if (this->isLoop)
				{
					nextTime = std::fmod(nextTime, endSec);
					if (nextTime < 0.0) { nextTime += endSec; }
				}
				else
				{
					if (nextTime >= endSec)
					{
						nextTime = endSec;
						this->isPlaying = false;
					}
					if (nextTime < 0.0) { nextTime = 0.0; }
				}
			}

			this->currentTime = nextTime;
		}
	}
	else
	{
		// 再生していない場合は時間を進めない
		if (!this->currentClip)
		{
			this->currentTime = 0.0;
		}
		else
		{
			const double tps = this->currentClip->ticksPerSecond;
			if (tps > 0.0)
			{
				// トラックから算出した終端でクランプする
				const double endTicks = (this->clipEndTicks > ForceEndTicksEps) ? this->clipEndTicks : this->currentClip->durationTicks;
				const double endSec = (endTicks > ForceEndTicksEps) ? (endTicks / tps) : 0.0;

				if (!this->isLoop && endSec > 0.0 && this->currentTime > endSec)
				{
					this->currentTime = endSec;
				}
			}
		}
	}

	// ポーズ更新
	if (this->currentClip)
	{
		this->UpdatePoseFromClip(this->currentTime);
	}
	else
	{
		// クリップ未設定時は bind pose を表示
		this->currentPose.Reset(*this->skeletonCache);
	}

	//-----------------------------------------------------------------------------
	// ボーン行列更新
	//-----------------------------------------------------------------------------

	// アップロードするボーン数を決定する
	const uint32_t actualBoneCount = static_cast<uint32_t>(this->skeletonCache->boneIndexToNodeIndex.size());
	const uint32_t uploadBoneCount = std::min<uint32_t>(actualBoneCount, static_cast<uint32_t>(ShaderCommon::MaxBones));
	this->boneBuffer.boneCount = uploadBoneCount;

	// 初期化
	for (size_t i = 0; i < ShaderCommon::MaxBones; ++i)
	{
		this->boneBuffer.boneMatrices[i] = DX::Matrix4x4::Identity;
	}

	// ボーン行列を更新
	for (uint32_t i = 0; i < uploadBoneCount; ++i)
	{
		this->boneBuffer.boneMatrices[i] = DX::TransposeMatrix(this->currentPose.cpuBoneMatrices[static_cast<size_t>(i)]);
	}

	// 定数バッファを更新
	if (this->boneCB)
	{
		auto& d3d = SystemLocator::Get<D3D11System>();
		this->boneCB->Update(d3d.GetContext(), this->boneBuffer);
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent::UpdatePoseFromClip
//-----------------------------------------------------------------------------
void AnimationComponent::UpdatePoseFromClip(double _timeSeconds)
{
	if (!this->currentClip) { return; }
	if (!this->skeletonCache) { return; }

	const double ticksPerSecond = this->currentClip->ticksPerSecond;

	// 秒を ticks に換算
	// ticksPerSecond が 0 の場合は ticks=0 として扱う（挙動は既存に近い）
	double ticks = 0.0;
	if (ticksPerSecond > 0.0)
	{
		ticks = _timeSeconds * ticksPerSecond;
	}

	// トラックから算出した終端で回す
	const double endTicks =
		(this->clipEndTicks > ForceEndTicksEps) ? this->clipEndTicks : this->currentClip->durationTicks;

	if (this->isLoop)
	{
		if (endTicks > ForceEndTicksEps)
		{
			ticks = std::fmod(ticks, endTicks);
			if (ticks < 0.0) { ticks += endTicks; }
		}
	}
	else
	{
		ticks = std::min(ticks, endTicks);
	}

	const size_t nodeCount = this->skeletonCache->nodes.size();

	//-----------------------------------------------------------------------------
	// キーから Local 行列を更新
	//-----------------------------------------------------------------------------

	// 初期化
	for (size_t i = 0; i < nodeCount; ++i)
	{
		this->currentPose.localMatrices[i] = this->skeletonCache->nodes[i].bindLocalMatrix;
	}

	// トラックごとに Local 行列を更新
	// nodeIndex が同じ場合は最後に処理したものが優先される
	for (const auto& track : this->currentClip->tracks)
	{
		const int nodeIndex = track.nodeIndex;
		if (nodeIndex < 0) { continue; }
		if (nodeIndex >= static_cast<int>(nodeCount)) { continue; }

		const size_t nodeIdx = static_cast<size_t>(nodeIndex);
		this->UpdateLocalMatrixFromKeys(nodeIdx, ticks, track);
	}

	//-----------------------------------------------------------------------------
	// Global 行列の更新
	//-----------------------------------------------------------------------------

	// order は親子順が前提なのでこの順に合成する
	for (int nodeIndex : this->skeletonCache->order)
	{
		const size_t nodeIdx = static_cast<size_t>(nodeIndex);
		const int parentIndex = this->skeletonCache->nodes[nodeIdx].parentIndex;

		if (parentIndex < 0)
		{
			this->currentPose.globalMatrices[nodeIdx] =
				this->currentPose.localMatrices[nodeIdx];
		}
		else
		{
			const size_t parentIdx = static_cast<size_t>(parentIndex);

			this->currentPose.globalMatrices[nodeIdx] =
				this->currentPose.localMatrices[nodeIdx] *
				this->currentPose.globalMatrices[parentIdx];
		}
	}

	//-----------------------------------------------------------------------------
	// スキニング行列計算
	//-----------------------------------------------------------------------------

	// 原則 offset * global[node] * rootInv
	for (size_t boneIdx = 0; boneIdx < this->skeletonCache->boneIndexToNodeIndex.size(); ++boneIdx)
	{
		if (boneIdx >= ShaderCommon::MaxBones) { break; }

		const int nodeIndex = this->skeletonCache->boneIndexToNodeIndex[boneIdx];
		const DX::Matrix4x4& offset = this->skeletonCache->boneOffset[boneIdx];

		this->currentPose.cpuBoneMatrices[boneIdx] =
			offset *
			this->currentPose.globalMatrices[static_cast<size_t>(nodeIndex)] *
			this->skeletonCache->globalInverse;
	}
}

//-----------------------------------------------------------------------------
// Key interpolation helpers
//-----------------------------------------------------------------------------
DX::Vector3 AnimationComponent::InterpolateTranslation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback)
{
	if (!_track) { return _fallback; }

	const auto& keys = _track->positionKeys;
	if (keys.empty()) { return _fallback; }
	if (keys.size() == 1) { return keys[0].value; }

	size_t rightIdx = 0;
	for (; rightIdx < keys.size(); ++rightIdx)
	{
		if (static_cast<float>(keys[rightIdx].ticksTime) > _ticks) { break; }
	}

	if (rightIdx == keys.size()) { return keys.back().value; }
	if (rightIdx == 0) { return keys.front().value; }

	const auto& leftKey = keys[rightIdx - 1];
	const auto& rightKey = keys[rightIdx];

	const float denom = static_cast<float>(rightKey.ticksTime - leftKey.ticksTime);
	if (denom <= 1.0e-6f) { return leftKey.value; }

	const float t = (_ticks - static_cast<float>(leftKey.ticksTime)) / denom;
	return DX::Vector3::Lerp(leftKey.value, rightKey.value, t);
}

DX::Quaternion AnimationComponent::InterpolateRotation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Quaternion& _fallback)
{
	if (!_track) { return _fallback; }

	const auto& keys = _track->rotationKeys;
	if (keys.empty()) { return _fallback; }
	if (keys.size() == 1) { return keys[0].value; }

	size_t rightIdx = 0;
	for (; rightIdx < keys.size(); ++rightIdx)
	{
		if (static_cast<float>(keys[rightIdx].ticksTime) > _ticks) { break; }
	}

	if (rightIdx == keys.size()) { return keys.back().value; }
	if (rightIdx == 0) { return keys.front().value; }

	const auto& leftKey = keys[rightIdx - 1];
	const auto& rightKey = keys[rightIdx];

	const float denom = static_cast<float>(rightKey.ticksTime - leftKey.ticksTime);
	if (denom <= 1.0e-6f) { return leftKey.value; }

	const float t = (_ticks - static_cast<float>(leftKey.ticksTime)) / denom;
	return DX::SlerpQuaternionSimple(leftKey.value, rightKey.value, t);
}

DX::Vector3 AnimationComponent::InterpolateScale(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback)
{
	if (!_track) { return _fallback; }

	const auto& keys = _track->scaleKeys;
	if (keys.empty()) { return _fallback; }
	if (keys.size() == 1) { return keys[0].value; }

	size_t rightIdx = 0;
	for (; rightIdx < keys.size(); ++rightIdx)
	{
		if (static_cast<float>(keys[rightIdx].ticksTime) > _ticks) { break; }
	}

	if (rightIdx == keys.size()) { return keys.back().value; }
	if (rightIdx == 0) { return keys.front().value; }

	const auto& leftKey = keys[rightIdx - 1];
	const auto& rightKey = keys[rightIdx];

	const float denom = static_cast<float>(rightKey.ticksTime - leftKey.ticksTime);
	if (denom <= 1.0e-6f) { return leftKey.value; }

	const float t = (_ticks - static_cast<float>(leftKey.ticksTime)) / denom;
	return DX::Vector3::Lerp(leftKey.value, rightKey.value, t);
}

//-----------------------------------------------------------------------------
// Constant buffer bind
//-----------------------------------------------------------------------------
void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!_context) { return; }
	if (!this->boneCB) { return; }

	ID3D11Buffer* buffer = this->boneCB->GetBuffer();
	if (!buffer) { return; }

	_context->VSSetConstantBuffers(_slot, 1, &buffer);
}

//-----------------------------------------------------------------------------
// Local matrix update from keys
//-----------------------------------------------------------------------------
void AnimationComponent::UpdateLocalMatrixFromKeys(size_t _nodeIdx, double _ticks, const Graphics::Import::NodeTrack& _track)
{
	auto& nodeInfo = this->skeletonCache->nodes[_nodeIdx];

	// デフォルト値として Bind 姿勢を分解して保持する
	// アニメにキーがない成分は Bind の値を維持して骨の長さを保つ
	XMMATRIX bindLocalMatrix = DX::LoadXMMATRIX(nodeInfo.bindLocalMatrix);
	XMVECTOR bindLocalScale{}, bindLocalRotation{}, bindLocalTranslation{};
	XMMatrixDecompose(&bindLocalScale, &bindLocalRotation, &bindLocalTranslation, bindLocalMatrix);



	DX::Vector3 finalPos{};
	DX::Vector3 finalScale{};
	DX::Quaternion finalRot{};
	{
		XMFLOAT3 posFloat{};
		XMFLOAT4 rotFloat{};
		XMFLOAT3 scaleFloat{};

		// Bind を float 型に変換しておく
		XMStoreFloat3(&posFloat, bindLocalTranslation);
		XMStoreFloat4(&rotFloat, bindLocalRotation);
		XMStoreFloat3(&scaleFloat, bindLocalScale);

		// 初期値セット
		finalPos = DX::ToDXVector3(posFloat);
		finalRot = DX::ToDXQuaternion(rotFloat);
		finalScale = DX::ToDXVector3(scaleFloat);
	}

	// キーがある成分だけ上書きし ない成分は Bind を維持する
	if (_track.hasPosition)
	{
		finalPos = this->InterpolateTranslation(&_track, static_cast<float>(_ticks), finalPos);
	}

	if (_track.hasRotation)
	{
		finalRot = this->InterpolateRotation(&_track, static_cast<float>(_ticks), finalRot);
	}

	if (_track.hasScale)
	{
		finalScale = this->InterpolateScale(&_track, static_cast<float>(_ticks), finalScale);
	}

	{
		// TRS を再構成する
		// ここは既存の実装と同じ合成順で維持する
		const XMFLOAT3 scaleFloat = DX::ToXMFLOAT3(finalScale);
		const XMFLOAT4 rotFloat = DX::ToXMFLOAT4(finalRot);
		const XMFLOAT3 posFloat = DX::ToXMFLOAT3(finalPos);

		XMMATRIX finalMatrix =
			XMMatrixScalingFromVector(XMLoadFloat3(&scaleFloat)) *
			XMMatrixRotationQuaternion(XMLoadFloat4(&rotFloat)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&posFloat));

		this->currentPose.localMatrices[_nodeIdx] = DX::StoreDXMatrix(finalMatrix);
	}
}