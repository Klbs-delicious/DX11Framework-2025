/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/TestScene.h"
#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Tests/TestMoveComponent.h"
#include"Include/Framework/Core/ResourceHub.h"
#include"Include/Framework/Graphics/SpriteManager.h"

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

	// 未設定の場合はデフォルト画像を設定する
	auto& spriteManager= ResourceHub::Get<SpriteManager>();

	// カメラオブジェクト
	auto cameraObj = this->gameObjectManager.Instantiate("CameraObject", GameTags::Tag::Camera);
	cameraObj->AddComponent<Camera2D>();

	// オブジェクトを生成する
	auto obj_1 = this->gameObjectManager.Instantiate("obj_1");
	std::cout << obj_1->GetName() << " : " << std::to_string(obj_1->transform->GetWorldPosition().x) << std::endl;
	obj_1->transform->SetLocalPosition(DX::Vector3(320.0f, 240.0f, 0.0f));
	obj_1->transform->SetLocalScale(DX::Vector3(150.0f, 150.0f, 0.0f));

	obj_1->AddComponent<SpriteRenderer>();
	obj_1->AddComponent<TestMoveComponent>();
	obj_1->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("Eidan"));

	auto obj_2 =this->gameObjectManager.Instantiate("obj_2");
	obj_2->transform->SetLocalPosition(DX::Vector3(100.0f, 100.0f, 0.0f));
	obj_2->transform->SetLocalScale(DX::Vector3(100.0f, 100.0f, 0.0f));

	obj_2->AddComponent<Camera2D>();
	obj_2->AddComponent<SpriteRenderer>();
}