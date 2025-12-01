/**	@file	SceneManager.h
*	@date	2025/07/04
*/
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Scenes/SceneFactory.h"
#include"Include/Framework/Scenes/SceneType.h"
#include"Include/Framework/Scenes/BaseScene.h"

#include <memory>
#include <functional>

/**	@class	SceneManager
 *	@brief	シーンの遷移、保持
 *	@details	このクラスはコピー、代入を禁止している
 */
class SceneManager :private NonCopyable
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

	/// @brief	保留中のシーン破棄を行う
	void FlushPendingDestroys();

	/**	@brief	シーン遷移をリクエストする
	 *	@param	SceneType _nextScenetype	次のシーンタイプ
	 */
	void RequestSceneChange(SceneType _nextScenetype);

	/**	@brief	遷移開始時の演出や外部トリガー向けのコールバック設定を行う
	 *	@param	std::function<void(SceneType)> _callback	演出や外部トリガー向けのコールバック
	 */
	void SetTransitionCallback(std::function<void(SceneType)> _callback);

	/**	@brief	遷移開始処理を行うためのラッパー関数
	 *	@param	SceneType _nextSceneType	次のシーンタイプ
	 */
	void NotifyTransitionReady(SceneType _nextSceneType);

private:
	/**	@brief	遷移開始処理を行う
	 *	@param	SceneType _nextSceneType	次のシーンタイプ
	 */
	void BeginTransition(SceneType _nextSceneType);

	/// @brief Factoryを使ってシーン生成・切り替えを行う
	void CompleteTransition();

	/// @brief 終了処理
	void Dispose();
private:
	std::unique_ptr<SceneFactory> sceneFactory;							///< シーン生成を行うファクトリクラス
	std::unique_ptr<BaseScene>	currentScene;							///< 現在のシーン
	SceneType currentSceneType;											///< 現在のシーンタイプ
	SceneType pendingSceneType;											///< 切り替え予約シーンタイプ

	std::function<void(SceneType _nextSceneType)> onTransitionBegin;  ///< 遷移開始通知イベント
	std::function<void(SceneType _nextSceneType)> onTransitionEnd;    ///< 遷移完了通知イベント
	
	bool isTransitioning = false;		///< 遷移フラグ
	bool isSceneInitialized = false;	///< 初期化チェック
};