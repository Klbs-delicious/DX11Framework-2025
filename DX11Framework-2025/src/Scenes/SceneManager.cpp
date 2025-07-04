/**	@file	SceneManager.cpp
*	@date	2025/07/04
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Scenes/SceneManager.h"

#include<assert.h>

//-----------------------------------------------------------------------------
// SceneManager Class
//-----------------------------------------------------------------------------

/**	@brief	コンストラクタ
 *	@param	std::unique_ptr<SceneFactory>	_factory	シーン生成を行うファクトリクラス
 */
SceneManager::SceneManager(std::unique_ptr<SceneFactory> _factory) :sceneFactory(std::move(_factory))
{
	// 始めに遷移するシーンタイプを初期化
	this->currentSceneType = SceneType::Test;
	this->pendingSceneType = SceneType::Test;
}
/// @brief	デストラクタ
SceneManager::~SceneManager() {}

/**	@brief	シーンの更新を行う
 *	@param	float _deltaTime	デルタタイム
 */
void SceneManager::Update(float _deltaTime)
{    
	// 遷移フラグが立っていればシーン切り替えを行う
	if (this->isTransitioning)
	{
		this->CompleteTransition();
	}

	if (this->currentScene)
	{
		// 初期化が行われていなければ初期化処理を行う
		if (!this->isSceneInitialized)
		{
			this->currentScene->Initialize();
			this->isSceneInitialized = true;
		}
		this->currentScene->Update(_deltaTime);
	}
}

/// @brief	シーンの描画を行う
void SceneManager::Draw()
{
	if (this->currentScene)
	{
		this->currentScene->Draw();
	}
}

/**	@brief	シーン遷移をリクエストする
 *	@param	SceneType _nextScenetype	次のシーンタイプ
 */
void SceneManager::RequestSceneChange(SceneType _nextScenetype)
{
	// 外部フック（遷移、その他演出など）があれば通知する
	if (this->transitionCallback)
	{
		this->transitionCallback(_nextScenetype);
	}

	// 遷移処理を開始する
	this->BeginTransition(_nextScenetype);
}

/**	@brief	演出や外部トリガー向けのコールバック設定を行う
 *	@param	std::function<void(SceneType)> _callback	出や外部トリガー向けのコールバック
 */
void SceneManager::SetTransitionCallback(std::function<void(SceneType)> _callback)
{
	this->transitionCallback = std::move(_callback);
}

/**	@brief	遷移開始処理を行う
 *	@param	SceneType _nextSceneType	次のシーンタイプ
 */
void SceneManager::BeginTransition(SceneType _nextSceneType)
{
	// 遷移シーンタイプを切り替える
	this->pendingSceneType = _nextSceneType;
	this->isTransitioning = true;

	this->isSceneInitialized = false;
}

/// @brief Factoryを使ってシーン生成・切り替えを行う
void SceneManager::CompleteTransition()
{
	// sceneFactoryが存在しない場合
	assert(this->sceneFactory && "シーン生成に失敗しました。SceneFactoryがnullでした。");

	// シーンを生成する
	auto newScene = this->sceneFactory->Create(this->pendingSceneType);

	// newSceneが存在しない場合
	assert(newScene && "シーン遷移に失敗しました。newSceneがnullptrでした。");

	if (newScene)
	{
		// シーンを切り替える
		this->currentScene = std::move(newScene);
		this->currentSceneType = this->pendingSceneType;
	}
	this->isTransitioning = false;
}