/** @file   PostEffectController.cpp
 *  @brief  ポストエフェクトの制御を行う
 *  @date   2026/05/24
 */

#include "Include/Game/Graphics/PostProcess/PostEffectController.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/TimeScaleSystem.h"
#include "Include/Game/Graphics/PostProcess/SepiaEffectPass.h"

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 */
PostEffectController::PostEffectController(GameObject* _owner) :
    Component(_owner, true),
    timeScaleSystem(nullptr),
    sepiaEffectPass(nullptr),
    smoothIntensity(0.0f),
	justDodgeSepiaIntensity(1.0f),
    fadeSpeed(5.0f)
{
}

/// @brief 初期化処理
void PostEffectController::Initialize()
{
    this->timeScaleSystem = &SystemLocator::Get<TimeScaleSystem>();
}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void PostEffectController::Update(float _deltaTime)
{
    if (!this->timeScaleSystem || !this->sepiaEffectPass)
    {
        return;
    }

    // TimeScaleSystemから、「現在のスローの強さ」を取得
    TimeScaleEffectContext context = this->timeScaleSystem->GetEffectContext(TimeScaleEventId::TestDodge);

    float targetIntensity = 0.0f;
    if (context.isActive)
    {
        // ジャスト回避中は、設定されたセピア強度を目標にする
        targetIntensity = this->justDodgeSepiaIntensity;
    }

    // フェード処理
    if (this->smoothIntensity < targetIntensity)
    {
        this->smoothIntensity += _deltaTime * this->fadeSpeed;

        if (this->smoothIntensity > targetIntensity)
        {
            this->smoothIntensity = targetIntensity;
        }
    }
    else if (this->smoothIntensity > targetIntensity)
    {
        // 目標強度に向かってフェードアウトを行う
        this->smoothIntensity -= _deltaTime * this->fadeSpeed;

        if (this->smoothIntensity < targetIntensity)
        {
            this->smoothIntensity = targetIntensity;
        }
    }

    this->sepiaEffectPass->SetIntensity(this->smoothIntensity);
}

/** @brief 制御対象のSepiaEffectPassを設定
 *  @param _sepiaPass SepiaEffectPassのポインタ
 */
void PostEffectController::SetSepiaEffectPass(SepiaEffectPass* _sepiaPass)
{
    this->sepiaEffectPass = _sepiaPass;
}

