/** @file   CharacterController.h
 *  @brief  キャラクターの操作を行う
 *  @date   2025/11/11
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Core/InputSystem.h"

 /** @class CharacterController
  *  @brief キャラクターの移動・操作を管理する
  */
class CharacterController : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param GameObject* _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param bool _active コンポーネントの有効/無効
	 */
	CharacterController(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~CharacterController() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/** @brief 更新処理
	 *  @param float _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	InputSystem& inputSystem;	// 入力処理を管理している

	float moveSpeed;		// 移動速度
};

