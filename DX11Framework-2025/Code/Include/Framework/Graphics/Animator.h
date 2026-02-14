/** @file	Animator.h
 *  @brief	アニメーション状態管理・再生（テンプレート）
 *
 *  テンプレートクラスのため、実装はヘッダ内に定義
 */
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/IAnimator.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include <algorithm>
#include <cmath>
#include <vector>

//-----------------------------------------------------------------------------
// Debug / Temporary settings
//-----------------------------------------------------------------------------
namespace Graphics::Animation::Detail
{
	inline constexpr double ForceEndTicksEps = 1.0e-6;	///< 終端判定用
}

//-----------------------------------------------------------------------------
// Local Helpers
//-----------------------------------------------------------------------------
namespace Graphics::Animation::Detail
{
	/** @brief トラックの終端 ticks を取得する
	 *  @param _keys トラックのキー
	 *  @return 終端 ticks
	 */
	inline double GetTrackEndTickVector3(const std::vector<Graphics::Import::AnimKeyVec3>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	/** @brief トラックの終端 ticks を取得する
	 *  @param _keys トラックのキー
	 *  @return 終端 ticks
	 */
	inline double GetTrackEndTickQuaternion(const std::vector<Graphics::Import::AnimKeyQuat>& _keys)
	{
		if (_keys.empty()) { return 0.0; }
		return _keys.back().ticksTime;
	}

	/** @brief クリップの終端 ticks をトラックから算出する
	 *  @param _clip 対象クリップ
	 *  @return 終端 ticks
	 */
	inline double ComputeClipEndTicksFromTracks(const Graphics::Import::AnimationClip& _clip)
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
	inline double SafeClipEndTicks(const Graphics::Import::AnimationClip* _clip)
	{
		if (!_clip) { return 0.0; }

		const double trackEnd = ComputeClipEndTicksFromTracks(*_clip);
		if (trackEnd > ForceEndTicksEps) { return trackEnd; }

		if (_clip->durationTicks > 0.0) { return _clip->durationTicks; }

		return 0.0;
	}

	//-----------------------------------------------------------------------------
	// キー探索の高速化：ノードごとの「前回キー位置」キャッシュ
	//-----------------------------------------------------------------------------
	struct TrackKeyCursor
	{
		size_t posLeftIndex = 0;	///< 直前に使った position の左キー index
		size_t rotLeftIndex = 0;	///< 直前に使った rotation の左キー index
		size_t sclLeftIndex = 0;	///< 直前に使った scale の左キー index
	};

	inline void ResetCursor(TrackKeyCursor& _c)
	{
		_c.posLeftIndex = 0;
		_c.rotLeftIndex = 0;
		_c.sclLeftIndex = 0;
	}
}

/** @class Animator
 *  @brief アニメーションクリップを管理・再生するクラス（再生・クロスフェードのみ）
 */
template<typename StateId>
class Animator : public IAnimator
{
public:
	Animator() = default;
	~Animator() = default;

	/** @brief 初期化（参照を保持するだけ）
	 *  @param _skeletonCache スケルトンキャッシュ
	 *  @param _stateTable 状態定義テーブル
	 *  @param _initState 初期状態
	 */
	void Initialize(
		const Graphics::Import::SkeletonCache* _skeletonCache,
		const Graphics::Animation::StateTable<StateId>* _stateTable,
		StateId _initState);

	/** @brief 次の状態への遷移をリクエストする
	 *  @param _next 次の状態ID
	 *  @param _overrideFadeSec フェード秒数のオーバーライド（デフォルト -1 で無効）
	 */
	void RequestState(StateId _next, float _overrideFadeSec = -1.0f);

	/** @brief アニメーションを再生する
	 *  @details - 停止中からの再生時に使用（ほぼデバッグ目的）
	 */
	void Play() override;

	/// @brief アニメーションを停止する
	void Stop() override;

	/** @brief アニメーションを最初から再生する
	 *  @details - 停止中からの再生時に使用（ほぼデバッグ目的）
	 */
	void Restart() override;

	/** @brief アニメーションを更新する
	 *  @param _deltaTime デルタ時間（秒）
	 */
	void Update(float _deltaTime) override;

	/** @brief 指定ティック位置のトラックから位置を補間取得する
	 *  @param _track 対象トラック
	 *  @param _ticks 補間位置（ティック）
	 *  @param _fallback キーが存在しない場合のフォールバック値
	 *  @return 補間後の位置
	 */
	DX::Vector3 InterpolateTranslation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback);

