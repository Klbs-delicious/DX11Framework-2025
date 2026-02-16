/** @file   TimeScaleTestComponent.h
 *  @brief  時間スケールの挙動を検証するテスト用コンポーネント
 *  @date   2025/11/12
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Core/InputSystem.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

 /** @class TimeScaleTestComponent
  *  @brief 時間スケールの挙動を検証するテスト用コンポーネント
  *  @details - Componentを継承し、UpdateフェーズでTimeScale関連の動作確認を行う
  */
class TimeScaleTestComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	TimeScaleTestComponent(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~TimeScaleTestComponent() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	InputSystem& inputSystem;			///< 入力システムの参照
	TimeScaleSystem& timeScaleSystem;	///< 時間スケールシステムの参照
};