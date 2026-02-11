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
#include "Include/Framework/Graphics/Animator.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;
class IAnimator;

/** @class AnimationComponent
 *  @brief 固定更新フェーズでボーン行列を更新し、GPUへ送るコンポーネント
 *  @details 行ベクトル（mul(v, M)）で計算し、GPUへ送る直前に転置してVSに合わせる
 */
class AnimationComponent : public Component, public IUpdatable
{
public:
	/// @brief ボーン行列用定数バッファ
	struct BoneBuffer
	{
		uint32_t boneCount;						///< ボーン数
		float pad[3];							///< パディング
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

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/** @brief ボーン行列用定数バッファをバインドする
	 *  @param _context デバイスコンテキスト
	 *  @param _slot バインドスロット
	 */
	void BindBoneCBVS(ID3D11DeviceContext* _context, UINT _slot) const;

	/** @brief スケルトンキャッシュを設定する（モデル読み込み側で生成したものを渡す）
	 *  @param _cache スケルトンキャッシュ
	 */
	void SetSkeletonCache(const Graphics::Import::SkeletonCache* _cache);

	/** @brief アニメーターを設定する（所有する）
	 *  @param _animator アニメーター
	 */
	void SetAnimator(std::unique_ptr<IAnimator> _animator);
	
	/** @brief アニメーションの再生を開始する
	 *  @note ループ設定はアニメーター側で行うこと
	 */
	template<typename StateId>
	void RequestState(StateId _next, float _fadeSec = -1.0f)
	{
		// アニメーターが設定されていなければ無視
		auto* animator = dynamic_cast<Animator<StateId>*>(this->animator.get());
		if (!animator) { return; }

		animator->RequestState(_next, _fadeSec);
	}

	/// @brief 再生を開始する
	void Play();

	/// @brief 停止する
	void Stop();

	/// @brief 最初から再生する
	void Restart();

private:
	/// @brief ボーン用定数バッファを更新する（Pose -> BoneBuffer）
	void UpdateBoneBufferFromPose();

private:
	std::unique_ptr<IAnimator> animator;					///< アニメーター（LocalPose生成）
	MeshComponent* meshComponent = nullptr;					///< メッシュコンポーネント

	BoneBuffer boneBuffer{};									///< GPUへ送るデータ
	std::unique_ptr<DynamicConstantBuffer<BoneBuffer>> boneCB;	///< 定数バッファ

	const Graphics::Import::SkeletonCache* skeletonCache = nullptr; ///< スケルトンキャッシュ
	Graphics::Import::Pose currentPose{};							///< 現在のポーズ（global/skin/cpuBoneMatrices）
	bool isSkeletonCached = false;									///< スケルトンキャッシュ設定済みか
};