	/** @brief 指定ティック位置のトラックから回転を補間取得する
	 *  @param _track 対象トラック
	 *  @param _ticks 補間位置（ティック）
	 *  @param _fallback キーが存在しない場合のフォールバック値
	 *  @return 補間後の回転
	 */
	DX::Quaternion InterpolateRotation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Quaternion& _fallback);

	/** @brief 指定ティック位置のトラックからスケールを補間取得する
	 *  @param _track 対象トラック
	 *  @param _ticks 補間位置（ティック）
	 *  @param _fallback キーが存在しない場合のフォールバック値
	 *  @return 補間後のスケール
	 */
	DX::Vector3 InterpolateScale(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback);

	/** @brief 現在のローカルポーズを取得する
	 *  @return ローカルポーズ参照
	 */
	const Graphics::Animation::LocalPose& GetLocalPose() const override { return this->localPose; }

	/** @brief 現在の正規化時間を取得する（0.0～1.0）
	 *  @return 正規化時間
	 */
	float GetNormalizedTime() const override;

	/** @brief アニメーションが終了しているかを取得する
	 *  @return 終了していれば true
	 */
	bool IsFinished() const override { return this->isFinished; }

	/** @brief 現在のアニメーションクリップを取得する
	 *  @return アニメーションクリップ（無ければ nullptr）
	 */
	Graphics::Import::AnimationClip* GetCurrentClip() const override;

private:
	/// @brief バインドローカル姿勢を基準として適用する
	void ApplyBindLocalAsBase();

	/// @brief キー位置キャッシュをリセットする
	void ResetTrackCursors();

	/** @brief 任意のクリップを評価してローカルポーズを作る
	 *  @param _clip クリップ
	 *  @param _loop ループ有無
	 *  @param _timeSeconds 再生時間（秒）
	 *  @param _outPose 出力ローカルポーズ
	 *  @param _outNormalizedTime 出力正規化時間（0..1）
	 *  @param _outFinished 出力終了フラグ（非ループ時）
	 */
	void EvaluateClipLocalPose(
		const Graphics::Import::AnimationClip* _clip,
		bool _loop,
		double _timeSeconds,
		Graphics::Animation::LocalPose& _outPose,
		float& _outNormalizedTime,
		bool& _outFinished);

	/** @brief ローカルポーズ同士をTRSで補間して出力する
	 *  @param _from 遷移元
	 *  @param _to 遷移先
	 *  @param _w 重み（0..1）
	 *  @param _out 出力
	 */
	void BlendLocalPoseTRS(
		const Graphics::Animation::LocalPose& _from,
		const Graphics::Animation::LocalPose& _to,
		float _w,
		Graphics::Animation::LocalPose& _out);

	/** @brief 指定ノードのローカル行列をキーから更新して、出力先に書き込む
	 *  @param _nodeIdx ノードインデックス
	 *  @param _ticks 補間位置（ティック）
	 *  @param _track 対象トラック
	 *  @param _outPose 出力先
	 */
	void UpdateLocalMatrixFromKeysToPose(
		size_t _nodeIdx,
		double _ticks,
		const Graphics::Import::NodeTrack& _track,
		Graphics::Animation::LocalPose& _outPose);

	/** @brief キー位置キャッシュ付きで Vec3 を補間する
	 *  @param _keys キー配列
	 *  @param _ticks 補間位置（ティック）
	 *  @param _fallback フォールバック
	 *  @param _inOutLeftIndex 入出力：左キー index キャッシュ
	 *  @return 補間後の値
	 */
	DX::Vector3 InterpolateVec3Cached(
		const std::vector<Graphics::Import::AnimKeyVec3>& _keys,
		double _ticks,
		const DX::Vector3& _fallback,
		size_t& _inOutLeftIndex);

	/** @brief キー位置キャッシュ付きで Quaternion を補間する
	 *  @param _keys キー配列
	 *  @param _ticks 補間位置（ティック）
	 *  @param _fallback フォールバック
	 *  @param _inOutLeftIndex 入出力：左キー index キャッシュ
	 *  @return 補間後の値
	 */
	DX::Quaternion InterpolateQuatCached(
		const std::vector<Graphics::Import::AnimKeyQuat>& _keys,
		double _ticks,
		const DX::Quaternion& _fallback,
		size_t& _inOutLeftIndex);

