/**	@file	SceneManager.h
*	@date	2025/07/04
*/
#pragma once
#include"Framework/Scenes/SceneFactory.h"
#include"Framework/Scenes/SceneType.h"
#include"Framework/Scenes/BaseScene.h"

#include <memory>
#include <functional>

/**	@class	SceneManager
 *	@brief	シーンの遷移、保持
 */
class SceneManager
{
public:
	/**	@brief	コンストラクタ
	 *	@param	std::unique_ptr<SceneFactory>	_factory	シーン生成を行うファクトリクラス
	 */
	SceneManager(std::unique_ptr<SceneFactory> _factory);
	/// @brief	デストラクタ
	~SceneManager();

	/**	@brief	シーンの更新を行う
	 *	@param	float _deltaTime	デルタタイム
	 */
	void Update(float _deltaTime);

	/// @brief	シーンの描画を行う
	void Draw();

	/**	@brief	シーン遷移をリクエストする
	 *	@param	SceneType _nextScenetype	次のシーンタイプ
	 */
	void RequestSceneChange(SceneType _nextScenetype);

	/**	@brief	演出や外部トリガー向けのコールバック設定を行う
	 *	@param	std::function<void(SceneType)> _callback	演出や外部トリガー向けのコールバックコールバック
	 */
	void SetTransitionCallback(std::function<void(SceneType)> _callback);

private:
	/**	@brief	遷移開始処理を行う
	 *	@param	SceneType _nextSceneType	次のシーンタイプ
	 */
	void BeginTransition(SceneType _nextSceneType);

	/// @brief Factoryを使ってシーン生成・切り替えを行う
	void CompleteTransition();

private:
	std::unique_ptr<SceneFactory> sceneFactory;							///< シーン生成を行うファクトリクラス
	std::unique_ptr<BaseScene>	currentScene;							///< 現在のシーン
	SceneType currentSceneType;											///< 現在のシーンタイプ
	SceneType pendingSceneType;											///< 切り替え予約シーンタイプ
	std::function<void(SceneType _nextSceneType)> transitionCallback;	///< 遷移のトリガーを行うコールバック
	
	bool isTransitioning = false;		///< 遷移フラグ
	bool isSceneInitialized = false;	///< 初期化チェック
};