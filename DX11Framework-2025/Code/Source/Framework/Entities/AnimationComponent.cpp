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

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <DirectXMath.h>

using namespace DirectX;

static_assert(ShaderCommon::MaxBones == 128, "HLSL BoneBuffer boneMatrices[128] と C++ 側 MaxBones が不一致です。");

//-----------------------------------------------------------------------------
// Debug / Temporary settings
//---------------------------------------------------
// --------------------------
namespace
{
	static constexpr double ForceEndTicks = 81.0;		///< 暫定：81固定で挙動確認
	static constexpr double ForceEndTicksEps = 1.0e-6;	///< 暫定：終端判定用イプシロン

	static constexpr bool EnablePoseDump = true;			///< true の間、毎フレーム txt 出力
	static constexpr const char* PoseDumpFilePath = "PoseDump_AnimationComponent.txt";
	static int PoseDumpFrameIndex = 0;

	static constexpr bool EnableTrackPickDumpOnce = true;
	static bool TrackPickDumped = false;

	static constexpr bool EnableBindVsAnimDumpOnce = true;
	static bool BindVsAnimDumped = false;

	static constexpr bool EnableGlobalInverseCheckOnce = true;
	static bool GlobalInverseChecked = false;

	static constexpr bool EnableBoneOffsetCheckOnce = true;
	static bool BoneOffsetChecked = false;

	static constexpr bool EnableBakeValidationDumpOnce = true;
	static bool BakeValidationDumped = false;

