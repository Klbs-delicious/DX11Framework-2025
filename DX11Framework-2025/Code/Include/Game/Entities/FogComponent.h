/** @file   FogComponent.h
 *  @brief  フォグ演出を制御するコンポーネント
 *  @date   2025/12/19
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Graphics/DynamicConstantBuffer.h"
#include "Include/Framework/Utils/CommonTypes.h"
#include "Include/Framework/Entities/Camera3D.h"

#include <memory>

// HLSL Common.hlsli の FogBuffer(b5) に対応するCPU側構造体
struct FogBuffer
{
	DX::Vector3 cameraPos; // カメラ位置（ワールド座標）
	float       fogStart;  // フォグ開始距離
	float       fogEnd;    // フォグ終了距離
	DX::Vector3 fogColor;  // フォグ色（RGB）
	
	// レンジ揺らぎ用
	float       timeSec;   // 経過時間（秒）
	float       waveSpeed; // 揺らぎ速度
	float       waveAmp;   // 揺らぎ強度
	float       pad;       // アラインメント
};

// HLSL NormalMatrixBuffer(b6) に対応するCPU側構造体（行ベクトル3本）
struct NormalMatrixBuffer
{
	DX::Vector3 row0;
	DX::Vector3 row1;
	DX::Vector3 row2;
	float       pad2; // 16バイトアラインメント用
};

 /** @class FogComponent
  *  @brief フォグ演出を制御するコンポーネント
  *  @details Update フェーズでフォグ関連のパラメータ更新を行う
  */
class FogComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	FogComponent(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~FogComponent() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間
	 */
	void Update(float _deltaTime) override;

	// レンジ揺らぎの設定
	void SetRangeWave(float speed, float amplitude) { this->waveSpeed = speed; this->waveAmp = amplitude; }

private:
	std::unique_ptr<DynamicConstantBuffer<FogBuffer>> fogBuffer;            ///< フォグ用定数バッファ（b5）
	std::unique_ptr<DynamicConstantBuffer<NormalMatrixBuffer>> normalBuffer; ///< 法線行列定数バッファ（b6）
	Camera3D* camera;                                                         ///< カメラ参照
	float elapsedSec = 0.0f;                                                  ///< 経過時間
	float waveSpeed = 0.8f;                                                   ///< 速度
	float waveAmp = 0.01f;                                                   ///< 強度
};
