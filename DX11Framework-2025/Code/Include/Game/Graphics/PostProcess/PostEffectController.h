/** @file   PostEffectController.h
 *  @brief  ポストエフェクトの制御を行う
 *  @date   2026/05/24
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

class TimeScaleSystem;
class SepiaEffectPass;

/** @class PostEffectController
 *  @brief ポストエフェクトのパラメータを調整・更新する
 */
class PostEffectController : public Component, public IUpdatable
{
public:
    /** @brief コンストラクタ
     *  @param _owner このコンポーネントがアタッチされるオブジェクト
     */
    PostEffectController(GameObject* _owner);

    /// @brief 初期化処理
    void Initialize() override;

    /** @brief 更新処理
     *  @param _deltaTime 前フレームからの経過時間（秒）
     */
    void Update(float _deltaTime) override;

    /** @brief 制御対象のSepiaEffectPassを設定
     *  @param _sepiaPass SepiaEffectPassのポインタ
     */
    void SetSepiaEffectPass(SepiaEffectPass* _sepiaPass);

private:
    TimeScaleSystem* timeScaleSystem;   ///< タイムスケールシステムのポインタ
    SepiaEffectPass* sepiaEffectPass;   ///< セピアエフェクトパスのポインタ
    float smoothIntensity;              ///< 平滑化された強度
    float justDodgeSepiaIntensity;      ///< ジャスト回避中のセピア強度
	float fadeSpeed;                    ///< フェードの速度
};