	static constexpr bool EnableLocalKeyAppliedDumpOnce = true;
	static bool LocalKeyAppliedDumped = false;
}

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace
{
	static int FindKeyIndex(double _ticksTime, const std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		if (_keys.size() < 2) { return 0; }
		if (_ticksTime >= _keys.back().ticksTime) { return static_cast<int>(_keys.size()) - 2; }
		if (_ticksTime <= _keys.front().ticksTime) { return 0; }

		auto it = std::lower_bound(
			_keys.begin(),
			_keys.end(),
			_ticksTime,
			[](const Graphics::Import::AnimKeyVec3& _k, double _t)
			{
				return _k.ticksTime < _t;
			});

		const int idx = static_cast<int>(std::distance(_keys.begin(), it)) - 1;
		return (idx < 0) ? 0 : idx;
	}

	static int FindKeyIndex(double _ticksTime, const std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		if (_keys.size() < 2) { return 0; }
		if (_ticksTime >= _keys.back().ticksTime) { return static_cast<int>(_keys.size()) - 2; }
		if (_ticksTime <= _keys.front().ticksTime) { return 0; }

		auto it = std::lower_bound(
			_keys.begin(),
			_keys.end(),
			_ticksTime,
			[](const Graphics::Import::AnimKeyQuat& _k, double _t)
			{
				return _k.ticksTime < _t;
			});

		const int idx = static_cast<int>(std::distance(_keys.begin(), it)) - 1;
		return (idx < 0) ? 0 : idx;
	}

	static float DotQuat(const DX::Quaternion& _a, const DX::Quaternion& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z) + (_a.w * _b.w);
	}

	static DX::Quaternion NormalizeQuatSafe(const DX::Quaternion& _q)
	{
		DX::Quaternion q = _q;

		const float lenSq = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
		if (!std::isfinite(lenSq) || lenSq <= 1.0e-12f)
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

	static DX::Quaternion NegateQuat(const DX::Quaternion& _q)
	{
		return DX::Quaternion(-_q.x, -_q.y, -_q.z, -_q.w);
	}

	static DX::Quaternion LerpQuat(const DX::Quaternion& _a, const DX::Quaternion& _b, float _t)
	{
		return DX::Quaternion(
			_a.x + (_b.x - _a.x) * _t,
			_a.y + (_b.y - _a.y) * _t,
			_a.z + (_b.z - _a.z) * _t,
			_a.w + (_b.w - _a.w) * _t);
	}

	/** @brief DX::Quaternion の球面補間（分かりやすさ優先）
	 *  @param _q0 始点
	 *  @param _q1 終点
	 *  @param _t 0～1
	 *  @return 補間結果（正規化済み）
	 */
	static DX::Quaternion SlerpQuatSimple(const DX::Quaternion& _q0, const DX::Quaternion& _q1, float _t)
	{
		DX::Quaternion q0 = NormalizeQuatSafe(_q0);
		DX::Quaternion q1 = NormalizeQuatSafe(_q1);

		float dot = DotQuat(q0, q1);

		if (dot < 0.0f)
		{
			q1 = NegateQuat(q1);
			dot = -dot;
		}

		if (dot > 0.9995f)
		{
			return NormalizeQuatSafe(LerpQuat(q0, q1, _t));
		}

		dot = std::clamp(dot, -1.0f, 1.0f);

		const float theta = std::acos(dot);
		const float sinTheta = std::sin(theta);

		if (std::abs(sinTheta) <= 1.0e-6f)
		{
			return NormalizeQuatSafe(LerpQuat(q0, q1, _t));
		}

		const float w0 = std::sin((1.0f - _t) * theta) / sinTheta;
		const float w1 = std::sin(_t * theta) / sinTheta;

		DX::Quaternion out(
			(q0.x * w0) + (q1.x * w1),
			(q0.y * w0) + (q1.y * w1),
			(q0.z * w0) + (q1.z * w1),
			(q0.w * w0) + (q1.w * w1));

		return NormalizeQuatSafe(out);
	}

	static DX::Matrix4x4 TransposeDxMatrix(const DX::Matrix4x4& _m)
	{
		XMFLOAT4X4 f{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&f, &_m, sizeof(XMFLOAT4X4));

		const XMMATRIX xm = XMLoadFloat4x4(&f);
		const XMMATRIX t = XMMatrixTranspose(xm);

		XMFLOAT4X4 outF{};
		XMStoreFloat4x4(&outF, t);

		DX::Matrix4x4 out{};
		std::memcpy(&out, &outF, sizeof(XMFLOAT4X4));
		return out;
	}

	/** @brief TRS 行列を作成する（行ベクトル版）
	 *  @param _t 平行移動成分
	 *  @param _r 回転成分
	 *  @param _s スケーリング成分
	 *  @return 作成した行列
	 */
	static DX::Matrix4x4 MakeTRS_RowVector(const DX::Vector3& _t, const DX::Quaternion& _r, const DX::Vector3& _s)
	{
		const XMMATRIX S = XMMatrixScaling(_s.x, _s.y, _s.z);
		const XMMATRIX R = XMMatrixRotationQuaternion(XMVectorSet(_r.x, _r.y, _r.z, _r.w));
		const XMMATRIX T = XMMatrixTranslation(_t.x, _t.y, _t.z);

		const XMMATRIX m = S * R * T;

		XMFLOAT4X4 f{};
		XMStoreFloat4x4(&f, m);

		DX::Matrix4x4 out{};
		std::memcpy(&out, &f, sizeof(XMFLOAT4X4));
		return out;
	}

	/** @brief 行列を TRS 成分に分解する（行ベクトル版）
	 *  @param _bindLocal 分解対象の行列
	 *  @param _outT 平行移動成分の出力先
	 *  @param _outR 回転成分の出力先
	 *  @param _outS スケーリング成分の出力先
	 *  @return 分解に成功した場合は true、失敗した場合は false を返す
	 */
	static bool DecomposeBindLocalTRS(
		const DX::Matrix4x4& _bindLocal,
		DX::Vector3& _outT,
		DX::Quaternion& _outR,
		DX::Vector3& _outS)
	{
		XMFLOAT4X4 f{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&f, &_bindLocal, sizeof(XMFLOAT4X4));

		const XMMATRIX m = XMLoadFloat4x4(&f);

		XMVECTOR s{};
		XMVECTOR r{};
		XMVECTOR t{};
		const bool ok = (XMMatrixDecompose(&s, &r, &t, m) != 0);
		if (!ok)
		{
			_outT = DX::Vector3(0, 0, 0);
			_outR = DX::Quaternion(0, 0, 0, 1);
			_outS = DX::Vector3(1, 1, 1);
			return false;
		}

		XMFLOAT3 tf{};
		XMStoreFloat3(&tf, t);
		_outT = DX::Vector3(tf.x, tf.y, tf.z);

		XMFLOAT3 sf{};
		XMStoreFloat3(&sf, s);
		_outS = DX::Vector3(sf.x, sf.y, sf.z);

		XMFLOAT4 qf{};
		XMStoreFloat4(&qf, r);
		_outR = NormalizeQuatSafe(DX::Quaternion(qf.x, qf.y, qf.z, qf.w));

		return true;
	}

	static DX::Matrix4x4 InverseDxMatrix(const DX::Matrix4x4& _m)
	{
		XMFLOAT4X4 f{};
		std::memcpy(&f, &_m, sizeof(XMFLOAT4X4));

		const XMMATRIX xm = XMLoadFloat4x4(&f);
		const XMMATRIX inv = XMMatrixInverse(nullptr, xm);

		XMFLOAT4X4 outF{};
		XMStoreFloat4x4(&outF, inv);

		DX::Matrix4x4 out{};
		std::memcpy(&out, &outF, sizeof(XMFLOAT4X4));
		return out;
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

		_ofs << "  col4(translation) = (" << _m._14 << ", " << _m._24 << ", " << _m._34 << ")\n";
	}

	static double GetTrackEndTickVec3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	static double GetTrackEndTickQuat(const std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	static double ComputeClipEndTicksFromKeys(const Graphics::Import::AnimationClip& _clip)
	{
		double maxTick = 0.0;

		for (const auto& tr : _clip.tracks)
		{
			const double tp = GetTrackEndTickVec3(tr.positionKeys);
			const double trt = GetTrackEndTickQuat(tr.rotationKeys);
			const double ts = GetTrackEndTickVec3(tr.scaleKeys);

			maxTick = std::max(maxTick, tp);
			maxTick = std::max(maxTick, trt);
			maxTick = std::max(maxTick, ts);
		}

		return maxTick;
	}

	static double SafeClipEndTicks(const Graphics::Import::AnimationClip* _clip)
	{
		if (!_clip) { return 0.0; }

		const double keyEnd = ComputeClipEndTicksFromKeys(*_clip);
		if (keyEnd > 0.0) { return keyEnd; }

		return _clip->durationTicks;
	}

	static void BuildBindGlobalMatrices(
		std::vector<DX::Matrix4x4>& _outBindGlobal,
		const Graphics::Import::SkeletonCache& _cache)
	{
		const size_t nodeCount = _cache.nodes.size();
		_outBindGlobal.assign(nodeCount, DX::Matrix4x4::Identity); // 全ノード分確保

		// cache.order は親→子の順序が保証されている前提
		for (int nodeIndex : _cache.order)
		{
			const auto& node = _cache.nodes[nodeIndex];
			if (node.parentIndex < 0) {
				_outBindGlobal[nodeIndex] = node.bindLocalMatrix;
			}
			else {
				// 行ベクトル形式: 子 = 子Local * 親Global
				_outBindGlobal[nodeIndex] = node.bindLocalMatrix * _outBindGlobal[node.parentIndex];
			}
		}
	}

	static void DumpTrackBakeStatus(
		const char* _filePath,
		const Graphics::Import::AnimationClip* _clip,
		const Graphics::Import::SkeletonCache* _skeletonCache,
		const char* _tag)
	{
		if (!_filePath) { return; }
		if (!_clip) { return; }

		std::ofstream ofs(_filePath, std::ios::app);
		if (!ofs.is_open()) { return; }

		ofs << "\n============================================================\n";
		ofs << "[TrackBakeStatus] " << (_tag ? _tag : "") << "\n";
		ofs << "clipName=\"" << _clip->name << "\"\n";
		ofs << "isBaked=" << (_clip->IsBaked() ? "true" : "false") << "\n";
		ofs << "trackCount=" << _clip->tracks.size() << "\n";

		if (_skeletonCache)
		{
			ofs << "nodeCount=" << _skeletonCache->nodes.size() << "\n";
		}

		int unresolvedCount = 0;

		for (size_t ti = 0; ti < _clip->tracks.size(); ++ti)
		{
			const auto& tr = _clip->tracks[ti];

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
				ofs << " [UNRESOLVED]";
			}

			ofs << "\n";
		}

		ofs << "unresolvedCount=" << unresolvedCount << "\n";
		ofs << "============================================================\n";
	}

	static void DumpBakeValidationOnce(
		const char* _filePath,
		const Graphics::Import::AnimationClip* _clip,
		const Graphics::Import::SkeletonCache* _skeletonCache,
		const char* _tag)
	{
		if (!EnableBakeValidationDumpOnce) { return; }
		if (BakeValidationDumped) { return; }

		if (!_filePath) { return; }
		if (!_clip) { return; }
		if (!_skeletonCache) { return; }

		std::ofstream ofs(_filePath, std::ios::app);
		if (!ofs.is_open()) { return; }

		ofs << "\n============================================================\n";
		ofs << "[BakeValidationOnce] " << (_tag ? _tag : "") << "\n";
		ofs << "clipName=\"" << _clip->name << "\"\n";
		ofs << "isBaked=" << (_clip->IsBaked() ? "true" : "false") << "\n";
		ofs << "trackCount=" << _clip->tracks.size() << "\n";
		ofs << "nodeCount=" << _skeletonCache->nodes.size() << "\n";

		std::unordered_map<std::string, std::vector<int>> nodeNameToIndices{};
		nodeNameToIndices.reserve(_skeletonCache->nodes.size());

		for (int i = 0; i < static_cast<int>(_skeletonCache->nodes.size()); ++i)
		{
			nodeNameToIndices[_skeletonCache->nodes[static_cast<size_t>(i)].name].push_back(i);
		}

		int duplicateNameCount = 0;
		size_t duplicateNodesTotal = 0;

		for (const auto& kv : nodeNameToIndices)
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

		BakeValidationDumped = true;
	}

	static void DumpBoneMatrixCpuGpuPairOnce(
		const char* _filePath,
		int _boneIndex,
		const DX::Matrix4x4& _skinCpu,
		const DX::Matrix4x4& _skinUploaded)
	{
		std::ofstream ofs(_filePath, std::ios::app);
		if (!ofs.is_open()) { return; }

		ofs << "\n============================================================\n";
		ofs << "[BoneMatrixCpuGpuPairOnce]\n";
		ofs << "boneIndex=" << _boneIndex << "\n";

		DumpMatrix4x4_RowCol(ofs, "CPU skin (no transpose):", _skinCpu);
		ofs << "CPU translation(row4) = (" << _skinCpu._41 << ", " << _skinCpu._42 << ", " << _skinCpu._43 << ")\n";

		DumpMatrix4x4_RowCol(ofs, "Uploaded (transposed):", _skinUploaded);
		ofs << "Uploaded translation(col4) = (" << _skinUploaded._14 << ", " << _skinUploaded._24 << ", " << _skinUploaded._34 << ")\n";

		ofs << "============================================================\n";
	}

	static void DumpNodeCalculation(std::ofstream& _ofs, int _idx, const std::string& _name,
		const DX::Matrix4x4& _bind, const DX::Matrix4x4& _anim, const char* _type) {

		_ofs << "--- Node[" << _idx << "]: " << _name << " (" << _type << ") ---\n";
		_ofs << "  Bind: " << _bind._41 << ", " << _bind._42 << ", " << _bind._43 << " (Trans)\n";
		_ofs << "  Anim: " << _anim._41 << ", " << _anim._42 << ", " << _anim._43 << " (Trans)\n";

		// Bind の分解
		DX::Matrix4x4 tempAnim = _anim;
		DX::Vector3 s, t;
		DX::Quaternion r;
		tempAnim.Decompose(s, r, t);

		_ofs << "  AnimScale: " << s.x << ", " << s.y << ", " << s.z << "\n\n";
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent Methods
//-----------------------------------------------------------------------------
AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive)
	: Component(_owner, _isActive)
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

	this->isPlaying = true;
	this->isLoop = true;

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
	this->trackToNodeIndex.clear();

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
	// SetSkeletonCache 内に追記
	if (_cache) {
		std::ofstream ofs("Import_Skeleton_Dump.txt");
		ofs << "=== Skeleton Import Check ===\n";
		ofs << "Node Count: " << _cache->nodes.size() << "\n";

		// 1. Rootの逆行列（これに巨大なスケールが入っていないか）
		ofs << "\n[Global Inverse Root Matrix]\n";
		DumpMatrix4x4_RowCol(ofs, "globalInverse", _cache->globalInverse);

		// 2. Hipsのボーンオフセット（これがバインドポーズと一致するか）
		if (!_cache->boneOffset.empty()) {
			ofs << "\n[Bone Offset (First Bone)]\n";
			DumpMatrix4x4_RowCol(ofs, "boneOffset[0]", _cache->boneOffset[0]);
		}

		// 3. 最初の数ノードの Raw Bind Matrix
		for (size_t i = 0; i < std::min<size_t>(5, _cache->nodes.size()); ++i) {
			ofs << "\nNode[" << i << "]: " << _cache->nodes[i].name << "\n";
			DumpMatrix4x4_RowCol(ofs, "  bindLocalMatrix", _cache->nodes[i].bindLocalMatrix);
		}
		ofs.close();
	}


	// --- ここからチェック用ログ ---
	std::ofstream ofs("SkeletonOrderCheck.txt");
	ofs << "Node Count: " << this->skeletonCache->nodes.size() << "\n";
	ofs << "Order Size: " << this->skeletonCache->order.size() << "\n\n";

	std::unordered_set<int> processedIndices;
	bool isOrderValid = true;

	for (size_t i = 0; i < this->skeletonCache->order.size(); ++i) {
		int idx = this->skeletonCache->order[i];
		const auto& node = this->skeletonCache->nodes[idx];

		// 親が自分より先に処理されているかチェック
		if (node.parentIndex >= 0) {
			if (processedIndices.find(node.parentIndex) == processedIndices.end()) {
				ofs << "[ERROR] Node '" << node.name << "' (idx:" << idx
					<< ") has parent (idx:" << node.parentIndex
					<< ") that appears LATER in order.\n";
				isOrderValid = false;
			}
		}
		processedIndices.insert(idx);
		ofs << "Order[" << i << "]: Index " << idx << " (Name: " << node.name << ")\n";
	}

	if (isOrderValid) {
		ofs << "\nRESULT: Skeleton order is VALID (Parents before children).\n";
	}
	else {
		ofs << "\nRESULT: Skeleton order is INVALID!\n";
	}

	this->isSkeletonCached = true;
	this->currentPose.Reset(*this->skeletonCache);

	this->bindGlobalMatrices.clear();
	BuildBindGlobalMatrices(this->bindGlobalMatrices, *this->skeletonCache);

	if (this->currentClip && !this->currentClip->IsBaked())
	{
		this->currentClip->BakeNodeIndices(*this->skeletonCache);
		DumpTrackBakeStatus(PoseDumpFilePath, this->currentClip, this->skeletonCache, "after BakeNodeIndices in SetSkeletonCache");
		DumpBakeValidationOnce(PoseDumpFilePath, this->currentClip, this->skeletonCache, "after BakeNodeIndices in SetSkeletonCache");
	}
	else
	{
		DumpTrackBakeStatus(PoseDumpFilePath, this->currentClip, this->skeletonCache, "SetSkeletonCache (no bake)");
		DumpBakeValidationOnce(PoseDumpFilePath, this->currentClip, this->skeletonCache, "SetSkeletonCache (no bake)");
	}

	this->clipEndTicks = SafeClipEndTicks(this->currentClip);
}

