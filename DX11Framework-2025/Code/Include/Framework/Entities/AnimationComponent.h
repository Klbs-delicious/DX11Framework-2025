/** @file   AnimationComponent.h
 *  @brief  アニメーション更新専用コンポーネント
 *  @date   2026/01/13
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

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <memory>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class AnimationComponent
 *  @brief 固定更新フェーズでアニメーションを進めるコンポーネント
 *  @details 物理時間と同期したボーン行列更新を想定する
 */
class AnimationComponent : public Component, public IFixedUpdatable
{
public:
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

private:
	struct BindTRS
	{
		aiVector3D t{};
		aiQuaternion r{};
		aiVector3D s{ 1.0f, 1.0f, 1.0f };
	};

	Graphics::Import::AnimationClip* currentClip;	///< 現在のアニメーションクリップ
	MeshComponent* meshComponent;	///< メッシュコンポーネント
	Graphics::Import::ModelData* modelData;		///< モデルデータ

	bool	isPlaying;		///< アニメーション再生中か
	double	currentTime;	///< 現在のアニメーション時間（秒）
	float	playbackSpeed;	///< 再生速度

	bool isSkeletonCached = false;	///< スケルトンキャッシュ生成済みか

	std::vector<std::string> nodeNames{};						///< index -> nodeName
	std::unordered_map<std::string, int> nodeIndexMap{};		///< nodeName -> index
	std::vector<int> parentIndex{};								///< index -> parentIndex（rootは-1）

	DX::Matrix4x4 inverseRootBind = DX::Matrix4x4::Identity;	///< root globalBind の逆行列

	std::vector<BindTRS> bindTrs{};								///< bind姿勢のTRS（fallback用）

	std::vector<DX::Matrix4x4> localPose{};			///< 現在のローカル行列
	std::vector<DX::Matrix4x4> globalPose{};		///< 現在グローバル行列
	std::vector<DX::Matrix4x4> skinMatrices{};		///< GPU に渡す最終行列

	std::vector<DX::Matrix4x4> localBindPose{};		///< モデルの初期ローカル姿勢
	std::vector<DX::Matrix4x4> offsetMatrices{};	///< ボーンごとのオフセット行列

	static constexpr size_t MaxBones = 256;

	/** @struct BoneCBData
	 *  @brief ボーン行列用定数バッファデータ構造
	 */
	struct BoneBuffer
	{
		std::array<DX::Matrix4x4, MaxBones> skin{};	///< 最終スキン行列
		UINT boneCount = 0;							///< 実ボーン数
		UINT _pad0 = 0;
		UINT _pad1 = 0;
		UINT _pad2 = 0;
	};

	std::unique_ptr<DynamicConstantBuffer<BoneBuffer>> skinningBuffer;	///< スキニング用定数バッファ
	BoneBuffer boneBuffer{};											///< 定数バッファ用データ
};