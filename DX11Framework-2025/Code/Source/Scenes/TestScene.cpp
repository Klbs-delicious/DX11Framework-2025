/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/TestScene.h"
#include"Include/Framework/Core/ResourceHub.h"
//#include"Include/Framework/Core/SystemLocator.h"

//#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/MeshRenderer.h"
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"
#include"Include/Framework/Entities/MeshComponent.h"

#include"Include/Game/Entities/CharacterController.h"

//#include"Include/Framework/Graphics/Mesh.h"
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Graphics/MeshManager.h"
//#include"Include/Framework/Graphics/ModelImporter.h"
//#include"Include/Framework/Shaders/ShaderManager.h"

#include"Include/Tests/TestMoveComponent.h"
//#include"Include/Framework/Entities/TestRenderer.h"

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

	//--------------------------------------------------------------
	// リソースマネージャの取得
	//--------------------------------------------------------------
	auto& spriteManager= ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();

	//--------------------------------------------------------------
	// カメラの生成
	//--------------------------------------------------------------

	// 3Dカメラオブジェクト
	auto camera3D = this->gameObjectManager.Instantiate("Camera3D", GameTags::Tag::Camera);
	camera3D->transform->SetLocalPosition({ 0.0f, 10.0f, -10.0f });
	camera3D->transform->SetLocalRotation(DX::Quaternion::CreateFromYawPitchRoll(0.0f, DX::ToRadians(45.0f), 0.0f));
	camera3D->AddComponent<Camera3D>();

	//camera3D->AddComponent<TestMoveComponent>();

	// 2Dカメラオブジェクト
	auto camera2D = this->gameObjectManager.Instantiate("Camera2D", GameTags::Tag::Camera);
	camera2D->AddComponent<Camera2D>();

	//--------------------------------------------------------------
	// オブジェクトの生成
	//--------------------------------------------------------------

	//// スプライトオブジェクト
	//auto obj_1 = this->gameObjectManager.Instantiate("obj_1");
	//std::cout << obj_1->GetName() << " : " << std::to_string(obj_1->transform->GetWorldPosition().x) << std::endl;
	//obj_1->transform->SetLocalPosition(DX::Vector3(320.0f, 240.0f, 0.0f));
	//obj_1->transform->SetLocalScale(DX::Vector3(150.0f, 150.0f, 0.0f));
	//obj_1->AddComponent<SpriteRenderer>();
	//obj_1->AddComponent<TestMoveComponent>();
	//obj_1->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("Eidan"));

	// 球体オブジェクト
	auto obj_2 = this->gameObjectManager.Instantiate("obj_2");
	obj_2->transform->SetLocalPosition(DX::Vector3(0.0f, 0.0f, 10.0f));
	obj_2->transform->SetLocalScale(DX::Vector3(2.0f, 2.0f, 2.0f));
	auto meshComp = obj_2->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Sphere"));
	obj_2->AddComponent<MeshRenderer>();

	// 立方体オブジェクト
	auto obj_3 = this->gameObjectManager.Instantiate("obj_3");
	obj_3->transform->SetLocalPosition(DX::Vector3(5.0f, 0.0f, 5.0f));
	meshComp = obj_3->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Box"));
	auto matComp = obj_3->AddComponent<MaterialComponent>();
	matComp->SetTexture(spriteManager.Get("Eidan"));
	obj_3->AddComponent<MeshRenderer>();
	obj_3->AddComponent<CharacterController>();


	// 平面オブジェクト
	auto obj_4 = this->gameObjectManager.Instantiate("obj_4");
	obj_4->transform->SetLocalPosition(DX::Vector3(0.0f, -5.0f, 0.0f));
	obj_4->transform->SetLocalScale(DX::Vector3(20.0f, 1.0f, 20.0f));
	meshComp = obj_4->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	obj_4->AddComponent<MeshRenderer>();

	//// カプセルオブジェクト
	//auto obj_5 = this->gameObjectManager.Instantiate("obj_5");
	//obj_5->transform->SetLocalPosition(DX::Vector3(0.0f, 7.0f, 3.0f));
	//obj_5->transform->SetLocalScale(DX::Vector3(5.0f, 5.0f, 5.0f));
	//meshComp = obj_5->AddComponent<MeshComponent>();
	//meshComp->SetMesh(meshManager.Get("Capsule"));
	//obj_5->AddComponent<MeshRenderer>();

	/*
	// モデル読み込み（現状p_modelDataはメモリリークを起こす）
	Graphics::Import::ModelData* p_modelData = new Graphics::Import::ModelData();
	Graphics::Import::ModelImporter importer;
	bool isModelLoaded = importer.Load(
		"Assets/Models/Woman/woman.fbx",   // モデルパス
		"",                                // テクスチャフォルダ（空でOK）

		//"Assets/Models/akai/Akai.fbx",   // モデルパス
		//"",                                // テクスチャフォルダ（空でOK）

		//"Assets/Models/Man/man.fbx",   // モデルパス
		//"",                                // テクスチャフォルダ（空でOK）

		//"Assets/Models/jack2/jack2.pmx",   // モデルパス
		//"Assets/Models/jack2/tx",			// テクスチャフォルダ
		*p_modelData
	);

	if (!isModelLoaded)
	{
		std::cout << "[MeshRenderer] Failed to load model.\n";
		return;
	}

	//// メッシュの作成
	//Graphics::Mesh* p_mesh = new Graphics::Mesh();
	//p_mesh->Create(SystemLocator::Get<D3D11System>().GetDevice(),
	//	SystemLocator::Get<D3D11System>().GetContext(), 
	//	&ResourceHub::Get<ShaderManager>(),
	//	*p_modelData);

	//auto& meshes = ResourceHub::Get<MeshManager>();
	//meshes.Register("Woman", p_mesh);
	//auto mesh = meshes.Register("Woman");
	//mesh->Create(SystemLocator::Get<D3D11System>().GetDevice(),
	//	SystemLocator::Get<D3D11System>().GetContext(),
	//	&ResourceHub::Get<ShaderManager>(),
	//	*p_modelData);

	//// メッシュをセット
	//meshComp->SetMesh(meshes.Get("Woman"));
	//obj_2->AddComponent<MeshRenderer>();
	*/
}