void AnimationComponent::SetAnimationClip(Graphics::Import::AnimationClip* _clip)
{
	this->currentClip = _clip;

	if (this->currentClip && this->isSkeletonCached && this->skeletonCache && !this->currentClip->IsBaked())
	{
		this->currentClip->BakeNodeIndices(*this->skeletonCache);
		DumpTrackBakeStatus(PoseDumpFilePath, this->currentClip, this->skeletonCache, "after BakeNodeIndices in SetAnimationClip");
		DumpBakeValidationOnce(PoseDumpFilePath, this->currentClip, this->skeletonCache, "after BakeNodeIndices in SetAnimationClip");
	}
	else
	{
		DumpTrackBakeStatus(PoseDumpFilePath, this->currentClip, this->skeletonCache, "SetAnimationClip (no bake)");
		DumpBakeValidationOnce(PoseDumpFilePath, this->currentClip, this->skeletonCache, "SetAnimationClip (no bake)");
	}

	this->clipEndTicks = SafeClipEndTicks(this->currentClip);

	// SetAnimationClip 内、または読み込み完了直後に一度だけ実行
	if (_clip) {
		std::ofstream ofs("Anim_Import_Detail_Check.txt");
		ofs << "=== Animation Clip Detail Check ===\n";
		ofs << "Clip Name: " << _clip->name << "\n";
		ofs << "Duration: " << _clip->durationTicks << " TicksPerSecond: " << _clip->ticksPerSecond << "\n";
		ofs << "Track Count: " << _clip->tracks.size() << "\n\n";

		for (const auto& track : _clip->tracks) {
			// Hips または 最初の数個のトラックをダンプ
			if (track.nodeName.find("Hips") != std::string::npos || track.nodeIndex < 5) {
				ofs << "Track: " << track.nodeName << " (NodeIndex: " << track.nodeIndex << ")\n";

				if (!track.positionKeys.empty()) {
					const auto& pk = track.positionKeys[0];
					ofs << "  [First PosKey] Time: " << pk.ticksTime
						<< " Val: (" << pk.value.x << ", " << pk.value.y << ", " << pk.value.z << ")\n";
				}
				else {
					ofs << "  [No PosKeys]\n";
				}

				if (!track.rotationKeys.empty()) {
					const auto& rk = track.rotationKeys[0];
					ofs << "  [First RotKey] Time: " << rk.ticksTime
						<< " Val: (" << rk.value.x << ", " << rk.value.y << ", " << rk.value.z << ", " << rk.value.w << ")\n";
				}

				if (!track.scaleKeys.empty()) {
					const auto& sk = track.scaleKeys[0];
					ofs << "  [First SclKey] Time: " << sk.ticksTime
						<< " Val: (" << sk.value.x << ", " << sk.value.y << ", " << sk.value.z << ")\n";
				}
				ofs << "\n";
			}
		}
		ofs.close();
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
	if (!this->isPlaying) { return; }
	if (!this->currentClip) { return; }
	if (!this->isSkeletonCached) { return; }
	if (!this->skeletonCache) { return; }
	if (this->skeletonCache->nodes.empty()) { return; }

	const double dt = static_cast<double>(_deltaTime) * static_cast<double>(this->playbackSpeed);
	const double tps = this->currentClip->ticksPerSecond;
	if (tps <= 0.0) { return; }

	const double endTicksRaw = (this->clipEndTicks > 0.0) ? this->clipEndTicks : this->currentClip->durationTicks;
	const double endTicks = (ForceEndTicks > 0.0) ? std::min(endTicksRaw, ForceEndTicks) : endTicksRaw;
	const double endSec = (endTicks > 0.0) ? (endTicks / tps) : 0.0;

	double nextTime = this->currentTime + dt;
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
			if (nextTime < 0.0)
			{
				nextTime = 0.0;
			}
		}
	}
	this->currentTime = nextTime;

	//-----------------------------------------------------------------------------
	// ポーズ更新
	//-----------------------------------------------------------------------------
	this->UpdatePoseFromClip(this->currentTime);

	//-----------------------------------------------------------------------------
	// ボーン行列更新
	//-----------------------------------------------------------------------------
	boneBuffer.boneCount = static_cast<uint32_t>(skeletonCache->boneIndexToNodeIndex.size());
	// ↑ ここで ShaderCommon::MaxBones (512) を超えない限り、 boneMatrices 配列はリサイズされない

	const size_t count = static_cast<size_t>(this->boneBuffer.boneCount);

	for (size_t i = 0; i < ShaderCommon::MaxBones; ++i)
	{
		this->boneBuffer.boneMatrices[i] = DX::Matrix4x4::Identity;
	}

	for (size_t i = 0; i < count; ++i)
	{
		const DX::Matrix4x4& m = this->currentPose.cpuBoneMatrices[i];

		// GPU へは最終だけ転置して送る（mul(v, M) 前提）
		this->boneBuffer.boneMatrices[i] = TransposeDxMatrix(m);

		static bool dumped = false;
		if (!dumped && count > 0)
		{
			const int boneIndex = 0;
			const DX::Matrix4x4 skinCpu = this->currentPose.cpuBoneMatrices[static_cast<size_t>(boneIndex)];
			const DX::Matrix4x4 uploaded = this->boneBuffer.boneMatrices[static_cast<size_t>(boneIndex)];
			DumpBoneMatrixCpuGpuPairOnce(PoseDumpFilePath, boneIndex, skinCpu, uploaded);
			dumped = true;
		}
	}

	//// 306行目付近：boneCB を GPU に送る直前
	//for (int b = 0; b < static_cast<int>(this->skeletonCache->boneOffset.size()); ++b)
	//{
	//	// HLSL側の mul(v, M) に合わせるために転置する
	//	this->boneBuffer.boneMatrices[b] = XMMatrixTranspose(this->boneBuffer.boneMatrices[b]);
	//}

	//-----------------------------------------------------------------------------
	// 定数バッファ更新
	//-----------------------------------------------------------------------------
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
	if (!currentClip || !skeletonCache) return;

	// 1. Ticks換算
	double ticks = _timeSeconds * currentClip->ticksPerSecond;
	if (isLoop) {
		ticks = std::fmod(ticks, currentClip->durationTicks);
	}
	else {
		ticks = std::min(ticks, currentClip->durationTicks);
	}

	const size_t nodeCount = skeletonCache->nodes.size();

	// 2. Local行列の決定 (全ノードを走査)
	for (size_t i = 0; i < nodeCount; ++i) {
		const auto& nodeInfo = skeletonCache->nodes[i];

		// このノードに対するアニメーション（Track）があるか探す
		const Graphics::Import::NodeTrack* track = nullptr;
		// 本来は map 等で引くのが理想ですが、現状の構成に合わせて検索
		for (const auto& t : currentClip->tracks) {
			if (t.nodeIndex == (int)i) {
				track = &t;
				break;
			}
		}

		if (track) {
			// アニメーションが存在するノード (Hipsや関節など)
			UpdateLocalMatrixFromKeys(i, ticks);
		}
		else {
			// アニメーションが存在しないノード ($AssimpFbx$ やその他中間ノード)
			// FBX本来のトランスフォーム（Bind姿勢）を代入
			currentPose.localMatrices[i] = nodeInfo.bindLocalMatrix;
		}
	}

	// 3. Global行列の更新 (全ノードを親子順に合成)
	// skeletonCache->order には Root から Leaf までの全インデックスが親子順で入っているため、
	// これを回すだけで全ノードの CombinedTransform が正しく計算されます。
	for (int nodeIndex : skeletonCache->order) {
		const int parentIndex = skeletonCache->nodes[nodeIndex].parentIndex;
		if (parentIndex < 0) {
			// 親がいない（RootNode）
			currentPose.globalMatrices[nodeIndex] = currentPose.localMatrices[nodeIndex];
		}
		else {
			// 行ベクトル形式: Global = Local(自分) * Global(親)
			currentPose.globalMatrices[nodeIndex] =
				currentPose.localMatrices[nodeIndex] * currentPose.globalMatrices[parentIndex];
		}
	}

	// 4. スキニング行列（Shaderへ送る boneMatrices / cpuBoneMatrices）の計算
	// ここは Mesh に紐づいているボーンのみを処理
	for (size_t boneIdx = 0; boneIdx < skeletonCache->boneIndexToNodeIndex.size(); ++boneIdx) {
		if (boneIdx >= ShaderCommon::MaxBones) break;

		int nodeIdx = skeletonCache->boneIndexToNodeIndex[boneIdx];
		const auto& offset = skeletonCache->boneOffset[boneIdx];

		// Skin = Offset * Global * GlobalInverse (Root基準に戻す)
		currentPose.cpuBoneMatrices[boneIdx] = offset * currentPose.globalMatrices[nodeIdx] * skeletonCache->globalInverse;
	}

	// デバッグ出力や GPU 転送は現状のままでOK
}

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
	return SlerpQuatSimple(leftKey.value, rightKey.value, t);
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

