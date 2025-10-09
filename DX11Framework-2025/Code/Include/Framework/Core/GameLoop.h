/**@file   GameLoop.h
 * @date   2025/09/12
 */
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Core/InputSystem.h"
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Shaders/ShaderManager.h"
#include"Include/Framework/Entities/GameObjectManager.h"
#include"Include/Scenes/SceneManager.h"

#include <memory>

 /**@class	GameLoop
  *	@brief	ゲーム進行の管理を行う
  */
class GameLoop :private NonCopyable
{
public:
	/// @brief	コンストラクタ
	GameLoop();
	/// @brief	デストラクタ
	~GameLoop();

	/**	@brief		初期化処理を行う
	 */
	void Initialize();

	/**	@brief		更新処理を行う
	 *	@param		float _deltaTime	デルタタイム
	 */
	void Update(float _deltaTime);

	/**	@brief		描画処理を行う
	 */
	void Draw();

	/**	@brief		終了処理を行う
	 */
	void Dispose();

	/**	@brief	ゲームが進行中かどうかを返す
	 *	@return	bool	trueなら進行中
	 */
	bool IsRunning()const { return this->isRunning; }

	/// @brief	ゲームループを抜ける
	void RequestExit() { this->isRunning = false; }

private:
	bool isRunning;			///< ゲームが進行中かどうか

	// @enum  ゲームの状態
	enum class GameState {
		Play = 0,
		Pause,
		NUM = Pause,
		MAX,
	};
	GameState gameState;	///< ゲームの状態

	std::unique_ptr<SceneManager> sceneManager;				///< シーン管理
	std::unique_ptr<InputSystem> inputSystem;				///< 入力の管理
	std::unique_ptr<GameObjectManager> gameObjectManager;	///< ゲームオブジェクトの管理
	std::unique_ptr<SpriteManager> spriteManager;			///< 画像データの管理
	std::unique_ptr<ShaderManager> shaderManager;			///< シェーダーの管理
	///< [TODO]物理、衝突の処理
	///< [TODO]サウンドの処理
	///< [TODO]UIの管理
	///< [TODO]ゲームの状態
};