/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Scenes/TestScene.h"
#include"Framework/Scenes/TestComponent.h"

#include<iostream>

//-----------------------------------------------------------------------------
// TestScene Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
TestScene::TestScene(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
TestScene::~TestScene() {}

/// @brief	オブジェクトの生成、登録等を行う
void TestScene::SetupObjects()
{
	std::cout << "シーン名" << "TestScene" << std::endl;

	// オブジェクトを生成する
	auto obj_1 = this->gameObjectManager.Instantiate("obj_1");
	obj_1->AddComponent<HogeComponent>();
	this->gameObjectManager.Instantiate("obj_2");
	auto obj_2 = this->gameObjectManager.Instantiate("obj_3");
	obj_2->AddComponent<HogeComponent>();
}