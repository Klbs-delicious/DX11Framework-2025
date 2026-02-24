/** @file   DebugSceneChanger.h
 *  @brief  デバッグ用シーン切り替えコンポーネント
 *  @date   2026/01/18
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Scenes/SceneManager.h"
#include "Include/Framework/Core/InputSystem.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class GameObject;

/** @class DebugSceneChanger
 *  @brief 固定更新でシーン切り替えを監視するデバッグ専用コンポーネント
 *  @details 入力や条件を満たした場合にシーン遷移を行うことを想定する
 */
class DebugSceneChanger : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	DebugSceneChanger(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~DebugSceneChanger() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param float _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	SceneManager& sceneManager;	///< シーンマネージャーの参照
	InputSystem& inputSystem;	///< 入力処理を管理している

	SceneType currentSceneType;	///< 現在のシーンタイプ
};

