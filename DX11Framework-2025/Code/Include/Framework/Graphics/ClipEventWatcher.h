/** @file   ClipEventWatcher.h
 *  @brief  クリップイベント（正規化時刻）の通過判定を行う監視器
 *  @date   2026/02/12
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/AnimationData.h"

#include <vector>

//-----------------------------------------------------------------------------
// ClipEventWatcher class
//-----------------------------------------------------------------------------
namespace Graphics::Import
{
	/** @class ClipEventWatcher
	 *  @brief  prev < t <= now の通過判定で、通過したイベントIDを列挙する（正規化時間）
	 *  @details
	 *          - 時刻はすべて「正規化時間（0.0～1.0）」で扱う
	 *          - 秒は使用しない
	 *          - 通知は行わず、通過イベントIDを返すだけ
	 *          - 巻き戻り（now < prev）の場合は、このフレームでは何も出さない
	 */
	class ClipEventWatcher
	{
	public:
		/// @brief コンストラクタ
		ClipEventWatcher() = default;

		/// @brief デストラクタ
		~ClipEventWatcher() = default;

		/** @brief 監視状態を初期化する
		 *  @param _normalizedTime 初期の正規化時間（0.0～1.0）
		 */
		void Reset(float _normalizedTime = 0.0f);

		/** @brief 現在の正規化時間を与えて、通過したイベントIDを列挙する
		 *  @param _table クリップのイベントテーブル
		 *  @param _nowNormalizedTime 現在の正規化時間（0.0～1.0）
		 *  @param _outPassed 通過したイベントID（このフレーム分）
		 */
		void Update(
			const Graphics::Import::ClipEventTable* _table,
			float _nowNormalizedTime,
			std::vector<Graphics::Import::ClipEventId>& _outPassed
		);

		/// @brief 前回の正規化時間を取得する
		float GetPrevNormalizedTime() const { return this->prevNormalizedTime; }

	private:
		float prevNormalizedTime = 0.0f;	///< 前回の正規化時間（0.0～1.0）
		bool hasPrev = false;				///< prevNormalizedTime が有効か
	};
}