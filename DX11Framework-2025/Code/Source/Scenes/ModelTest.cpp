/**	@file	ModelTest.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/ModelTest.h"
#include"Include/Framework/Core/ResourceHub.h"
#include"Include/Framework/Core/SystemLocator.h"

//#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/MeshRenderer.h"
#include"Include/Framework/Entities/SkinnedMeshRenderer.h"
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"
#include"Include/Framework/Entities/MeshComponent.h"
#include"Include/Framework/Entities/TimeScaleComponent.h"
#include"Include/Framework/Entities/TimeScaleGroup.h"
#include"Include/Framework/Entities/Rigidbody3D.h"
#include"Include/Framework/Entities/Collider3DComponent.h"
#include"Include/Framework/Entities/ColliderDebugRenderer.h"
#include"Include/Framework/Entities/AnimationComponent.h"

#include"Include/Game/Entities/FollowCamera.h"
#include"Include/Game/Entities/DebugFreeMoveComponent.h"
#include"Include/Game/Entities/CharacterController.h"
#include"Include/Game/Entities/CameraLookComponent.h"

//#include"Include/Framework/Graphics/Mesh.h"
#include"Include/Framework/Graphics/SpriteManager.h"
#include"Include/Framework/Graphics/MeshManager.h"
#include"Include/Framework/Graphics/TextureFactory.h"
#include"Include/Framework/Graphics/ModelManager.h"
#include"Include/Framework/Graphics/AnimationClipManager.h"
#include"Include/Framework/Graphics/Animator.h"

#include"Include/Tests/TestMoveComponent.h"
#include"Include/Tests/TimeScaleTestComponent.h"
#include"Include/Tests/FreeMoveTestComponent.h"
#include"Include/Tests/TestCollisionHandler.h"
#include"Include/Tests/TestDodge.h"
#include"Include/Tests/TestEnemy.h"
//#include"Include/Framework/Entities/TestRenderer.h"

#include<iostream>
#include<array>

//-----------------------------------------------------------------------------
// ModelTest Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
ModelTest::ModelTest(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
ModelTest::~ModelTest() {}

/// @brief	オブジェクトの生成、登録等を行う
void ModelTest::SetupObjects()
{
	std::cout << "[ModelTest] シーン名" << "ModelTest" << std::endl;

	//--------------------------------------------------------------
	// リソースマネージャの取得
	//--------------------------------------------------------------
	auto& spriteManager = ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();
	auto& modelManager = ResourceHub::Get<ModelManager>();
	auto& animationClipManager = ResourceHub::Get<AnimationClipManager>();

	//--------------------------------------------------------------
	// アニメーションクリップの登録
	//--------------------------------------------------------------
	//animationClipManager.Register("Walk");
	//animationClipManager.Register("Run");
	animationClipManager.Register("Jump");
	animationClipManager.Register("HeadHit");
	animationClipManager.Register("Idle");
	animationClipManager.Register("Dodge");
	animationClipManager.Register("Punch");

	//---------------------------------------------------------------
	// 状態テーブルの設定（テスト的にシーン側で生成しっぱなしにする）
	//---------------------------------------------------------------
	Graphics::Animation::StateTable<TestEnemy::EnemyAnimState>* enemyStateTable = new Graphics::Animation::StateTable<TestEnemy::EnemyAnimState>();
	Graphics::Animation::StateTable<TestDodge::TestPlayerAnimState>* playerStateTable = new Graphics::Animation::StateTable<TestDodge::TestPlayerAnimState>();

	auto clip = animationClipManager.Get("Punch");
	enemyStateTable->Set(TestEnemy::EnemyAnimState::Idle, { clip,1.0f, true, 0.2f });

	clip = animationClipManager.Get("Idle");
	playerStateTable->Set(TestDodge::TestPlayerAnimState::Idle, { clip,1.0f, true, 0.2f });

	clip = animationClipManager.Get("Dodge");
	playerStateTable->Set(TestDodge::TestPlayerAnimState::Dodging, { clip,1.0f, true, 0.2f });

	clip = animationClipManager.Get("Jump");
	playerStateTable->Set(TestDodge::TestPlayerAnimState::Jumping, { clip,1.0f, true, 0.2f });

	//--------------------------------------------------------------
	// カメラの生成
	//--------------------------------------------------------------

	// 3Dカメラオブジェクト
	auto camera3D = this->gameObjectManager.Instantiate("Camera3D", GameTags::Tag::Camera);
	camera3D->transform->SetLocalPosition({ 0.0f, -10.0f, -10.0f });
	camera3D->transform->SetLocalRotation(DX::Quaternion::CreateFromYawPitchRoll(/*DX::ToRadians(30.0f)*/0.0f, DX::ToRadians(0.0f), 0.0f));
	camera3D->AddComponent<Camera3D>();
	auto debugMove = camera3D->AddComponent<DebugFreeMoveComponent>();
	debugMove->SetSpeed(10.0f);

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
	
	// モデルデータの取得
	modelManager.Register("Player");
	auto modelData = modelManager.Get("Player");

	auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	player->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 0.0f));
	player->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));

	auto timeScale = player->AddComponent<TimeScaleTestComponent>();
	timeScale->SetTimeScaleGroup(timeGroup);
	timeGroup->AddGroup("PlayerGroup", player->GetComponent<TimeScaleComponent>());

	auto meshComp = player->AddComponent<MeshComponent>();
	meshComp->SetMesh(modelData->mesh);
	auto materialComp = player->AddComponent<MaterialComponent>();
	materialComp->SetMaterial(modelData->material);
	auto animComp = player->AddComponent<AnimationComponent>();
	animComp->SetSkeletonCache(modelData->GetSkeletonCache());
	// アニメーターの設定
	std::unique_ptr<Animator<TestDodge::TestPlayerAnimState>> playerAnimator = std::make_unique<Animator<TestDodge::TestPlayerAnimState>>();
	playerAnimator->Initialize(
		modelData->GetSkeletonCache(),
		playerStateTable,
		TestDodge::TestPlayerAnimState::Idle);
	animComp->SetAnimator(std::move(playerAnimator));

	clip = animationClipManager.Get("Dodge");
	player->AddComponent<SkinnedMeshRenderer>();
	player->AddComponent<TestDodge>();

	// キャラクターコントローラーを追加する
	auto charaController = player->AddComponent<CharacterController>();
	auto coll3D = player->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	auto rigidbody3D = player->AddComponent<Framework::Physics::Rigidbody3D>();

	// カメラピボットオブジェクトを生成する
	auto pivotObj = gameObjectManager.Instantiate("CameraPivot");
	meshComp = pivotObj->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Box"));

	//// カメラ注視コンポーネントを追加する
	//auto cameraLook = pivotObj->AddComponent<CameraLookComponent>();
	//cameraLook->SetTarget(player->transform);
	//cameraLook->SetOffset(DX::Vector3(12.0f, 6.0f, -10.0f)); // 少し右上にオフセット

	//// カメラ追従コンポーネントを追加する
	//auto followCamera = camera3D->AddComponent<FollowCamera>();
	//followCamera->SetTarget(player->transform);
	//followCamera->SetPivot(pivotObj->transform);
	//followCamera->SetSmoothSpeed(5.0f);

	//敵
	auto enemy = this->gameObjectManager.Instantiate("Enemy", GameTags::Tag::Enemy);
	enemy->transform->SetLocalPosition(DX::Vector3(10.0f, -10.0f, 0.0f));
	enemy->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));
	meshComp = enemy->AddComponent<MeshComponent>();
	meshComp->SetMesh(modelData->mesh);
	materialComp = enemy->AddComponent<MaterialComponent>();
	materialComp->SetMaterial(modelData->material);
	animComp = enemy->AddComponent<AnimationComponent>();
	animComp->SetSkeletonCache(modelData->GetSkeletonCache());

	// アニメーターの設定
	std::unique_ptr<Animator<TestEnemy::EnemyAnimState>> enemyAnimator = std::make_unique<Animator<TestEnemy::EnemyAnimState>>();
	enemyAnimator->Initialize(
		modelData->GetSkeletonCache(),
		enemyStateTable,
		TestEnemy::EnemyAnimState::Idle);
	animComp->SetAnimator(std::move(enemyAnimator));

	enemy->AddComponent<SkinnedMeshRenderer>();

	// 敵のコライダー・リジッドボディ
	coll3D = enemy->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	rigidbody3D = enemy->AddComponent<Framework::Physics::Rigidbody3D>();

	enemy->AddComponent<TestEnemy>();

	// 平面オブジェクト
	auto obj_4 = this->gameObjectManager.Instantiate("obj_4");
	obj_4->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	obj_4->transform->SetLocalScale(DX::Vector3(500.0f, 1.0f, 500.0f));
	meshComp = obj_4->AddComponent<MeshComponent>();
	meshComp->SetMesh(meshManager.Get("Plane"));
	obj_4->AddComponent<MeshRenderer>();
	coll3D = obj_4->AddComponent<Framework::Physics::Collider3DComponent>();
	coll3D->SetShape(Framework::Physics::ColliderShapeType::Box);
	rigidbody3D = obj_4->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody3D->SetMotionTypeStatic();
}