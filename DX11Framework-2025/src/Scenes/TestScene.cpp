/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Scenes/TestScene.h"
#include"Framework/Entities/TestComponent.h"
#include"Framework/Entities/TestRenderer.h"
#include"Framework/Entities/Camera2D.h"
#include"Tests/TestMoveComponent.h"

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
	obj_1->transform->SetLocalPosition(DX::Vector3(320.0f, 240.0f, 0.0f));

	obj_1->AddComponent<Camera2D>();
	obj_1->AddComponent<TestRenderer>();
	obj_1->AddComponent<TestMoveComponent>();

	auto obj_2 =this->gameObjectManager.Instantiate("obj_2");
	obj_2->transform->SetLocalPosition(DX::Vector3(100.0f, 100.0f, 0.0f));

	obj_2->AddComponent<Camera2D>();
	obj_2->AddComponent<TestRenderer>();
}