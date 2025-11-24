/**	@file	SceneManager.cpp
*	@date	2025/07/04
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/SceneManager.h"

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
SceneManager::~SceneManager() { this->Dispose(); }

/**	@brief	シーンの更新を行う
 *	@param	float _deltaTime	デルタタイム
 */
void SceneManager::Update(float _deltaTime)
{    
	// 遷移フラグが立っていればシーン切り替えを行う
	if (this->isTransitioning)
	{
		this->CompleteTransition();

		// 旧シーンのUpdateをスキップ
		return; 
	}

	if (this->currentScene)
	{
		// 初期化が行われていなければ初期化処理を行う
		if (!this->isSceneInitialized)
		{
			this->currentScene->SetupObjects();
			this->currentScene->Initialize();
			this->isSceneInitialized = true;
		}
		this->currentScene->Update(_deltaTime);
	}
}

/// @brief	シーンの描画を行う
void SceneManager::Draw()
{
	if (this->isTransitioning || !this->isSceneInitialized) { return; }

	if (this->currentScene)
	{
		this->currentScene->Draw();
	}
}

//// @brief	保留中のシーン破棄を行う
void SceneManager::FlushPendingDestroys()
{
	if (this->currentScene)
	{
		this->currentScene->GetGameObjectManager().FlushDestroyQueue();
	}
}

/**	@brief	シーン遷移をリクエストする
 *	@param	SceneType _nextScenetype	次のシーンタイプ
 */
void SceneManager::RequestSceneChange(SceneType _nextScenetype)
{
	// 外部フック（遷移、その他演出など）があれば通知する
	if (this->onTransitionBegin)
	{
		this->onTransitionBegin(_nextScenetype);
	}
	else
	{
		// 演出がない場合は遷移を開始する
		NotifyTransitionReady(_nextScenetype);
	}
}

/**	@brief	遷移開始時の演出や外部トリガー向けのコールバック設定を行う
 *	@param	std::function<void(SceneType)> _callback	演出や外部トリガー向けのコールバック
 */
void SceneManager::SetTransitionCallback(std::function<void(SceneType)> _callback)
{
	this->onTransitionBegin = std::move(_callback);
}

/**	@brief	遷移開始処理を行うためのラッパー関数
 *	@param	SceneType _nextSceneType	次のシーンタイプ
 */
void SceneManager::NotifyTransitionReady(SceneType _nextSceneType)
{
	if (!this->isTransitioning)
	{
		this->BeginTransition(_nextSceneType);
	}
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
		// シーンの終了処理
		if (this->currentScene) { this->currentScene->Finalize(); }

		// シーンを切り替える
		this->currentScene = std::move(newScene);
		this->currentSceneType = this->pendingSceneType;
	}
	this->isTransitioning = false;

	// 遷移完了を通知する
	if (this->onTransitionEnd)
	{
		this->onTransitionEnd(this->currentSceneType);
	}
}

/// @brief 終了処理
void SceneManager::Dispose()
{
	if (this->currentScene) {
		this->currentScene->Finalize(); // 最後のシーンを明示的に終了
		this->currentScene.reset();
	}

	this->sceneFactory.reset();
	this->onTransitionBegin = nullptr;
	this->onTransitionEnd = nullptr;
}