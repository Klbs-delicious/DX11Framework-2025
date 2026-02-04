/** @file   TestDodge.h
 *  @brief  ジャスト回避挙動を検証するテスト用コンポーネント
 *  @date   2026/02/02
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/TimeScaleGroup.h"
#include "Include/Framework/Entities/Rigidbody3D.h"

#include "Include/Framework/Core/InputSystem.h"


//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class TestDodge
 *  @brief 固定更新で回避判定を行うテスト用コンポーネント
 *  @details 入力とタイミング判定のみを行い、演出や時間操作は別責務とする
 */
class TestDodge : public Component, public IFixedUpdatable,public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	TestDodge(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~TestDodge() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 固定更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void FixedUpdate(float _deltaTime) override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	bool isDodging;		///< 回避中か
	float dodgeTimer;	///< 回避受付時間

	InputSystem& inputSystem;			///< 入力処理を管理している
	AnimationComponent* animComponent;	///< アニメーションコンポーネント
	TimeScaleGroup* timeScaleGroup;		///< タイムスケールグループ
	Framework::Physics::Rigidbody3D* rigidbody;		///< 自身のRigidbody3Dコンポーネント

};
