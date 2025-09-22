/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Scenes/TestScene.h"
#include"Framework/Entities/TestComponent.h"
#include"Framework/Entities/TestRenderer.h"

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
	std::cout << obj_1->GetName() << " : " << std::to_string(obj_1->transform->GetWorldPosition().x) << std::endl;
	obj_1->AddComponent<TestRenderer>();
	//std::cout << obj_1->GetName() << " : " <<
	//	std::to_string(obj_1->transform->GetWorldPosition().x) <<
	//	std::to_string(obj_1->transform->GetWorldPosition().y) <<
	//	std::to_string(obj_1->transform->GetWorldPosition().z) <<
	//	std::endl;

	this->gameObjectManager.Instantiate("obj_2");
	auto obj_2 = this->gameObjectManager.Instantiate("obj_3");
}