private:
	const Graphics::Import::SkeletonCache* skeletonCache = nullptr;			///< 参照：スケルトンキャッシュ
	const Graphics::Animation::StateTable<StateId>* stateTable = nullptr;	///< 参照：状態定義テーブル

	StateId currentState{};													///< 現在の状態ID
	Graphics::Animation::LocalPose localPose{};								///< 現在のローカルポーズ

	float currentTimeSec = 0.0f;											///< 現在の再生時間（秒）
	float normalizedTime = 0.0f;											///< 正規化時間（0～1）
	bool isFinished = false;												///< 非ループ時の終了フラグ
	bool isPaused = false;													///< 停止中フラグ

	Graphics::Animation::CrossFadeData<StateId> crossFadeData{};			///< クロスフェード中の状態情報

	//-----------------------------------------------------------------------------
	// キー探索の高速化用キャッシュ
	//-----------------------------------------------------------------------------
	std::vector<Graphics::Animation::Detail::TrackKeyCursor> trackCursors{};	///< ノードごとのキー位置キャッシュ
	const Graphics::Import::AnimationClip* cursorClip = nullptr;				///< キャッシュが対応しているクリップ
	double cursorLastTicks = 0.0;											///< 前回評価した ticks（巻き戻り検出用）
};

//-----------------------------------------------------------------------------
// Animator<StateId>
//-----------------------------------------------------------------------------

template<typename StateId>
void Animator<StateId>::Initialize(
	const Graphics::Import::SkeletonCache* _skeletonCache,
	const Graphics::Animation::StateTable<StateId>* _stateTable,
	StateId _initState)
{
	this->skeletonCache = _skeletonCache;
	this->stateTable = _stateTable;
	this->currentState = _initState;

	this->currentTimeSec = 0.0f;
	this->normalizedTime = 0.0f;
	this->isFinished = false;
	this->isPaused = false;

	this->crossFadeData = Graphics::Animation::CrossFadeData<StateId>{};

	this->ApplyBindLocalAsBase();
	this->ResetTrackCursors();

	const auto* curDef = this->stateTable ? this->stateTable->Find(this->currentState) : nullptr;
	if (!curDef || !curDef->clip || !this->skeletonCache)
	{
		return;
	}

	// 先に焼き込む（これが無いと nodeIndex=-1 のままで更新されない）
	curDef->clip->BakeNodeIndices(*this->skeletonCache);

	// そのあと 0 秒評価（最初の見た目を正しくする）
	float nrm = 0.0f;
	bool fin = false;

	this->EvaluateClipLocalPose(
		curDef->clip,
		curDef->isLoop,
		0.0,
		this->localPose,
		nrm,
		fin);

	this->normalizedTime = nrm;
	this->isFinished = fin;
}

template<typename StateId>
void Animator<StateId>::RequestState(StateId _next, float _overrideFadeSec)
{
	if (!this->skeletonCache) { return; }
	if (!this->stateTable) { return; }
	if (_next == this->currentState) { return; }

	// 状態定義を取得
	const auto* fromDef = this->stateTable->Find(this->currentState);
	const auto* toDef = this->stateTable->Find(_next);
	if (!fromDef || !toDef) { return; }
	if (!toDef->clip) { return; }

	float fadeSec = toDef->recommendedCrossFadeSec;
	if (_overrideFadeSec >= 0.0f)
	{
		fadeSec = _overrideFadeSec;
	}

	if (fadeSec <= 0.0f)
	{
		this->currentState = _next;
		this->currentTimeSec = 0.0f;
		this->normalizedTime = 0.0f;
		this->isFinished = false;
		this->crossFadeData.isActive = false;

		this->ResetTrackCursors();

		// 先に焼き込み
		toDef->clip->BakeNodeIndices(*this->skeletonCache);

		// そのあと 0 秒評価
		float nrm = 0.0f;
		bool fin = false;

		this->EvaluateClipLocalPose(
			toDef->clip,
			toDef->isLoop,
			0.0,
			this->localPose,
			nrm,
			fin);

		this->normalizedTime = nrm;
		this->isFinished = fin;
		return;
	}

	//-----------------------------------------------------------------------------
	// クロスフェード情報をセットする
	//-----------------------------------------------------------------------------
	this->crossFadeData.isActive = true;
	this->crossFadeData.elapsed = 0.0f;
	this->crossFadeData.duration = fadeSec;

	this->crossFadeData.fromState = this->currentState;
	this->crossFadeData.toState = _next;

	this->crossFadeData.fromTime = this->currentTimeSec;
	this->crossFadeData.toTime = 0.0f;

	//-----------------------------------------------------------------------------
	// 重要：クロスフェードで使う両クリップを先に焼き込む
	// to 側が焼けてないと toPose がローカルバインドのままになり、バインドへ戻って見える
	//-----------------------------------------------------------------------------
	if (fromDef->clip)
	{
		fromDef->clip->BakeNodeIndices(*this->skeletonCache);
	}
	toDef->clip->BakeNodeIndices(*this->skeletonCache);

	this->currentState = _next;
	this->currentTimeSec = 0.0f;
	this->normalizedTime = 0.0f;
	this->isFinished = false;

	//-----------------------------------------------------------------------------
	// 状態が変わるのでキャッシュをリセットする
	//-----------------------------------------------------------------------------
	this->ResetTrackCursors();

	// 必要なら、ここで「遷移開始直後の見た目」を作っておく（任意）
	// （まずは上の Bake だけで症状が消えるか確認が最短）
	/*
	{
		float nrm = 0.0f;
		bool fin = false;
		this->EvaluateClipLocalPose(
			toDef->clip,
			toDef->isLoop,
			0.0,
			this->localPose,
			nrm,
			fin);
		this->normalizedTime = nrm;
		this->isFinished = fin;
	}
	*/
}

