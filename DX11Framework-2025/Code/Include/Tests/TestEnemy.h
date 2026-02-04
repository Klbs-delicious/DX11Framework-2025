/** @file   TestEnemy.h
 *  @brief  テスト用エネミー挙動コンポーネント
 *  @date   2026/01/18
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class TestEnemy
 *  @brief 固定更新で簡易的な敵挙動を行うテスト用コンポーネント
 *  @details 攻撃タイミングや状態遷移のみを扱い、演出やダメージ処理は別責務とする
 */
class TestEnemy : public Component,
	public IFixedUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	TestEnemy(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~TestEnemy() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 固定更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void FixedUpdate(float _deltaTime) override;

private:
	bool isAttacking;	///< 攻撃中か
	float attackTimer;	///< 攻撃タイマー
};

