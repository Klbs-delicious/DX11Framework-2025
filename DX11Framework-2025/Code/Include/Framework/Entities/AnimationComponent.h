/** @file   AnimationComponent.h
 *  @brief  アニメーション更新専用コンポーネント
 *  @date   2026/01/19
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/MeshComponent.h"

#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/AnimationData.h"
#include "Include/Framework/Graphics/DynamicConstantBuffer.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class AnimationComponent
 *  @brief 固定更新フェーズでアニメーションを進めるコンポーネント
 *  @details 行ベクトル（mul(v, M)）で計算し、GPUへ送る直前に転置してVSに合わせる
 */
class AnimationComponent : public Component, public IFixedUpdatable
{
public:
	/// @brief ボーン行列用定数バッファ
	struct BoneBuffer
	{
		uint32_t boneCount;						///< ボーン数
		float pad[3]; 							///< パディング
		DX::Matrix4x4 boneMatrices[128];		///< ボーン行列（最大128本）
	};

	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	AnimationComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~AnimationComponent() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 固定更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void FixedUpdate(float _deltaTime) override;

	/** @brief ボーン行列用定数バッファをバインドする
	 *  @param _context デバイスコンテキスト
	 *  @param _slot バインドスロット
	 */
	void BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const;

	/** @brief アニメーションクリップを設定する（トラック解決もここで行う）
	 *  @param _clip 設定するアニメーションクリップ
	 */
	void SetAnimationClip(Graphics::Import::AnimationClip* _clip);

	/** @brief スケルトンキャッシュを設定する（モデル読み込み側で生成したものを渡す）
	 *  @param _cache スケルトンキャッシュ
	 */
	void SetSkeletonCache(const Graphics::Import::SkeletonCache* _cache);

	/** @brief 再生中か
	 *  @return 再生中なら true
	 */
	bool IsPlaying() const { return this->isPlaying; }

	/// @brief 再生開始
	void Play();

	/// @brief 停止
	void Stop();

	/** @brief 現在のアニメーションクリップ名を取得する
	 *  @return アニメーションクリップ名（読み取り専用）
	 */
	const std::string& GetCurrentAnimationName() const;

	/** @brief 再生速度を設定する
	 *  @param _speed 再生速度（1.0f が通常速度）
	 */
	void SetPlaybackSpeed(float _speed) { this->playbackSpeed = _speed; }

	/** @brief 現在のアニメーション時間を設定する
	 *  @param _time 現在のアニメーション時間（秒）
	 */
	void SetCurrentTime(double _time) { this->currentTime = _time; }

	/** @brief ループ再生を設定する
	* @param _isLoop ループするなら true
	*/
	void SetLoop(bool _isLoop) { this->isLoop = _isLoop; }

	/** @brief ループ再生か
	* @return ループするなら true
	*/
	bool IsLoop() const { return this->isLoop; }

private:
	void UpdatePoseFromClip(double _timeSeconds);
	void UpdateLocalMatrixFromKeys(size_t _nodeIdx, double _ticks, const Graphics::Import::NodeTrack& _track);
	DX::Vector3 InterpolateTranslation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback);
	DX::Quaternion InterpolateRotation(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Quaternion& _fallback);
	DX::Vector3 InterpolateScale(const Graphics::Import::NodeTrack* _track, float _ticks, const DX::Vector3& _fallback);

private:
	std::vector<DX::Matrix4x4> bindGlobalMatrices{}; ///< バインド姿勢の global（ノード数分、SetSkeletonCache時に作る）

	Graphics::Import::AnimationClip* currentClip = nullptr; ///< 現在のアニメーションクリップ
	MeshComponent* meshComponent = nullptr;                 ///< メッシュコンポーネント

	double clipEndTicks = 0.0;	///< 実キー終端（tracksの最後キー時刻の最大値）

	bool isPlaying = false;     ///< 再生中フラグ
	double currentTime = 0.0;   ///< 現在の再生時間（秒）
	float playbackSpeed = 1.0f; ///< 再生速度
	bool isLoop = false;		///< ループ再生

	const Graphics::Import::SkeletonCache* skeletonCache = nullptr;	///< スケルトンキャッシュ
	Graphics::Import::Pose currentPose{};							///< 現在のポーズ
	bool isSkeletonCached = false;									///< スケルトンキャッシュ設定済みか

	std::vector<int> trackToNodeIndex{};							///< trackIndex -> nodeIndex（解決結果）

	BoneBuffer boneBuffer{};
	std::unique_ptr<DynamicConstantBuffer<BoneBuffer>> boneCB;
};