template<typename StateId>
void Animator<StateId>::Play()
{
	this->isFinished = false;
	this->isPaused = false;
}

template<typename StateId>
void Animator<StateId>::Stop()
{
	this->isPaused = true;
}

template<typename StateId>
void Animator<StateId>::Restart()
{
	// 再生位置を最初に戻す
	this->currentTimeSec = 0.0f;
	this->normalizedTime = 0.0f;
	this->isFinished = false;
	this->isPaused = false;
	this->crossFadeData.isActive = false;

	// キャッシュをリセットする
	this->ResetTrackCursors();

	// バインドローカル姿勢を基準として適用する
	this->ApplyBindLocalAsBase();
}

template<typename StateId>
void Animator<StateId>::Update(float _deltaTime)
{
	if (!this->skeletonCache)
	{
		OutputDebugStringA("[Animator] Update skip: skeletonCache=null\n");
		return;
	}
	if (!this->stateTable)
	{
		OutputDebugStringA("[Animator] Update skip: stateTable=null\n");
		return;
	}
	if (_deltaTime <= 0.0f)
	{
		OutputDebugStringA("[Animator] Update skip: dt<=0\n");
		return;
	}
	if (this->isPaused)
	{
		OutputDebugStringA("[Animator] Update skip: paused\n");
		return;
	}

	const auto* curDef = this->stateTable->Find(this->currentState);
	if (!curDef)
	{
		OutputDebugStringA("[Animator] Update skip: curDef=null\n");
		this->ApplyBindLocalAsBase();
		this->normalizedTime = 0.0f;
		this->isFinished = false;
		return;
	}
	if (!curDef->clip)
	{
		OutputDebugStringA("[Animator] Update skip: curDef->clip=null\n");
		this->ApplyBindLocalAsBase();
		this->normalizedTime = 0.0f;
		this->isFinished = false;
		return;
	}
	//-----------------------------------------------------------------------------
	// クロスフェード中かどうかで処理を分岐
	//-----------------------------------------------------------------------------
	if (!this->crossFadeData.isActive)
	{
		// 通常再生
		const double dt = static_cast<double>(_deltaTime) * static_cast<double>(curDef->playbackSpeed);
		this->currentTimeSec += static_cast<float>(dt);

		float nrm = 0.0f;
		bool fin = false;

		// クリップを評価してローカルポーズを作る
		this->EvaluateClipLocalPose(
			curDef->clip,
			curDef->isLoop,
			static_cast<double>(this->currentTimeSec),
			this->localPose,
			nrm,
			fin);

		this->normalizedTime = nrm;
		this->isFinished = fin;
		return;
	}

	// クロスフェード中
	const auto* fromDef = this->stateTable->Find(this->crossFadeData.fromState);
	const auto* toDef = this->stateTable->Find(this->crossFadeData.toState);

	if (!fromDef || !toDef || !fromDef->clip || !toDef->clip)
	{
		// 状態定義が見つからない or クリップが無い場合はバインドローカル姿勢を適用して終了
		this->crossFadeData.isActive = false;
		this->ApplyBindLocalAsBase();
		this->normalizedTime = 0.0f;
		this->isFinished = false;
		return;
	}

	// クロスフェード更新
	this->crossFadeData.elapsed += _deltaTime;
	this->crossFadeData.fromTime += _deltaTime * fromDef->playbackSpeed;
	this->crossFadeData.toTime += _deltaTime * toDef->playbackSpeed;

	Graphics::Animation::LocalPose fromPose{};
	Graphics::Animation::LocalPose toPose{};
	float fromNrm = 0.0f;
	float toNrm = 0.0f;
	bool fromFin = false;
	bool toFin = false;

	// 遷移前のクリップを評価してローカルポーズを作る
	this->EvaluateClipLocalPose(
		fromDef->clip,
		fromDef->isLoop,
		static_cast<double>(this->crossFadeData.fromTime),
		fromPose,
		fromNrm,
		fromFin);

	// 遷移後のクリップを評価してローカルポーズを作る
	this->EvaluateClipLocalPose(
		toDef->clip,
		toDef->isLoop,
		static_cast<double>(this->crossFadeData.toTime),
		toPose,
		toNrm,
		toFin);

	//-----------------------------------------------------------------------------
	// ローカルポーズ同士をTRSで補間して出力する
	//-----------------------------------------------------------------------------
	float w = 1.0f;
	if (this->crossFadeData.duration > 0.0f)
	{
		w = this->crossFadeData.elapsed / this->crossFadeData.duration;
		if (w < 0.0f) { w = 0.0f; }
		if (w > 1.0f) { w = 1.0f; }
	}

	// ローカルポーズ同士をTRSで補間して出力する
	this->BlendLocalPoseTRS(fromPose, toPose, w, this->localPose);

	this->normalizedTime = toNrm;
	this->isFinished = toFin;
	this->currentTimeSec = this->crossFadeData.toTime;

	if (w >= 1.0f)
	{
		// クロスフェード完了
		this->crossFadeData.isActive = false;
	}
}

