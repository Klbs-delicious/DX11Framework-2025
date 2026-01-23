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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <cmath>
#include <functional>

#include <DirectXMath.h>

using namespace DirectX;

//-----------------------------------------------------------------------------
// Internal helpers
//-----------------------------------------------------------------------------
namespace
{	
	/** @brief 値を範囲に収める
	 *  @param _v 値
	 *  @param _min 最小
	 *  @param _max 最大
	 *  @return クランプ後の値
	 */
	static double ClampDouble(double _v, double _min, double _max)
	{
		if (_v < _min) { return _min; }
		if (_v > _max) { return _max; }
		return _v;
	}

	/** @brief aiVector3D を線形補間
	 *  @param _a 開始
	 *  @param _b 終了
	 *  @param _alpha 0..1
	 *  @return 補間結果
	 */
	static aiVector3D LerpVec3(const aiVector3D& _a, const aiVector3D& _b, float _alpha)
	{
		return aiVector3D(
			_a.x + (_b.x - _a.x) * _alpha,
			_a.y + (_b.y - _a.y) * _alpha,
			_a.z + (_b.z - _a.z) * _alpha
		);
	}

	/** @brief キー配列から「区間の左側キー index」を探す（ticks）
	 *  @param _keys キー配列（time が単調増加想定）
	 *  @param _t 現在時刻（ticks）
	 *  @return 左側キー index（最後区間は size-2 を返す）
	 */
	template<typename TKey>
	static int FindLeftKeyIndexByTime(const std::vector<TKey>& _keys, double _t)
	{
		if (_keys.size() < 2)
		{
			return 0;
		}

		if (_t <= _keys.front().time)
		{
			return 0;
		}
		if (_t >= _keys.back().time)
		{
			return static_cast<int>(_keys.size()) - 2;
		}

		int low = 0;
		int high = static_cast<int>(_keys.size()) - 1;
		while (low <= high)
		{
			const int mid = (low + high) / 2;
			if (_keys[mid].time <= _t)
			{
				low = mid + 1;
			}
			else
			{
				high = mid - 1;
			}
		}

		const int idx = std::max(0, std::min(high, static_cast<int>(_keys.size()) - 2));
		return idx;
	}

	/** @brief 位置/スケール（Vec3）をキー配列からサンプリングする（ticks）
	 *  @param _keys キー配列
	 *  @param _tickTicks 現在時刻（ticks）
	 *  @param _fallback キーが無い場合に返す値
	 *  @return サンプリング結果
	 */
	static aiVector3D SampleVec3(
		const std::vector<Graphics::Import::AnimKeyVec3>& _keys,
		double _tickTicks,
		const aiVector3D& _fallback)
	{
		if (_keys.empty())
		{
			return _fallback;
		}
		if (_keys.size() == 1)
		{
			return _keys[0].value;
		}

		const int left = FindLeftKeyIndexByTime(_keys, _tickTicks);
		const int right = left + 1;

		const double t0 = _keys[left].time;
		const double t1 = _keys[right].time;

		const double denom = (t1 - t0);
		float alpha = 0.0f;
		if (denom > 0.0)
		{
			alpha = static_cast<float>((_tickTicks - t0) / denom);
			alpha = static_cast<float>(ClampDouble(alpha, 0.0, 1.0));
		}

		return LerpVec3(_keys[left].value, _keys[right].value, alpha);
	}

	/** @brief 回転（Quat）をキー配列からサンプリングする（ticks）
	 *  @param _keys キー配列
	 *  @param _tickTicks 現在時刻（ticks）
	 *  @param _fallback キーが無い場合に返す値
	 *  @return サンプリング結果
	 */
	static aiQuaternion SampleQuat(
		const std::vector<Graphics::Import::AnimKeyQuat>& _keys,
		double _tickTicks,
		const aiQuaternion& _fallback)
	{
		if (_keys.empty())
		{
			return _fallback;
		}
		if (_keys.size() == 1)
		{
			return _keys[0].value;
		}

		const int left = FindLeftKeyIndexByTime(_keys, _tickTicks);
		const int right = left + 1;

		const double t0 = _keys[left].time;
		const double t1 = _keys[right].time;

		const double denom = (t1 - t0);
		float alpha = 0.0f;
		if (denom > 0.0)
		{
			alpha = static_cast<float>((_tickTicks - t0) / denom);
			alpha = static_cast<float>(ClampDouble(alpha, 0.0, 1.0));
		}

		aiQuaternion out{};
		aiQuaternion::Interpolate(out, _keys[left].value, _keys[right].value, alpha);
		out.Normalize();
		return out;
	}

