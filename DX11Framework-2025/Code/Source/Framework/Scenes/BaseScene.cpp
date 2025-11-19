/**	@file	BaseScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Scenes/BaseScene.h"

#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Core/InputSystem.h"
#include"Include/Scenes/SceneManager.h"

#include<iostream>

//-----------------------------------------------------------------------------
// BaseScene Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
BaseScene::BaseScene(GameObjectManager& _gameObjectManager) :gameObjectManager(_gameObjectManager) {}

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

	// テスト的にメンバに持たずに直接取得する
	auto& input = SystemLocator::Get<InputSystem>();
	auto& scenemanager = SystemLocator::Get<SceneManager>();

	if (input.IsActionPressed("Space")) { std::cout << "Space：Press" << std::endl; }
	if (input.IsActionTriggered("Space")) { std::cout << "Space：Trigger" << std::endl; }

    //if (input.IsActionTriggered("SceneChangeTest")) { scenemanager.RequestSceneChange(SceneType::Test); }
    //if (input.IsActionTriggered("SceneChangeTitle")) { scenemanager.RequestSceneChange(SceneType::Title); }
}


/**	@brief 		オブジェクトの固定更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::FixedUpdate(float _deltaTime)
{
    // オブジェクトの一括固定更新
	this->gameObjectManager.FixedUpdateAll(_deltaTime);
}

/**	@brief		ゲームオブジェクトの描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Draw()
{
    this->gameObjectManager.RenderUIAll();  // 3Dコンポーネントの一括描画
    this->gameObjectManager.Render3DAll();
}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Finalize()
{
    // オブジェクトを削除
    this->gameObjectManager.Dispose();
}