template<typename StateId>
DX::Vector3 Animator<StateId>::InterpolateTranslation(
	const Graphics::Import::NodeTrack* _track,
	float _ticks,
	const DX::Vector3& _fallback)
{
	// 互換用：外部から呼ばれる可能性があるため残す（キャッシュなしの単純探索）
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

template<typename StateId>
DX::Quaternion Animator<StateId>::InterpolateRotation(
	const Graphics::Import::NodeTrack* _track,
	float _ticks,
	const DX::Quaternion& _fallback)
{
	// 互換用：外部から呼ばれる可能性があるため残す（キャッシュなしの単純探索）
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

template<typename StateId>
DX::Vector3 Animator<StateId>::InterpolateScale(
	const Graphics::Import::NodeTrack* _track,
	float _ticks,
	const DX::Vector3& _fallback)
{
	// 互換用：外部から呼ばれる可能性があるため残す（キャッシュなしの単純探索）
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

template<typename StateId>
float Animator<StateId>::GetNormalizedTime() const
{
	return this->normalizedTime;
}

template<typename StateId>
inline Graphics::Import::AnimationClip* Animator<StateId>::GetCurrentClip() const
{
	if(!this->stateTable){ return nullptr; }

	// 現在の状態定義を取得
	const auto* curDef = this->stateTable->Find(this->currentState);
	if (!curDef) { return nullptr; }

	return curDef->clip;
}

//-----------------------------------------------------------------------------
// バインドローカル姿勢を基準として適用する
//-----------------------------------------------------------------------------
template<typename StateId>
void Animator<StateId>::ApplyBindLocalAsBase()
{
	const size_t nodeCount = this->skeletonCache->nodes.size();
	this->localPose.localMatrices.assign(nodeCount, DX::Matrix4x4::Identity);

	for (size_t i = 0; i < nodeCount; i++)
	{
		this->localPose.localMatrices[i] = this->skeletonCache->nodes[i].bindLocalMatrix;
	}
}

template<typename StateId>
void Animator<StateId>::ResetTrackCursors()
{
	if (!this->skeletonCache)
	{
		this->trackCursors.clear();
		this->cursorClip = nullptr;
		this->cursorLastTicks = 0.0;
		return;
	}

	const size_t nodeCount = this->skeletonCache->nodes.size();
	this->trackCursors.assign(nodeCount, Graphics::Animation::Detail::TrackKeyCursor{});
	for (auto& c : this->trackCursors)
	{
		Graphics::Animation::Detail::ResetCursor(c);
	}

	this->cursorClip = nullptr;
	this->cursorLastTicks = 0.0;
}

template<typename StateId>
void Animator<StateId>::EvaluateClipLocalPose(
	const Graphics::Import::AnimationClip* _clip,
	bool _loop,
	double _timeSeconds,
	Graphics::Animation::LocalPose& _outPose,
	float& _outNormalizedTime,
	bool& _outFinished)
{
	_outNormalizedTime = 0.0f;
	_outFinished = false;

	if (!this->skeletonCache || !_clip)
	{
		if (this->skeletonCache)
		{
			_outPose.localMatrices.clear();
			_outPose.localMatrices.reserve(this->skeletonCache->nodes.size());
			for (const auto& n : this->skeletonCache->nodes)
			{
				_outPose.localMatrices.push_back(n.bindLocalMatrix);
			}
		}
		return;
	}

	const size_t nodeCount = this->skeletonCache->nodes.size();
	if (_outPose.localMatrices.size() != nodeCount)
	{
		_outPose.localMatrices.assign(nodeCount, DX::Matrix4x4::Identity);
	}

	for (size_t i = 0; i < nodeCount; ++i)
	{
		_outPose.localMatrices[i] = this->skeletonCache->nodes[i].bindLocalMatrix;
	}

	// キャッシュ配列サイズ保証
	if (this->trackCursors.size() != nodeCount)
	{
		this->ResetTrackCursors();
	}

	const double tps = _clip->ticksPerSecond;
	const double endTicks = Graphics::Animation::Detail::SafeClipEndTicks(_clip);

	if (tps <= Graphics::Animation::Detail::ForceEndTicksEps || endTicks <= Graphics::Animation::Detail::ForceEndTicksEps)
	{
		// 再生できない時はバインド姿勢のまま終了する
		_outNormalizedTime = 0.0f;
		_outFinished = (!_loop);
		return;
	}

	// 再生時間（秒）からティック位置を算出する
	double ticks = _timeSeconds * tps;

	if (_loop)
	{
		ticks = std::fmod(ticks, endTicks);
		if (ticks < 0.0) { ticks += endTicks; }
		_outFinished = false;
	}
	else
	{
		if (ticks >= endTicks)
		{
			ticks = endTicks;
			_outFinished = true;
		}
		if (ticks < 0.0) { ticks = 0.0; }
	}

	//-----------------------------------------------------------------------------
	// ticks の巻き戻り（ループ等）や clip 変更を検知したらキャッシュをリセットする
	//-----------------------------------------------------------------------------
	if (this->cursorClip != _clip || ticks + Graphics::Animation::Detail::ForceEndTicksEps < this->cursorLastTicks)
	{
		for (auto& c : this->trackCursors)
		{
			Graphics::Animation::Detail::ResetCursor(c);
		}
		this->cursorClip = _clip;
		this->cursorLastTicks = ticks;
	}
	else
	{
		this->cursorClip = _clip;
		this->cursorLastTicks = ticks;
	}

	//-----------------------------------------------------------------------------
	// 各トラックを評価してローカル行列を更新する
	//-----------------------------------------------------------------------------
	for (const auto& track : _clip->tracks)
	{
		const int nodeIndex = track.nodeIndex;
		if (nodeIndex < 0) { continue; }
		if (nodeIndex >= static_cast<int>(nodeCount)) { continue; }

		const size_t nodeIdx = static_cast<size_t>(nodeIndex);
		this->UpdateLocalMatrixFromKeysToPose(nodeIdx, ticks, track, _outPose);
	}

	//-----------------------------------------------------------------------------
	// 正規化時間を算出する
	//-----------------------------------------------------------------------------
	double t = ticks / endTicks;

	if (_loop)
	{
		t = t - std::floor(t);
		if (t < 0.0) { t += 1.0; }
		_outNormalizedTime = static_cast<float>(t);
	}
	else
	{
		if (t >= 1.0) { t = 1.0; }
		if (t < 0.0) { t = 0.0; }
		_outNormalizedTime = static_cast<float>(t);
	}
}

template<typename StateId>
DX::Vector3 Animator<StateId>::InterpolateVec3Cached(
	const std::vector<Graphics::Import::AnimKeyVec3>& _keys,
	double _ticks,
	const DX::Vector3& _fallback,
	size_t& _inOutLeftIndex)
{
	if (_keys.empty()) { return _fallback; }
	if (_keys.size() == 1) { _inOutLeftIndex = 0; return _keys[0].value; }

	// leftIndex は「左キー」を指す想定。範囲外を補正。
	if (_inOutLeftIndex >= _keys.size()) { _inOutLeftIndex = 0; }

	// まずはキャッシュ位置から前進して right を見つける（ticks が増える前提で高速）
	size_t leftIdx = _inOutLeftIndex;
	size_t rightIdx = leftIdx + 1;

	// leftIdx が最後尾だったら補正
	if (leftIdx >= _keys.size() - 1)
	{
		leftIdx = _keys.size() - 2;
		rightIdx = _keys.size() - 1;
	}

	// ticks が left より前なら、巻き戻り扱いとして lower_bound で区間を探す
	if (_ticks < _keys[leftIdx].ticksTime)
	{
		const auto it = std::upper_bound(
			_keys.begin(),
			_keys.end(),
			_ticks,
			[](double _t, const Graphics::Import::AnimKeyVec3& _k)
			{
				return _t < _k.ticksTime;
			});

		rightIdx = static_cast<size_t>(std::distance(_keys.begin(), it));
		if (rightIdx == 0) { _inOutLeftIndex = 0; return _keys.front().value; }
		if (rightIdx >= _keys.size()) { _inOutLeftIndex = _keys.size() - 2; return _keys.back().value; }

		leftIdx = rightIdx - 1;
		_inOutLeftIndex = leftIdx;
	}
	else
	{
		// ticks が進む場合：rightIdx を進めて区間を見つける
		while (rightIdx < _keys.size() && _keys[rightIdx].ticksTime <= _ticks)
		{
			++leftIdx;
			++rightIdx;
		}

		if (rightIdx >= _keys.size())
		{
			_inOutLeftIndex = _keys.size() - 2;
			return _keys.back().value;
		}

		_inOutLeftIndex = leftIdx;
	}

	const auto& leftKey = _keys[leftIdx];
	const auto& rightKey = _keys[leftIdx + 1];

	const double denomD = rightKey.ticksTime - leftKey.ticksTime;
	if (denomD <= Graphics::Animation::Detail::ForceEndTicksEps)
	{
		return leftKey.value;
	}

	const float denom = static_cast<float>(denomD);
	const float t = static_cast<float>((_ticks - leftKey.ticksTime) / denomD);

	return DX::Vector3::Lerp(leftKey.value, rightKey.value, t);
}

template<typename StateId>
DX::Quaternion Animator<StateId>::InterpolateQuatCached(
	const std::vector<Graphics::Import::AnimKeyQuat>& _keys,
	double _ticks,
	const DX::Quaternion& _fallback,
	size_t& _inOutLeftIndex)
{
	if (_keys.empty()) { return _fallback; }
	if (_keys.size() == 1) { _inOutLeftIndex = 0; return _keys[0].value; }

	if (_inOutLeftIndex >= _keys.size()) { _inOutLeftIndex = 0; }

	size_t leftIdx = _inOutLeftIndex;
	size_t rightIdx = leftIdx + 1;

	if (leftIdx >= _keys.size() - 1)
	{
		leftIdx = _keys.size() - 2;
		rightIdx = _keys.size() - 1;
	}

	if (_ticks < _keys[leftIdx].ticksTime)
	{
		const auto it = std::upper_bound(
			_keys.begin(),
			_keys.end(),
			_ticks,
			[](double _t, const Graphics::Import::AnimKeyQuat& _k)
			{
				return _t < _k.ticksTime;
			});

		rightIdx = static_cast<size_t>(std::distance(_keys.begin(), it));
		if (rightIdx == 0) { _inOutLeftIndex = 0; return _keys.front().value; }
		if (rightIdx >= _keys.size()) { _inOutLeftIndex = _keys.size() - 2; return _keys.back().value; }

		leftIdx = rightIdx - 1;
		_inOutLeftIndex = leftIdx;
	}
	else
	{
		while (rightIdx < _keys.size() && _keys[rightIdx].ticksTime <= _ticks)
		{
			++leftIdx;
			++rightIdx;
		}

		if (rightIdx >= _keys.size())
		{
			_inOutLeftIndex = _keys.size() - 2;
			return _keys.back().value;
		}

		_inOutLeftIndex = leftIdx;
	}

	const auto& leftKey = _keys[leftIdx];
	const auto& rightKey = _keys[leftIdx + 1];

	const double denomD = rightKey.ticksTime - leftKey.ticksTime;
	if (denomD <= Graphics::Animation::Detail::ForceEndTicksEps)
	{
		return leftKey.value;
	}

	const float t = static_cast<float>((_ticks - leftKey.ticksTime) / denomD);
	return DX::SlerpQuaternionSimple(leftKey.value, rightKey.value, t);
}

template<typename StateId>
void Animator<StateId>::BlendLocalPoseTRS(
	const Graphics::Animation::LocalPose& _from,
	const Graphics::Animation::LocalPose& _to,
	float _w,
	Graphics::Animation::LocalPose& _out)
{
	if (!this->skeletonCache) { return; }

	const size_t nodeCount = this->skeletonCache->nodes.size();
	if (_out.localMatrices.size() != nodeCount)
	{
		_out.localMatrices.assign(nodeCount, DX::Matrix4x4::Identity);
	}

	float w = _w;
	if (w < 0.0f) { w = 0.0f; }
	if (w > 1.0f) { w = 1.0f; }

	for (size_t i = 0; i < nodeCount; ++i)
	{
		const DirectX::XMMATRIX m0 = DX::LoadXMMATRIX(_from.localMatrices[i]);
		const DirectX::XMMATRIX m1 = DX::LoadXMMATRIX(_to.localMatrices[i]);

		DirectX::XMVECTOR s0{}, r0{}, t0{};
		DirectX::XMVECTOR s1{}, r1{}, t1{};

		DirectX::XMMatrixDecompose(&s0, &r0, &t0, m0);
		DirectX::XMMatrixDecompose(&s1, &r1, &t1, m1);

		const DirectX::XMVECTOR s = DirectX::XMVectorLerp(s0, s1, w);
		const DirectX::XMVECTOR r = DirectX::XMQuaternionSlerp(r0, r1, w);
		const DirectX::XMVECTOR t = DirectX::XMVectorLerp(t0, t1, w);

		const DirectX::XMMATRIX outM =
			DirectX::XMMatrixScalingFromVector(s) *
			DirectX::XMMatrixRotationQuaternion(r) *
			DirectX::XMMatrixTranslationFromVector(t);

		_out.localMatrices[i] = DX::StoreDXMatrix(outM);
	}
}

template<typename StateId>
void Animator<StateId>::UpdateLocalMatrixFromKeysToPose(
	size_t _nodeIdx,
	double _ticks,
	const Graphics::Import::NodeTrack& _track,
	Graphics::Animation::LocalPose& _outPose)
{
	auto& nodeInfo = this->skeletonCache->nodes[_nodeIdx];

	DirectX::XMMATRIX bindLocalMatrix = DX::LoadXMMATRIX(nodeInfo.bindLocalMatrix);
	DirectX::XMVECTOR bindLocalScale{}, bindLocalRotation{}, bindLocalTranslation{};
	DirectX::XMMatrixDecompose(&bindLocalScale, &bindLocalRotation, &bindLocalTranslation, bindLocalMatrix);

	DX::Vector3 finalPos{};
	DX::Vector3 finalScale{};
	DX::Quaternion finalRot{};
	{
		DirectX::XMFLOAT3 posFloat{};
		DirectX::XMFLOAT4 rotFloat{};
		DirectX::XMFLOAT3 scaleFloat{};

		DirectX::XMStoreFloat3(&posFloat, bindLocalTranslation);
		DirectX::XMStoreFloat4(&rotFloat, bindLocalRotation);
		DirectX::XMStoreFloat3(&scaleFloat, bindLocalScale);

		finalPos = DX::ToDXVector3(posFloat);
		finalRot = DX::ToDXQuaternion(rotFloat);
		finalScale = DX::ToDXVector3(scaleFloat);
	}

	//-----------------------------------------------------------------------------
	// キー探索「前回キー位置キャッシュ」から前進する方式
	//-----------------------------------------------------------------------------
	auto& cursor = this->trackCursors[_nodeIdx];

	if (_track.hasPosition)
	{
		finalPos = this->InterpolateVec3Cached(_track.positionKeys, _ticks, finalPos, cursor.posLeftIndex);
	}

	if (_track.hasRotation)
	{
		finalRot = this->InterpolateQuatCached(_track.rotationKeys, _ticks, finalRot, cursor.rotLeftIndex);
	}

	if (_track.hasScale)
	{
		finalScale = this->InterpolateVec3Cached(_track.scaleKeys, _ticks, finalScale, cursor.sclLeftIndex);
	}

	const DirectX::XMFLOAT3 scaleFloat = DX::ToXMFLOAT3(finalScale);
	const DirectX::XMFLOAT4 rotFloat = DX::ToXMFLOAT4(finalRot);
	const DirectX::XMFLOAT3 posFloat = DX::ToXMFLOAT3(finalPos);

	DirectX::XMMATRIX finalMatrix =
		DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&scaleFloat)) *
		DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&rotFloat)) *
		DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&posFloat));

	_outPose.localMatrices[_nodeIdx] = DX::StoreDXMatrix(finalMatrix);
}