	/** @brief アニメーション時刻（ticks）を 0..durationTicks に収める（ループ）
	 *  @param _tickTicks		現在時刻（ticks）
	 *  @param _durationTicks	クリップ長（ticks）
	 *  @return 正規化後の ticks
	 */
	static double WrapTick(double _tickTicks, double _durationTicks)
	{
		if (_durationTicks <= 0.0)
		{
			// 長さゼロなら常に 0
			return 0.0;
		}

		double t = std::fmod(_tickTicks, _durationTicks);
		if (t < 0.0)
		{
			// 負の場合は一周分足す
			t += _durationTicks;
		}
		return t;
	}

	/** @brief キー配列から「区間の左側キー index」を探す
	 *  @param _times 単調増加のキー時刻列（ticks）
	 *  @param _t 現在時刻（ticks）
	 *  @return 左側キー index（最後区間は size-2 を返す）
	 */
	static int FindLeftKeyIndex(const std::vector<double>& _times, double _t)
	{
		if (_times.size() < 2)
		{
			return 0;
		}

		if (_t <= _times.front())
		{
			return 0;
		}
		if (_t >= _times.back())
		{
			return static_cast<int>(_times.size()) - 2;
		}

		auto it = std::upper_bound(_times.begin(), _times.end(), _t);
		const int idx = static_cast<int>(std::distance(_times.begin(), it)) - 1;
		return std::max(0, std::min(idx, static_cast<int>(_times.size()) - 2));
	}

