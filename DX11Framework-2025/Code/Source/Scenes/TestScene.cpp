/**	@file	TestScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/TestScene.h"
#include"Include/Framework/Core/ResourceHub.h"
#include"Include/Framework/Core/SystemLocator.h"

//#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/MeshRenderer.h"
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"
#include"Include/Framework/Entities/MeshComponent.h"
#include"Include/Framework/Entities/TimeScaleComponent.h"
#include"Include/Framework/Entities/TimeScaleGroup.h"
#include"Include/Framework/Entities/Rigidbody3D.h"
#include"Include/Framework/Entities/Collider3DComponent.h"
#include"Include/Framework/Entities/ColliderDebugRenderer.h"

#include"Include/Game/Entities/FollowCamera.h"
#include"Include/Game/Entities/DebugFreeMoveComponent.h"
#include"Include/Game/Entities/CharacterController.h"
#include"Include/Game/Entities/CameraLookComponent.h"

//#include"Include/Framework/Graphics/Mesh.h"
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Graphics/MeshManager.h"
#include"Include/Framework/Graphics/TextureFactory.h"
//#include"Include/Framework/Graphics/ModelImporter.h"
//#include"Include/Framework/Shaders/ShaderManager.h"

#include"Include/Tests/TestMoveComponent.h"
#include"Include/Tests/TimeScaleTestComponent.h"
#include"Include/Tests/FreeMoveTestComponent.h"
//#include"Include/Framework/Entities/TestRenderer.h"

#include<iostream>
#include<array>

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
	std::cout << "[TestScene] シーン名" << "TestScene" << std::endl;

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
	camera3D->transform->SetLocalPosition({ 0.0f, 20.0f, -10.0f });
	camera3D->transform->SetLocalRotation(DX::Quaternion::CreateFromYawPitchRoll(DX::ToRadians(30.0f), DX::ToRadians(30.0f), 0.0f));
	camera3D->AddComponent<Camera3D>();
	auto debugMove = camera3D->AddComponent<DebugFreeMoveComponent>();
	debugMove->SetSpeed(50.0f);

	//camera3D->AddComponent<TestMoveComponent>();

	//// 2Dカメラオブジェクト
	//auto camera2D = this->gameObjectManager.Instantiate("Camera2D", GameTags::Tag::Camera);
	//camera2D->AddComponent<Camera2D>();

	//--------------------------------------------------------------
	// オブジェクトの生成
	//--------------------------------------------------------------

	// 時間制御グループオブジェクトを生成する
	auto timeScaleGroup = this->gameObjectManager.Instantiate("TimeScaleGroup");
	auto timeGroup = timeScaleGroup->AddComponent<TimeScaleGroup>();

	//// スプライトオブジェクト
	//auto obj_1 = this->gameObjectManager.Instantiate("obj_1");
	//std::cout << obj_1->GetName() << " : " << std::to_string(obj_1->transform->GetWorldPosition().x) << std::endl;
	//obj_1->transform->SetLocalPosition(DX::Vector3(320.0f, 240.0f, 0.0f));
	//obj_1->transform->SetLocalScale(DX::Vector3(150.0f, 150.0f, 0.0f));
	//obj_1->AddComponent<SpriteRenderer>();
	//obj_1->AddComponent<TestMoveComponent>();
	//obj_1->GetComponent<SpriteComponent>()->SetSprite(spriteManager.Get("Eidan"));

	//// 球体オブジェクト
	//auto obj_2 = this->gameObjectManager.Instantiate("obj_2");
	//obj_2->transform->SetLocalPosition(DX::Vector3(0.0f, 0.0f, 10.0f));
	//obj_2->transform->SetLocalScale(DX::Vector3(2.0f, 2.0f, 2.0f));
	//auto meshComp = obj_2->AddComponent<MeshComponent>();
	//meshComp->SetMesh(meshManager.Get("Sphere"));
	//obj_2->AddComponent<MeshRenderer>();
	//obj_2->OnDestroy();

	// 立方体オブジェクト（プレイヤー）
	auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	player->transform->SetLocalPosition(DX::Vector3(5.0f, 0.0f, 5.0f));
	auto meshComp = player->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Box"));
	auto matComp = player->AddComponent<MaterialComponent>();
	matComp->SetTexture(spriteManager.Get("Eidan"));
	player->AddComponent<MeshRenderer>();
	auto charaController = player->AddComponent<CharacterController>();
	auto coll3D = player->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	coll3D->BuildShape();
	auto rigidbody3D = player->AddComponent<Framework::Physics::Rigidbody3D>();
	auto testComp = player->AddComponent<TimeScaleTestComponent>();
	testComp->SetTimeScaleGroup(timeGroup);
	//charaController->SetTurnSpeed(10.0f);
	timeGroup->AddGroup("PlayerGroup", player->GetComponent<TimeScaleComponent>());

	// 立方体オブジェクト（親子テスト）
	auto child = this->gameObjectManager.Instantiate("Child");
	child->transform->SetLocalPosition(DX::Vector3(0.0f, 0.0f, 0.0f));
	child->SetParent(player);
	meshComp = child->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Box"));
	child->AddComponent<MeshRenderer>();
	child->AddComponent<FreeMoveTestComponent>();

	// カメラピボットオブジェクトを生成する
	auto pivotObj = gameObjectManager.Instantiate("CameraPivot");
	//meshComp = pivotObj->AddComponent<MeshComponent>();
	//meshComp->SetMesh(meshManager.Get("Box"));
	//pivotObj->AddComponent<MeshRenderer>();
	
	// カメラ注視コンポーネントを追加する
	auto cameraLook = pivotObj->AddComponent<CameraLookComponent>();
	cameraLook->SetTarget(player->transform);
	cameraLook->SetOffset(DX::Vector3(6.0f, 3.0f, -5.0f)); // 少し右上にオフセット

	//// カメラ追従コンポーネントを追加する
	//auto followCamera = camera3D->AddComponent<FollowCamera>();
	//followCamera->SetTarget(player->transform);
	//followCamera->SetPivot(pivotObj->transform);
	//followCamera->SetSmoothSpeed(5.0f);

	// 平面オブジェクト
	auto obj_4 = this->gameObjectManager.Instantiate("obj_4");
	obj_4->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	obj_4->transform->SetLocalScale(DX::Vector3(100.0f, 1.0f,100.0f));
	DX::Quaternion q =
		DX::Quaternion::CreateFromAxisAngle(
			DX::Vector3(1.0f, 0.0f, 0.0f),   // X軸
			DX::ToRadians(30.0f)             // 角度
		);

	obj_4->transform->SetLocalRotation(q);

	meshComp = obj_4->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	obj_4->AddComponent<MeshRenderer>();
	coll3D = obj_4->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	coll3D->BuildShape();
	rigidbody3D = obj_4->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody3D->SetMotionTypeStatic();
	rigidbody3D->SetObjectLayerStatic();
	// 大量オブジェクト生成テスト
	SpawnManyBoxes(10, 10, 10);

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

//-----------------------------------------------------------------------------
// 大量オブジェクト生成テスト（中心基準）
//-----------------------------------------------------------------------------
void TestScene::SpawnManyBoxes(const int _countX, const int _countZ, const float _spacing)
{
	auto device = SystemLocator::Get<D3D11System>().GetDevice();

	auto& spriteManager = ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();
	auto& gameObjectManager = this->gameObjectManager;

	auto timeScaleGroup = gameObjectManager.GetFindObjectByName("TimeScaleGroup");
	auto timeGroup = timeScaleGroup->GetComponent<TimeScaleGroup>();

	std::array<DX::Color, 3> colors = {
		DX::Color(1,0,0,1),
		DX::Color(0,1,0,1),
		DX::Color(0,0,1,1)
	};

	//==============================================================
	// 生成範囲の中心計算
	//==============================================================
	float totalWidth = (_countX - 1) * _spacing;
	float totalDepth = (_countZ - 1) * _spacing;

	DX::Vector3 center(totalWidth * 0.5f, 0.0f, totalDepth * 0.5f);

	int index = 0;

	for (int z = 0; z < _countZ; z++)
	{
		for (int x = 0; x < _countX; x++)
		{
			std::string name = "Box_" + std::to_string(x) + "_" + std::to_string(z);
			auto obj = gameObjectManager.Instantiate(name);

			//==============================================================
			// 中心からの相対座標に補正
			//==============================================================
			DX::Vector3 pos(
				x * _spacing,
				0.0f,
				z * _spacing
			);

			pos -= center; 

			obj->transform->SetLocalPosition(pos);
			obj->transform->SetLocalScale(DX::Vector3(2, 2, 2));

			auto matComp = obj->AddComponent<MaterialComponent>();
			auto meshComp = obj->AddComponent<MeshComponent>();
			meshComp->SetMesh(meshManager.Get("Sphere"));

			auto coll3D = obj->AddComponent<Framework::Physics::Collider3DComponent>();
			coll3D->SetShape(Framework::Physics::ColliderShapeType::Sphere);
			//coll3D->SetCenterOffset(DX::Vector3(2.0f, 0.0f, 0.0f));
			coll3D->BuildShape();

			auto rigidbody3D = obj->AddComponent<Framework::Physics::Rigidbody3D>();

			float randomScale = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;

			obj->AddComponent<MeshRenderer>();
			obj->AddComponent<ColliderDebugRenderer>();
			obj->AddComponent<FreeMoveTestComponent>();

			int groupId = index % 3 + 1;
			std::string groupName = "EnemyGroup_" + std::to_string(groupId);

			auto tex = TextureFactory::CreateSolidColorTexture(device, colors[groupId - 1]);
			TextureResource* raw = tex.release();
			matComp->SetTexture(raw);

			timeGroup->AddGroup(groupName, obj->GetComponent<TimeScaleComponent>());

			index++;
		}
	}
	std::cout << "[Test] Spawned " << (_countX * _countZ) << " boxes.\n";
}