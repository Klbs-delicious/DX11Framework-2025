#pragma once
#include "Include/Framework/Graphics/AnimationData.h"
#include "Include/Framework/Graphics/ModelData.h"

/** @class IAnimator
 *  @brief ローカルポーズを生成する共通インターフェイス
 */
class IAnimator
{
public:
	virtual ~IAnimator() = default;

	/** @brief アニメーションを更新する
	 *  @param _dt デルタ時間（秒）
	 */
	virtual void Update(float _dt) = 0;

	/** @brief 現在のローカルポーズを取得する
	 *  @return ローカルポーズ参照
	 */
	virtual const Graphics::Animation::LocalPose& GetLocalPose() const = 0;

	/** @brief 現在の正規化時間（0.0～1.0）を取得する
	 *  @return 正規化時間
	 */
	virtual float GetNormalizedTime() const = 0;

	/** @brief 終了しているかを取得する（非ループ時）
	 *  @return 終了していれば true
	 */
	virtual bool IsFinished() const = 0;

	/// @brief アニメーションを再生する
	virtual void Play() = 0;

	/// @brief アニメーションを停止する
	virtual void Stop() = 0;

	/// @brief アニメーションを最初から再生する
	virtual void Restart() = 0;
};
