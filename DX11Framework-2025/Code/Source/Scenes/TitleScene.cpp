/**	@file	TitleScene.cpp
*	@date	2025/07/04
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/TitleScene.h"

// リソース
#include"Include/Framework/Core/ResourceHub.h"
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Graphics/MeshManager.h"
#include"Include/Framework/Shaders/ShaderManager.h"
#include"Include/Framework/Graphics/MaterialManager.h"

// 描画
#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/SpriteComponent.h"
#include"Include/Framework/Entities/MeshComponent.h"	
#include"Include/Framework/Entities/MeshRenderer.h"

// カメラ
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"	

#include<iostream>

//-----------------------------------------------------------------------------
// TitleScene Class
//-----------------------------------------------------------------------------

/**	@brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
TitleScene::TitleScene(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
TitleScene::~TitleScene() {}

/// @brief	オブジェクトの生成、登録等を行う
void TitleScene::SetupObjects()
{
	//--------------------------------------------------------------
	// リソースの取得
	//--------------------------------------------------------------
	auto& spriteManager = ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();

	//--------------------------------------------------------------
	// カメラの生成
	//--------------------------------------------------------------

	// 3Dカメラオブジェクト
	auto camera3D = this->gameObjectManager.Instantiate("Camera3D", GameTags::Tag::Camera);
	camera3D->AddComponent<Camera3D>();

	// 2Dカメラオブジェクト
	auto camera2D = this->gameObjectManager.Instantiate("Camera2D", GameTags::Tag::Camera);
	auto camera2DComp = camera2D->AddComponent<Camera2D>();
	camera2DComp->SetOriginMode(Camera2D::OriginMode::Center);

	//--------------------------------------------------------------
	// オブジェクトの生成
	//--------------------------------------------------------------

	// 床
	auto plane = this->gameObjectManager.Instantiate("Plane");
	plane->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	plane->transform->SetLocalScale(DX::Vector3(300.0f, 1.0f, 300.0f));
	auto meshComp = plane->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	plane->AddComponent<MeshRenderer>();
		
	// タイトルロゴ
	auto titleLogo = this->gameObjectManager.Instantiate("TitleLogo");
	titleLogo->transform->SetLocalScale(DX::Vector3(1980.0f * 0.6, 1080.0f * 0.6, 0.0f));
	titleLogo->AddComponent<SpriteRenderer>();
	titleLogo->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("TitleLogo"));

	// UI
	auto startUI = this->gameObjectManager.Instantiate("UI_Start");
	startUI->transform->SetLocalPosition(DX::Vector3(375.0f, 200.0f, 0.0f));
	startUI->transform->SetLocalScale(DX::Vector3(150.0f, 100.0f, 0.0f));
	startUI->AddComponent<SpriteRenderer>();
	startUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_Start"));

	auto howToPlayUI = this->gameObjectManager.Instantiate("UI_HowToPlay");
	howToPlayUI->transform->SetLocalPosition(DX::Vector3(300.0f, 250.0f, 0.0f));
	howToPlayUI->transform->SetLocalScale(DX::Vector3(300.0f, 100.0f, 0.0f));
	howToPlayUI->AddComponent<SpriteRenderer>();
	howToPlayUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_HowToPlay"));

	auto exitUI = this->gameObjectManager.Instantiate("UI_Exit");
	exitUI->transform->SetLocalPosition(DX::Vector3(375.0f, 300.0f, 0.0f));
	exitUI->transform->SetLocalScale(DX::Vector3(150.0f, 100.0f, 0.0f));
	exitUI->AddComponent<SpriteRenderer>();
	exitUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_Exit"));
}