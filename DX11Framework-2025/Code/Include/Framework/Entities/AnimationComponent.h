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

	/** @brief モデルデータを設定する
	 *  @param _modelData モデルデータ
	 */
	void SetModelData(Graphics::Import::ModelData* _modelData);

	/** @brief デバッグ用：ボーン行列を転置してGPUへ送るか
	 *  @details 行列の流儀（row-major/column-major）が合っていない場合の切り分け用
	 */
	// Row-vector CPU convention: keep this enabled to transpose before GPU upload.
	void SetTransposeBoneMatricesOnUpload(bool _enable) { this->transposeBoneMatricesOnUpload = _enable; }

	/// @brief 現在の設定を取得（デバッグ用）
	bool GetTransposeBoneMatricesOnUpload() const { return this->transposeBoneMatricesOnUpload; }

private:
	struct BindTRS
	{
		aiVector3D t{};
		aiQuaternion r{};
		aiVector3D s{ 1.0f, 1.0f, 1.0f };
	};

	// HLSL `Common.hlsli`:
	// cbuffer BoneBuffer : register(b7)
	// {
	//     float4x4 boneMatrices[256];
	//     uint boneCount;
	//     uint4 _pad0;
	//     uint4 _pad1;
	//     uint4 _pad2;
	// }
	struct BoneBuffer
	{
		static constexpr size_t MaxBones = 256;
		std::array<DX::Matrix4x4, MaxBones> boneMatrices{};
		uint32_t boneCount = 0;
		DX::Vector3 _pad0{ 0,0,0 }; // 16-byte alignment for `boneCount`
		DX::Vector4 _pad1{ 0,0,0,0 };
		DX::Vector4 _pad2{ 0,0,0,0 };
		DX::Vector4 _pad3{ 0,0,0,0 };
	};

	// -----------------------------
	// References
	// -----------------------------
	Graphics::Import::AnimationClip* currentClip = nullptr; ///< 現在のアニメーションクリップ
	MeshComponent* meshComponent = nullptr;                ///< メッシュコンポーネント
	Graphics::Import::ModelData* modelData = nullptr;      ///< 対象モデルデータ

	// -----------------------------
	// Playback
	// -----------------------------
	bool isPlaying = false;        ///< 再生中フラグ
	double currentTime = 0.0;      ///< 現在の再生時間（秒）
	float playbackSpeed = 1.0f;    ///< 再生速度

	// -----------------------------
	// Skeleton cache
	// -----------------------------
	bool isSkeletonCached = false; ///< スケルトンキャッシュ生成済みか
	std::vector<std::string> nodeNames{};                 ///< index -> nodeName
	std::unordered_map<std::string, int> nodeIndexMap{};  ///< nodeName -> nodeTreeIndex
	std::vector<int> parentIndex{};                       ///< nodeTreeIndex -> parentIndex（rootは-1）
	std::vector<int> boneIndexMap{};                      ///< nodeTreeIndex -> Bone::index（頂点boneIndexと一致させる）
	DX::Matrix4x4 inverseRootBind = DX::Matrix4x4::Identity; ///< root globalBind の逆行列
	std::vector<BindTRS> bindTrs{};                       ///< bind姿勢のTRS（fallback用）

	// Current pose
	std::vector<DX::Matrix4x4> localPose{};       ///< 現在のローカル行列
	std::vector<DX::Matrix4x4> globalPose{};      ///< 現在グローバル行列
	std::vector<DX::Matrix4x4> skinMatrices{};    ///< CPU計算した最終行列（デバッグ用）

	// Bind data
	std::vector<DX::Matrix4x4> localBindPose{};   ///< モデルの初期ローカル姿勢
	std::vector<DX::Matrix4x4> offsetMatrices{};  ///< ボーンごとのオフセット行列

	// -----------------------------
	// GPU upload
	// -----------------------------
	bool transposeBoneMatricesOnUpload = false;   ///< デバッグ用スイッチ
	BoneBuffer boneBuffer{};                     ///< CPU側の送信データ
	std::unique_ptr<DynamicConstantBuffer<BoneBuffer>> boneCB; ///< GPU定数バッファ
};