void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!_context) { return; }
	if (!this->boneCB) { return; }

	ID3D11Buffer* buf = this->boneCB->GetBuffer();
	if (!buf) { return; }

	_context->VSSetConstantBuffers(_slot, 1, &buf);
}

void AnimationComponent::UpdateLocalMatrixFromKeys(size_t _nodeIdx, double _ticks)
{
	auto& nodeInfo = this->skeletonCache->nodes[_nodeIdx];

	// 1. デフォルト値として Bind姿勢を分解して持っておく
	// (アニメーションにキーがない成分は、この値を維持することでボーンの長さを守る)
	XMMATRIX bindM = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&nodeInfo.bindLocalMatrix));
	XMVECTOR bS, bR, bT;
	XMMatrixDecompose(&bS, &bR, &bT, bindM);

	// 2. このノードに対応するアニメーションがあるか検索
	const Graphics::Import::NodeTrack* track = nullptr;
	for (const auto& t : currentClip->tracks) {
		if (t.nodeIndex == (int)_nodeIdx) {
			track = &t;
			break;
		}
	}

	// 3. トラックがない、または $AssimpFbx$ 中間ノードの場合は Bind姿勢をそのまま使う
	// ($AssimpFbx$系のノードには基本的にアニメーションキーは存在しません)
	if (!track) {
		currentPose.localMatrices[_nodeIdx] = nodeInfo.bindLocalMatrix;
		return;
	}

	// 4. アニメーションキーがある成分だけ上書き、ない成分は Bind姿勢を維持
	DX::Vector3 finalPos, finalScale;
	DX::Quaternion finalRot;

	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&finalPos), bT);
	XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&finalRot), bR);
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&finalScale), bS);

	if (track->hasPosition) {
		// キーがある場合のみ上書き（主にHipsのみ）
		// ※ここで変なスケール(0.189fなど)は掛けないでください
		finalPos = InterpolateTranslation(track, (float)_ticks, finalPos);
	}

	if (track->hasRotation) {
		// ほとんどの関節はここを通る
		finalRot = InterpolateRotation(track, (float)_ticks, finalRot);
	}

	if (track->hasScale) {
		finalScale = InterpolateScale(track, (float)_ticks, finalScale);
	}

	// 5. 行列再構成
	XMMATRIX m = XMMatrixScalingFromVector(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&finalScale))) *
		XMMatrixRotationQuaternion(XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&finalRot))) *
		XMMatrixTranslationFromVector(XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(&finalPos)));

	XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&currentPose.localMatrices[_nodeIdx]), m);
}