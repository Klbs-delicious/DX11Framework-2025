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
#include"Include/Framework/Graphics/ModelManager.h"
#include"Include/Framework/Graphics/AnimationClipManager.h"

// 描画
#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/SpriteComponent.h"
#include"Include/Framework/Entities/MeshComponent.h"	
#include"Include/Framework/Entities/MeshRenderer.h"
#include"Include/Framework/Entities/MaterialComponent.h"
#include"Include/Framework/Entities/AnimationComponent.h"
#include"Include/Framework/Entities/SkinnedMeshRenderer.h"

// カメラ
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"	

// ゲームコンポーネント
#include"Include/Game/Entities/FogComponent.h"
#include"Include/Game/Entities/CharacterController.h"

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
	auto& modelManager = ResourceHub::Get<ModelManager>();
	auto& animationClipManager = ResourceHub::Get<AnimationClipManager>();

	modelManager.Register("Player");
	animationClipManager.Register("Idle");

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

	// 疑似フォグ用の板ポリ
	auto fogPlane = this->gameObjectManager.Instantiate("FogPlane");
	fogPlane->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 200.0f));
	fogPlane->transform->SetLocalScale(DX::Vector3(500.0f, 1.0f, 500.0f));
	DX::Quaternion q =
		DX::Quaternion::CreateFromAxisAngle(
			DX::Vector3(1.0f, 0.0f, 0.0f),   // X軸
			DX::ToRadians(-90.0f)            // 角度
		);
	fogPlane->transform->SetLocalRotation(q);
	auto meshComp = fogPlane->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	fogPlane->AddComponent<MeshRenderer>();
	fogPlane->AddComponent<FogComponent>();

	//// モデル、アニメーションの設定
	//auto modelData = modelManager.Get("Player");
	//auto clip = animationClipManager.Get("Idle");

	//auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	//player->transform->SetLocalPosition(DX::Vector3(0.0f, 0.0f, 0.0f));
	//player->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));

	//meshComp = player->AddComponent<MeshComponent>();
	//meshComp->SetMesh(modelData->mesh);
	//auto materialComp = player->AddComponent<MaterialComponent>();
	//materialComp->SetMaterial(modelData->material);
	//auto animComp = player->AddComponent<AnimationComponent>();
	//animComp->SetSkeletonCache(modelData->GetSkeletonCache());
	//animComp->SetAnimationClip(clip);
	//animComp->SetLoop(true);
	//animComp->Play();
	//player->AddComponent<SkinnedMeshRenderer>();
	//// キャラクターコントローラーを追加する
	//auto charaController = player->AddComponent<CharacterController>();
	//auto coll3D = player->AddComponent<Framework::Physics::Collider3DComponent>();
	//coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	//auto rigidbody3D = player->AddComponent<Framework::Physics::Rigidbody3D>();

	// 床
	auto plane = this->gameObjectManager.Instantiate("Plane");
	plane->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	plane->transform->SetLocalScale(DX::Vector3(300.0f, 1.0f, 300.0f));
	meshComp = plane->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	plane->AddComponent<MeshRenderer>();
	plane->AddComponent<FogComponent>();
		
	// タイトルロゴ
	auto titleLogo = this->gameObjectManager.Instantiate("TitleLogo");
	titleLogo->transform->SetLocalScale(DX::Vector3(1980.0f * 0.6, 1080.0f * 0.6, 0.0f));
	titleLogo->AddComponent<SpriteRenderer>();
	titleLogo->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("TitleLogo"));

	// UI
	auto startUI = this->gameObjectManager.Instantiate("UI_Start");
	startUI->transform->SetLocalPosition(DX::Vector3(375.0f, 200.0f, 0.0f));
	startUI->transform->SetLocalScale(DX::Vector3(150.0f, 100.0f, 0.0f));
	auto renderComp = startUI->AddComponent<SpriteRenderer>();
	renderComp->SetColor(DX::Color(0.0f, 0.78f, 0.90f, 1.0f));
	startUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_Start"));

	auto howToPlayUI = this->gameObjectManager.Instantiate("UI_HowToPlay");
	howToPlayUI->transform->SetLocalPosition(DX::Vector3(300.0f, 250.0f, 0.0f));
	howToPlayUI->transform->SetLocalScale(DX::Vector3(300.0f, 100.0f, 0.0f));
	renderComp=howToPlayUI->AddComponent<SpriteRenderer>();
	renderComp->SetColor(DX::Color(0.0f, 0.78f, 0.90f, 1.0f));

	howToPlayUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_HowToPlay"));

	auto exitUI = this->gameObjectManager.Instantiate("UI_Exit");
	exitUI->transform->SetLocalPosition(DX::Vector3(375.0f, 300.0f, 0.0f));
	exitUI->transform->SetLocalScale(DX::Vector3(150.0f, 100.0f, 0.0f));
	renderComp = exitUI->AddComponent<SpriteRenderer>();
	renderComp->SetColor(DX::Color(0.0f, 0.78f, 0.90f, 1.0f));

	exitUI->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("UI_Exit"));
}