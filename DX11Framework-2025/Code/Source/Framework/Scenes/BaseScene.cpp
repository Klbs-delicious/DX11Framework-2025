/**	@file	BaseScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Scenes/BaseScene.h"

#include"Include/Framework/Core/InputSystem.h"
#include"Include/Scenes/SceneManager.h"

#include<iostream>

//-----------------------------------------------------------------------------
// BaseScene Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
BaseScene::BaseScene(GameObjectManager& _gameObjectManager, RenderSystem& _renderSystem)
	: gameObjectManager(_gameObjectManager), renderSystem(_renderSystem), postProcessPipeline(_renderSystem.GetPostProcessPipeline()) {}

/// @brief	デストラクタ
BaseScene::~BaseScene() {}

/**	@brief		ゲームオブジェクトの初期化処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Initialize()
{
    // 初期化
    this->gameObjectManager.FlushInitialize();
}

/**	@brief		オブジェクトの更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Update(float _deltaTime) 
{
    // 未初期化オブジェクトの初期化
    this->gameObjectManager.FlushInitialize();

    // オブジェクトの一括更新
    this->gameObjectManager.UpdateAll(_deltaTime);
}

/**	@brief		ゲームオブジェクトの描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Draw()
{
    // シーン用RTを描画対象に設定
	this->renderSystem.SetRenderTarget(RenderTargetType::SceneRT);  
	this->gameObjectManager.RenderWithLayer(GameTags::Layer::Default);

    // ポストプロセスの実行
    if (this->postProcessPipeline)
    {
        this->postProcessPipeline->Execute(this->renderSystem);
	}
}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Finalize()
{
    // オブジェクトを削除
    this->gameObjectManager.Dispose();

    // ポストプロセスパスをクリア
    if (this->postProcessPipeline)
    {
        this->postProcessPipeline->ClearPasses();
    }
}