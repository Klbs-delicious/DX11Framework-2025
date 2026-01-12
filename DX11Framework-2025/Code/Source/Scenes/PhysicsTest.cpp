/**	@file	PhysicsTest.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/PhysicsTest.h"
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
#include"Include/Tests/TestCollisionHandler.h"
//#include"Include/Framework/Entities/TestRenderer.h"

#include<iostream>
#include<array>

//-----------------------------------------------------------------------------
// PhysicsTest Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
PhysicsTest::PhysicsTest(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
PhysicsTest::~PhysicsTest() {}

/// @brief	オブジェクトの生成、登録等を行う
void PhysicsTest::SetupObjects()
{
	std::cout << "[PhysicsTest] シーン名" << "PhysicsTest" << std::endl;

	//--------------------------------------------------------------
	// リソースマネージャの取得
	//--------------------------------------------------------------
	auto& spriteManager = ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();

	//--------------------------------------------------------------
	// カメラの生成
	//--------------------------------------------------------------

	// 3Dカメラオブジェクト
	auto camera3D = this->gameObjectManager.Instantiate("Camera3D", GameTags::Tag::Camera);
	camera3D->transform->SetLocalPosition({ 0.0f, -10.0f, -10.0f });
	camera3D->transform->SetLocalRotation(DX::Quaternion::CreateFromYawPitchRoll(/*DX::ToRadians(30.0f)*/0.0f, DX::ToRadians(0.0f), 0.0f));
	camera3D->AddComponent<Camera3D>();
	camera3D->AddComponent<TimeScaleTestComponent>();
	//auto debugMove = camera3D->AddComponent<DebugFreeMoveComponent>();
	//debugMove->SetSpeed(10.0f);

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

	MeshComponent* meshComp;
	Framework::Physics::Collider3DComponent* coll3D;
	Framework::Physics::Rigidbody3D* rigidbody3D;

	// カプセルオブジェクト
	auto capsule = this->gameObjectManager.Instantiate("Capsule");
	capsule->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 10.0f));
	capsule->transform->SetLocalScale(DX::Vector3(2.0f, 2.0f, 2.0f));
	meshComp = capsule->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Capsule"));
	capsule->AddComponent<MeshRenderer>();

	coll3D = capsule->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	//coll3D->SetisTrigger(true);
	// Shape will be created on Initialize

	rigidbody3D = capsule->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody3D->SetUseGravity(true);
	//rigidbody3D->SetGravity(DX::Vector3(0.0f, 9.8f, 0.0f));
	rigidbody3D->SetMotionTypeKinematic();
	rigidbody3D->SetObjectLayer(Framework::Physics::PhysicsLayer::Player);
	capsule->AddComponent<ColliderDebugRenderer>();
	//capsule->AddComponent<TestMoveComponent>();
	capsule->AddComponent<CharacterController>();
	capsule->AddComponent<TestCollisionHandler>();

	// 球体オブジェクト
	auto sphere = this->gameObjectManager.Instantiate("Sphere");
	sphere->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 10.0f));
	sphere->transform->SetLocalScale(DX::Vector3(2.0f, 2.0f, 2.0f));
	meshComp = sphere->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Sphere"));
	sphere->AddComponent<MeshRenderer>();

	// トリガー用球体オブジェクト
	auto sphereTrigger = this->gameObjectManager.Instantiate("SphereTrigger");
	sphereTrigger->SetParent(sphere);
	coll3D = sphereTrigger->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Sphere);
	coll3D->SetCenterOffset(DX::Vector3(0.0f, 0.0f, 5.0f));
	coll3D->SetisTrigger(true);

	//rigidbody3D = sphereTrigger->AddComponent<Framework::Physics::Rigidbody3D>();
	//rigidbody3D->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	//sphereTrigger->AddComponent<ColliderDebugRenderer>();

	rigidbody3D = sphere->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody3D->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	rigidbody3D->SetMotionTypeKinematic();
	rigidbody3D->SetUseGravity(true);
	coll3D = sphere->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Sphere);
	sphere->AddComponent<ColliderDebugRenderer>();


	// 平面オブジェクト
	auto groundObj = this->gameObjectManager.Instantiate("Ground");
	groundObj->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	groundObj->transform->SetLocalScale(DX::Vector3(100.0f, 1.0f, 100.0f));
	//DX::Quaternion q =
	//	DX::Quaternion::CreateFromAxisAngle(
	//		DX::Vector3(1.0f, 0.0f, 0.0f),   // X軸
	//		DX::ToRadians(30.0f)             // 角度
	//	);
	//groundObj->transform->SetLocalRotation(q);

	meshComp = groundObj->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Box"));
	groundObj->AddComponent<MeshRenderer>();
	coll3D = groundObj->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	rigidbody3D = groundObj->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody3D->SetMotionTypeStatic();
	rigidbody3D->SetObjectLayer(Framework::Physics::PhysicsLayer::Ground);
	groundObj->AddComponent<ColliderDebugRenderer>();
}