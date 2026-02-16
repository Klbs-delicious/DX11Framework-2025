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

#include "Include/Game/Entities/DodgeComponent.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class TestDodge
 *  @brief 固定更新で回避判定を行うテスト用コンポーネント
 *  @details 入力とタイミング判定のみを行い、演出や時間操作は別責務とする
 */
class TestDodge : public Component,public IUpdatable
{
public:
	enum class TestPlayerAnimState
	{
		Idle,		///< 待機
		Dodging,	///< 回避
		Jumping		///< ジャンプ
	};

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

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	InputSystem& inputSystem;			///< 入力処理を管理している
	AnimationComponent* animComponent;	///< アニメーションコンポーネント
	Framework::Physics::Rigidbody3D* rigidbody;		///< 自身のRigidbody3Dコンポーネント

	DodgeComponent* dodgeComponent;	///< 回避コンポーネント

	bool prevIsDodging;			///< 前フレームの回避中
	bool prevTimingValid;		///< 前フレームの判定猶予有効

};
