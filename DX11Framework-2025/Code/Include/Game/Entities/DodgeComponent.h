/** @file   DodgeComponent.h
 *  @brief  回避（Dodge）状態の管理を行うコンポーネント
 *  @date   2026/02/10
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/ITimeProvider.h"

//-----------------------------------------------------------------------------
// DodgeComponent class
//-----------------------------------------------------------------------------

/** @class  DodgeComponent
 *  @brief  回避中かどうか等の状態を Update で管理する
 */
class DodgeComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	DodgeComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~DodgeComponent() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 解放処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/** @brief 回避状態を開始する
	 *  @param _duration 回避持続時間（秒）
	 */
	void StartDodge(float _duration = 0.0f);

	/// @brief 回避状態を終了する
	void EndDodge();

	/** @brief 回避中かどうかを取得する
	 *  @return bool 回避中なら true
	 */
	bool IsDodging() const { return this->isDodging; }

	/** @brief ジャスト回避判定猶予が有効かどうかを取得する
	 *  @return bool 有効なら true
	 */
	bool IsDodgeTimingValid() const { return this->dodgeTimingRemaining > 0.0f; }

private:
	bool isDodging;					///< 回避中か
	float dodgeTimer;				///< 回避経過時間

	float dodgeTimingRemaining;		///< ジャスト回避判定猶予の残り時間

	float defaultDodgeDuration;		///< 回避全体の標準持続時間
	float currentDodgeDuration;		///< 今回の回避に使用する持続時間
	float justDodgeWindowDuration;	///< ジャスト回避判定猶予の持続時間（0.15）

	ITimeProvider& timeSystem;		///< 時間情報提供システム
};