	/** @brief AnimKeyVec3 の時刻配列を作る
	 *  @param _keys キー配列
	 *  @return 時刻配列
	 */
	static std::vector<double> BuildTimesVec3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		std::vector<double> t;
		t.reserve(_keys.size());
		for (const auto& k : _keys)
		{
			t.push_back(k.time);
		}
		return t;
	}

	/** @brief AnimKeyQuat の時刻配列を作る
	 *  @param _keys キー配列
	 *  @return 時刻配列
	 */
	static std::vector<double> BuildTimesQuat(const std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		std::vector<double> t;
		t.reserve(_keys.size());
		for (const auto& k : _keys)
		{
			t.push_back(k.time);
		}
		return t;
	}

	/** @brief Assimp の TRS から DirectX のローカル行列を作る
	 *  @details 行ベクトル（mul(v, M)）で運用する想定
	 *  @param _t 位置
	 *  @param _r 回転
	 *  @param _s スケール
	 *  @return ローカル行列
	 */
	static DX::Matrix4x4 BuildLocalMatrixFromTrs(const aiVector3D& _t, const aiQuaternion& _r, const aiVector3D& _s)
	{
		const DX::Vector3 scale(_s.x, _s.y, _s.z);
		const DX::Vector3 trans(_t.x, _t.y, _t.z);
		const DX::Quaternion rot(_r.x, _r.y, _r.z, _r.w);

		DX::Matrix4x4 mS = DX::Matrix4x4::CreateScale(scale);
		DX::Matrix4x4 mR = DX::Matrix4x4::CreateFromQuaternion(rot);
		DX::Matrix4x4 mT = DX::Matrix4x4::CreateTranslation(trans);

		return mS * mR * mT;
	}

	/** @brief Assimpの行列をDirectXの行列に変換する（転置なし）
	 *  @param _aiMtx Assimp行列
	 *  @return DirectX行列
	 */
	DX::Matrix4x4 aiMtxToDxMtx(const aiMatrix4x4& _aiMtx)
	{
		return DX::Matrix4x4(
			_aiMtx.a1, _aiMtx.a2, _aiMtx.a3, _aiMtx.a4,
			_aiMtx.b1, _aiMtx.b2, _aiMtx.b3, _aiMtx.b4,
			_aiMtx.c1, _aiMtx.c2, _aiMtx.c3, _aiMtx.c4,
			_aiMtx.d1, _aiMtx.d2, _aiMtx.d3, _aiMtx.d4
		);
	}

	///** @brief ノード名からノードindexを引ける辞書を作る
	// *  @param _nodes ノード配列
	// *  @return name -> index の辞書
	// */
	//static std::unordered_map<std::string, int> BuildNodeIndexMap(const std::vector<Graphics::Import::SkeletonNodeCache>& _nodes)
	//{
	//	std::unordered_map<std::string, int> map;
	//	map.reserve(_nodes.size());

	//	for (int i = 0; i < static_cast<int>(_nodes.size()); i++)
	//	{
	//		map.emplace(_nodes[i].name, i);
	//	}

	//	return map;
	//}

	/** @brief TreeNode を親→子順に走査して nodes を構築する
	 *  @param _node 現在のノード
	 *  @param _parentIndex 親ノードの index（親なしは -1）
	 *  @param _boneDict ボーン辞書（name -> Bone）
	 *  @param _outNodes 出力先 nodes
	 */
	static void BuildNodesDfsPreOrder(
		const Utils::TreeNode<Graphics::Import::BoneNode>* _node,
		int _parentIndex,
		const std::unordered_map<std::string, Graphics::Import::Bone>& _boneDict,
		std::vector<Graphics::Import::SkeletonNodeCache>& _outNodes)
	{
		if (!_node){ return; }
		Graphics::Import::SkeletonNodeCache cache{};
		cache.name = _node->nodedata.name;
		cache.parentIndex = _parentIndex;

		// bindLocalMatrix は「ノードツリーのローカル姿勢」を必ず入れる
		// アニメが無いノードも親子合成に必要なので、ここは常に確定させる
		cache.bindLocalMatrix = aiMtxToDxMtx(_node->nodedata.localBind);

		// アニメは後で差し替えるので最初は nullptr
		cache.nodeTrackRef = nullptr;

		// boneIndex は「ノード名が boneDictionary に存在する場合のみ」入れる
		auto it = _boneDict.find(cache.name);
		if (it != _boneDict.end())
		{
			cache.boneIndex = it->second.index;
		}
		else
		{
			// ボーンでない場合
			cache.boneIndex = -1;
		}
		const int myIndex = static_cast<int>(_outNodes.size());
		_outNodes.push_back(cache);

		// 子へ再帰（親→子順）
		for (const auto& child : _node->children)
		{
			BuildNodesDfsPreOrder(child.get(), myIndex, _boneDict, _outNodes);
		}
	}

	/** @brief 親が必ず先になる計算順 order を作る（nodes を前提）
	 *  @details DFSのpre-orderで nodes を作るなら、order は 0..N-1 で成立する
	 *  @param _nodeCount ノード数
	 *  @param _outOrder 出力先
	 */
	static void BuildOrderDefault(int _nodeCount, std::vector<int>& _outOrder)
	{
		_outOrder.clear();
		_outOrder.reserve(_nodeCount);

		for (int i = 0; i < _nodeCount; i++)
		{
			_outOrder.push_back(i);
		}
	}

	/** @brief boneOffset（boneIndex -> OffsetMatrix）を作る
	 *  @param _boneDict ボーン辞書
	 *  @param _outBoneOffset 出力先
	 */
	static void BuildBoneOffsetArray(
		const std::unordered_map<std::string, Graphics::Import::Bone>& _boneDict,
		std::vector<DX::Matrix4x4>& _outBoneOffset)
	{
		int maxIndex = -1;
		for (const auto& p : _boneDict)
		{
			// ボーンが配列の何番目かを格納
			maxIndex = std::max(maxIndex, p.second.index);
		}

		// ボーンのオフセット行列を格納
		_outBoneOffset.clear();
		_outBoneOffset.resize(static_cast<size_t>(maxIndex + 1), DX::Matrix4x4::Identity);
		for (const auto& p : _boneDict)
		{
			const int idx = p.second.index;
			if (idx < 0) { continue; }

			// この段階では転置せずコピーのみ
			_outBoneOffset[static_cast<size_t>(idx)] = aiMtxToDxMtx(p.second.offsetMatrix);
		}
	}

	// Add logging helpers for debugging bone buffer
	static void PrintMatrixDebug(const DX::Matrix4x4& m)
	{
		std::cout << "[Matrix] ";
		std::cout << m._11 << "," << m._12 << "," << m._13 << "," << m._14 << " | ";
		std::cout << m._21 << "," << m._22 << "," << m._23 << "," << m._24 << " | ";
		std::cout << m._31 << "," << m._32 << "," << m._33 << "," << m._34 << " | ";
		std::cout << m._41 << "," << m._42 << "," << m._43 << "," << m._44 << std::endl;
	}

	// Print a single matrix in a compact form including translation
	static void PrintMatrixCompact(const DX::Matrix4x4& m)
	{
		// print rows and translation component separately for easier reading
		std::cout << "  [r0] " << m._11 << ", " << m._12 << ", " << m._13 << ", " << m._14 << "\n";
		std::cout << "  [r1] " << m._21 << ", " << m._22 << ", " << m._23 << ", " << m._24 << "\n";
		std::cout << "  [r2] " << m._31 << ", " << m._32 << ", " << m._33 << ", " << m._34 << "\n";
		std::cout << "  [tr] " << m._41 << ", " << m._42 << ", " << m._43 << "\n";
	}

	// Unified bone buffer logger: compatible with previous call sites that pass (buf, true)
	static void LogBoneBufferContent(const class AnimationComponent::BoneBuffer& buf, bool force = false, size_t maxBonesToShow = SIZE_MAX)
	{
		// Print basic info
		const uint32_t count = buf.boneCount;
		std::cout << "[AnimationComponent] BoneBuffer.boneCount = " << count << std::endl;

		if (count == 0)
		{
			return;
		}

		// Determine how many bones to actually print
		size_t toShow = count;
		if (maxBonesToShow != SIZE_MAX)
		{
			toShow = std::min<size_t>(toShow, maxBonesToShow);
		}

		// Safety clamp to array size
		toShow = std::min<size_t>(toShow, buf.boneMatrices.size());

		for (size_t i = 0; i < toShow; ++i)
		{
			std::cout << "[AnimationComponent] boneMatrices[" << i << "] =" << std::endl;
			PrintMatrixCompact(buf.boneMatrices[i]);
		}

		if (toShow < count)
		{
			std::cout << "[AnimationComponent] (skipped printing " << (count - toShow) << " bones)" << std::endl;
		}
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent
//-----------------------------------------------------------------------------
AnimationComponent::AnimationComponent(GameObject* _owner, bool _isActive) :
	Component(_owner, _isActive),
	currentClip(nullptr),
	meshComponent(nullptr),
	modelData(nullptr),
	isPlaying(false),
	currentTime(0.0),
	playbackSpeed(1.0f),
	isSkeletonCached(false),
	boneBuffer{},
	boneCB(nullptr),
	enableBoneDebugLog(true),
	debugLogFrameCounter(0),
	debugLogBurstSize(4),
	debugLogPeriod(120) // temporarily set to 1 for immediate visible bursts
{}

void AnimationComponent::Initialize()
{
	//----------------------------------------------
	// コンポーネントの取得
	//----------------------------------------------
	this->meshComponent = this->Owner()->GetComponent<MeshComponent>();

	//----------------------------------------------
	// 定数バッファの作成
	//----------------------------------------------
	auto& d3d = SystemLocator::Get<D3D11System>();
	this->boneCB = std::make_unique<DynamicConstantBuffer<BoneBuffer>>();
	this->boneCB->Create(d3d.GetDevice());
	this->boneBuffer.boneCount = 0;

	// Debug: print initial bone buffer before first upload
	LogBoneBufferContent(this->boneBuffer);

	this->boneCB->Update(d3d.GetContext(), this->boneBuffer);

	this->isSkeletonCached = false;

	if (this->modelData)
	{
		// モデルデータが既にセットされている場合はスケルトンキャッシュを構築する
		this->RebuildSkeletonCache();
	}

	////----------------------------------------------
	//// 時間関連の初期化
	////----------------------------------------------
	this->isPlaying = false;
	this->currentTime = 0.0;
	this->playbackSpeed = 100.0f;
}

void AnimationComponent::Dispose()
{
	this->currentClip = nullptr;
	this->meshComponent = nullptr;
	this->modelData = nullptr;

	this->isPlaying = false;
	this->currentTime = 0.0;

	this->isSkeletonCached = false;
	this->skeletonCache.nodes.clear();
	this->skeletonCache.order.clear();
	this->skeletonCache.boneOffset.clear();

	this->currentPose.localMatrices.clear();
	this->currentPose.globalMatrices.clear();
	this->currentPose.skinMatrices.clear();

	if (this->boneCB)
	{
		this->boneCB.reset();
	}
}

void AnimationComponent::BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const
{
	if (!this->boneCB || !_context) { return; }
	this->boneCB->BindVS(_context, _slot);
}

void AnimationComponent::SetAnimationClip(Graphics::Import::AnimationClip* _clip)
{
	this->currentClip = _clip;
	this->currentTime = 0.0;

	if (!this->isSkeletonCached)
	{
		return;
	}

	for (auto& n : this->skeletonCache.nodes)
	{
		n.nodeTrackRef = nullptr;
	}

	if (!this->currentClip)
	{
		return;
	}

	for (auto& n : this->skeletonCache.nodes)
	{
		auto it = this->currentClip->tracks.find(n.name);
		if (it != this->currentClip->tracks.end())
		{
			n.nodeTrackRef = &it->second;
		}
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent::SampleLocalFromTrack
//-----------------------------------------------------------------------------
DX::Matrix4x4 AnimationComponent::SampleLocalFromTrack(
	const Graphics::Import::NodeTrack& _track,
	double _tickTicks,
	double _ticksPerSecond,
	double _durationTicks)
{
	// 未使用パラメータ抑制
	(void)_ticksPerSecond;
	(void)_durationTicks;

	// キーが無い場合の既定値
	const aiVector3D fallbackT(0.0f, 0.0f, 0.0f);
	const aiQuaternion fallbackR(1.0f, 0.0f, 0.0f, 0.0f); // Assimp は (w,x,y,z)
	const aiVector3D fallbackS(1.0f, 1.0f, 1.0f);

	// ticks でサンプリングする（time は Assimp 由来の ticks）
	const aiVector3D t = SampleVec3(_track.positionKeys, _tickTicks, fallbackT);
	const aiQuaternion r = SampleQuat(_track.rotationKeys, _tickTicks, fallbackR);
	const aiVector3D s = SampleVec3(_track.scaleKeys, _tickTicks, fallbackS);

	return BuildLocalMatrixFromTrs(t, r, s);
}


const std::string& AnimationComponent::GetCurrentAnimationName() const
{
	static const std::string empty = "";

	if (!this->currentClip)
	{
		return empty;
	}

	return this->currentClip->name;
}

void AnimationComponent::Play()
{
	this->isPlaying = true;
}

void AnimationComponent::Stop()
{
	this->isPlaying = false;
}

void AnimationComponent::SetModelData(Graphics::Import::ModelData* _modelData)
{
	this->modelData = _modelData;
	this->isSkeletonCached = false;

	if (this->boneCB && this->modelData)
	{
		// モデルデータが変わったのでスケルトンキャッシュを再構築する
		this->RebuildSkeletonCache();
	}
}

//-----------------------------------------------------------------------------
// AnimationComponent::RebuildSkeletonCache
//-----------------------------------------------------------------------------
void AnimationComponent::RebuildSkeletonCache()
{
	if(!this->modelData)
	{
		std::cerr << "[AnimationComponent] モデルデータが設定されていないため、スケルトンキャッシュを構築できません。" << std::endl;
		return;
	}
	if (this->modelData->boneDictionary.empty())
	{
		std::cerr << "[AnimationComponent] モデルデータにボーン情報が存在しないため、スケルトンキャッシュを構築できません。" << std::endl;
		return;
	}
	//----------------------------------------------
	// スケルトンキャッシュの構築
	//----------------------------------------------
	this->isSkeletonCached = false;
	BuildSkeletonCache(
		this->modelData->nodeTree,
		this->modelData->boneDictionary,
		this->skeletonCache
	);
	this->isSkeletonCached = true;

	//----------------------------------------------
	// 現在のポーズの初期化（バインド姿勢）
	//----------------------------------------------
	this->currentPose.Reset(this->skeletonCache);

	//----------------------------------------------
	// ボーン行列用定数バッファの更新
	//----------------------------------------------
	this->boneBuffer.boneMatrices = this->currentPose.gpuBoneMatrices;

	// ボーン数は最大値までクリップ
	const size_t boneCount = std::min<size_t>(this->skeletonCache.boneOffset.size(), ShaderCommon::MaxBones);
	this->boneBuffer.boneCount = static_cast<uint32_t>(boneCount);

	// Debug: print bone buffer before uploading to GPU
	if (this->enableBoneDebugLog)
	{
		// Force logging during rebuild
		::LogBoneBufferContent(this->boneBuffer, true);
	}

	this->boneCB->Update(SystemLocator::Get<D3D11System>().GetContext(), this->boneBuffer);
}

void AnimationComponent::FixedUpdate(float _deltaTime)
{
	// Debug: indicate FixedUpdate is executing
	std::cout << "[AnimationComponent] FixedUpdate called, frameCounter=" << this->debugLogFrameCounter << std::endl;

	if (!this->modelData) { return; }
	if (!this->currentClip) { return; }
	if (!this->meshComponent) { return; }
	if (!this->isPlaying) { return; }
	if (!this->isSkeletonCached) { return; }
	
	//----------------------------------------------
	// アニメーションのサンプリング
	//----------------------------------------------
	const double tps = (this->currentClip->ticksPerSecond > 0.0f) ? this->currentClip->ticksPerSecond : 25.0f;
	const double durationTicks = this->currentClip->durationTicks;

	// 現在時刻を進める
	this->currentTime += static_cast<double>(_deltaTime) * static_cast<double>(this->playbackSpeed);
	if (this->currentTime < 0.0)
	{
		// ループさせる
		this->currentTime = 0.0;
	}

	// 現在の tick を計算してラップさせる
	double tick = this->currentTime * tps;
	tick = WrapTick(tick, durationTicks);

	//----------------------------------------------
	// ポーズの更新
	//----------------------------------------------
	const size_t nodeCount = this->skeletonCache.nodes.size();
	if (nodeCount == 0){ return; }

	if (this->currentPose.localMatrices.size() != nodeCount)
	{
		// ポーズの配列サイズが合っていない場合はリセット
		this->currentPose.Reset(this->skeletonCache);
	}

	//--------------------------------------------------------------
	// localMatrices を更新（トラックがあるノードはサンプリング、無いノードは bindLocal）
	//--------------------------------------------------------------
	for (size_t i = 0; i < nodeCount; i++)
	{
		const Graphics::Import::SkeletonNodeCache& node = this->skeletonCache.nodes[i];

		if (node.nodeTrackRef)
		{
			// トラックがある場合はサンプリングする
			this->currentPose.localMatrices[i] = SampleLocalFromTrack(*node.nodeTrackRef, tick, tps, durationTicks);
		}
		else
		{
			// トラックが無い場合はバインド姿勢を使う
			this->currentPose.localMatrices[i] = node.bindLocalMatrix;
		}
	}

	//-----------------------------------------------------------------------------
	// globalMatrices を更新（親が必ず先に来る order を使用）
	//-----------------------------------------------------------------------------
	for (size_t oi = 0; oi < this->skeletonCache.order.size(); oi++)
	{
		const int nodeIndex = this->skeletonCache.order[oi];
		if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodeCount)
		{
			// 不正な index はスキップ
			continue;
		}

		// 親 index を取得する
		const int parentIndex = this->skeletonCache.nodes[nodeIndex].parentIndex;
		if (parentIndex < 0)
		{
			// 親なし
			this->currentPose.globalMatrices[nodeIndex] = this->currentPose.localMatrices[nodeIndex];
		}
		else
		{
			// 親あり（子 * 親）
			this->currentPose.globalMatrices[nodeIndex] = this->currentPose.localMatrices[nodeIndex] * this->currentPose.globalMatrices[parentIndex];
		}
	}

	//-----------------------------------------------------------------------------
	// gpuBoneMatrices を更新（ボーンだけ）
	//-----------------------------------------------------------------------------
	for (size_t i = 0; i < ShaderCommon::MaxBones; i++)
	{
		this->currentPose.gpuBoneMatrices[i] = DX::Matrix4x4::Identity;
	}

	for (size_t i = 0; i < nodeCount; i++)
	{
		const int boneIndex = this->skeletonCache.nodes[i].boneIndex;
		if (boneIndex < 0)
		{
			// ボーンが存在しない
			continue;
		}

		if (static_cast<size_t>(boneIndex) >= this->skeletonCache.boneOffset.size())
		{
			// boneOffset が存在しないボーンはスキップ
			continue;
		}

		// スキニング行列を計算（ボーンのローカル行列 * バインドポーズ行列）
		const DX::Matrix4x4 skin =
			this->skeletonCache.boneOffset[boneIndex] *
			this->currentPose.globalMatrices[i] *
			this->skeletonCache.globalInverse;

		this->currentPose.skinMatrices[i] = skin;

		if (boneIndex < static_cast<int>(ShaderCommon::MaxBones))
		{
			// GPU用行列は転置して格納
			this->currentPose.gpuBoneMatrices[static_cast<size_t>(boneIndex)] = skin.Transpose();
		}
	}

	//-----------------------------------------------------------------------------
	// 定数バッファ更新
	//-----------------------------------------------------------------------------
	this->boneBuffer.boneMatrices = this->currentPose.gpuBoneMatrices;

	const size_t boneCount = std::min<size_t>(this->skeletonCache.boneOffset.size(), ShaderCommon::MaxBones);
	this->boneBuffer.boneCount = static_cast<uint32_t>(boneCount);

	//-------------------------------------------------------------
	// Debug: print bone buffer before uploading to GPU -- controlled burst logging
	//-------------------------------------------------------------
	if (this->enableBoneDebugLog)
	{
		// Increment frame counter each FixedUpdate
		this->debugLogFrameCounter++;

		// Determine position within period
		int frameInPeriod = this->debugLogFrameCounter % this->debugLogPeriod;

		if (frameInPeriod < this->debugLogBurstSize)
		{
			// Within burst -> print
			::LogBoneBufferContent(this->boneBuffer, true);
		}
	}

	this->boneCB->Update(SystemLocator::Get<D3D11System>().GetContext(), this->boneBuffer);
}

void AnimationComponent::BuildSkeletonCache(
	const Utils::TreeNode<Graphics::Import::BoneNode>& _nodeTree,
	const std::unordered_map<std::string, Graphics::Import::Bone>& _boneDict,
	Graphics::Import::SkeletonCache& _outCache)
{
	_outCache.nodes.clear();
	_outCache.order.clear();
	_outCache.boneOffset.clear();

	// nodes を作る（親→子順）
	BuildNodesDfsPreOrder(&_nodeTree, -1, _boneDict, _outCache.nodes);

	// order を作る（0..N-1）
	BuildOrderDefault(static_cast<int>(_outCache.nodes.size()), _outCache.order);

	// boneOffset（boneIndex -> OffsetMatrix）を作る
	BuildBoneOffsetArray(_boneDict, _outCache.boneOffset);

	// globalInverse を計算する
	if (!_outCache.nodes.empty())
	{
		const size_t nodeCount = _outCache.nodes.size();
		std::vector<DX::Matrix4x4> bindGlobal;
		bindGlobal.resize(nodeCount, DX::Matrix4x4::Identity);

		for (size_t oi = 0; oi < _outCache.order.size(); oi++)
		{
			const int nodeIndex = _outCache.order[oi];
			if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= nodeCount)
			{
				continue;
			}

			const int parentIndex = _outCache.nodes[nodeIndex].parentIndex;
			if (parentIndex < 0)
			{
				bindGlobal[nodeIndex] = _outCache.nodes[nodeIndex].bindLocalMatrix;
			}
			else
			{
				// 親あり（子 * 親）
				bindGlobal[nodeIndex] = _outCache.nodes[nodeIndex].bindLocalMatrix * bindGlobal[parentIndex];
			}
		}

		// parentIndex < 0 のノードが複数あるケースは基本想定しない（Assimp の root）
		// まず 0 番をルートとして扱う（DFS pre-order なので通常は root）
		DX::Matrix4x4 root = bindGlobal[0];

		// 逆行列を計算（失敗したら Identity のまま）
		_outCache.globalInverse = root.Invert();
	}
}