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
#include <unordered_map>
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
		std::array<DX::Matrix4x4, ShaderCommon::MaxBones> boneMatrices{};
		uint32_t boneCount = 0;
		DX::Vector3 _pad0{ 0,0,0 };
		DX::Vector4 _pad1{ 0,0,0,0 };
		DX::Vector4 _pad2{ 0,0,0,0 };
		DX::Vector4 _pad3{ 0,0,0,0 };
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

	/** @brief nodeTree と boneDictionary から SkeletonCache を構築する
	 *  @param _nodeTree モデルのノードツリー
	 *  @param _boneDict モデルのボーン辞書
	 *  @param _outCache 出力先キャッシュ
	 */
	void BuildSkeletonCache(
		const Utils::TreeNode<Graphics::Import::BoneNode>& _nodeTree, 
		const std::unordered_map<std::string, Graphics::Import::Bone>& _boneDict, 
		Graphics::Import::SkeletonCache& _outCache
	);

	/** @brief スケルトンキャッシュを再構築する
	 *  @details モデルデータ変更時に呼び出すこと
	 */
	void RebuildSkeletonCache();

	/** @brief ボーン行列用定数バッファをバインドする
	 *  @param _context デバイスコンテキスト
	 *  @param _slot バインドスロット
	 */
	void BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const;

	/** @brief アニメーションクリップを設定する
	 *  @param _clip 設定するアニメーションクリップ
	 */
	void SetAnimationClip(Graphics::Import::AnimationClip* _clip);

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

	/** @brief モデルデータを設定する
	 *  @param _modelData モデルデータ
	 */
	void SetModelData(Graphics::Import::ModelData* _modelData);

private:
	DX::Matrix4x4 SampleLocalFromTrack(
		const Graphics::Import::NodeTrack& track,
		double tickTicks,
		double ticksPerSecond,
		double durationTicks);

private:

	Graphics::Import::AnimationClip* currentClip = nullptr; ///< 現在のアニメーションクリップ
	MeshComponent* meshComponent = nullptr;                    ///< メッシュコンポーネント
	Graphics::Import::ModelData* modelData = nullptr;        ///< 対象モデルデータ

	bool isPlaying = false;     ///< 再生中フラグ
	double currentTime = 0.0;   ///< 現在の再生時間（秒）
	float playbackSpeed = 1.0f; ///< 再生速度

	Graphics::Import::SkeletonCache skeletonCache{};	///< スケルトンキャッシュ
	Graphics::Import::Pose currentPose{};				///< 現在のポーズ
	bool isSkeletonCached = false;                        ///< スケルトンキャッシュ生成済みか

	// ボーン行列用定数バッファ
	BoneBuffer boneBuffer{};
	std::unique_ptr<DynamicConstantBuffer<BoneBuffer>> boneCB;

	// Debug logging control: print bursts of frames instead of every frame
	bool enableBoneDebugLog = true;        ///< enable/disable bone debug logging
	int debugLogFrameCounter = 0;         ///< incremented each FixedUpdate
	int debugLogBurstSize = 4;            ///< number of consecutive frames to log
	int debugLogPeriod = 60;              ///< period (frames) between bursts
};