/**	@file	GameScene.cpp
*	@date	2025/07/04
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Scenes/GameScene.h"
#include"Include/Framework/Core/ResourceHub.h"
#include"Include/Framework/Core/SystemLocator.h"

//#include"Include/Framework/Entities/SpriteRenderer.h"
#include"Include/Framework/Entities/MeshRenderer.h"
#include"Include/Framework/Entities/SkinnedMeshRenderer.h"
#include"Include/Framework/Entities/Camera2D.h"
#include"Include/Framework/Entities/Camera3D.h"
#include"Include/Framework/Entities/MeshComponent.h"
#include"Include/Framework/Entities/TimeScaleComponent.h"
#include"Include/Framework/Entities/Rigidbody3D.h"
#include"Include/Framework/Entities/Collider3DComponent.h"
#include"Include/Framework/Entities/ColliderDebugRenderer.h"
#include"Include/Framework/Entities/AnimationComponent.h"

#include"Include/Game/Entities/FollowCamera.h"
#include"Include/Game/Entities/DebugFreeMoveComponent.h"
#include"Include/Game/Entities/CharacterController.h"
#include"Include/Game/Entities/CameraLookComponent.h"
#include"Include/Game/Entities/DodgeComponent.h"
#include"Include/Game/Entities/MoveComponent.h"

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

//-----------------------------------------------------------------------------
// GameScene Class
//-----------------------------------------------------------------------------

/**	@brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
GameScene::GameScene(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
GameScene::~GameScene() {}

/// @brief	オブジェクトの生成、登録等を行う
void GameScene::SetupObjects()
{
	std::cout << "[GameScene] シーン名" << "GameScene" << std::endl;

	//--------------------------------------------------------------
	// リソースマネージャの取得
	//--------------------------------------------------------------
	auto& spriteManager = ResourceHub::Get<SpriteManager>();
	auto& meshManager = ResourceHub::Get<MeshManager>();
	auto& modelManager = ResourceHub::Get<ModelManager>();
	auto& animationClipManager = ResourceHub::Get<AnimationClipManager>();

	//--------------------------------------------------------------
	// モデルデータの取得
	//--------------------------------------------------------------
	modelManager.Register("Player");
	auto modelData = modelManager.Get("Player");

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
	Graphics::Animation::StateTable<CharacterController::PlayerAnimState>* playerStateTable = new Graphics::Animation::StateTable<CharacterController::PlayerAnimState>();

	auto clip = animationClipManager.Get("Punch");
	enemyStateTable->Set(TestEnemy::EnemyAnimState::Idle, { clip,1.0f, true, 0.2f });

	playerStateTable->Set(CharacterController::PlayerAnimState::Punching, { clip,1.0f, false, 0.2f });

	clip = animationClipManager.Get("Idle");
	playerStateTable->Set(CharacterController::PlayerAnimState::Idle, { clip,1.0f, true, 0.2f });

	clip = animationClipManager.Get("Dodge");
	playerStateTable->Set(CharacterController::PlayerAnimState::Dodging, { clip,1.0f, false, 0.2f });

	clip = animationClipManager.Get("Jump");
	playerStateTable->Set(CharacterController::PlayerAnimState::Jumping, { clip,1.0f, false, 0.2f });

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

	// プレイヤーと敵のオブジェクトを生成する
	auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	auto enemy = this->gameObjectManager.Instantiate("Enemy", GameTags::Tag::Enemy);

	// 時間制御グループに所属させる
	player->TimeScale()->SetGroupName("TestGroup");
	enemy->TimeScale()->SetGroupName("TestGroup");

	// プレイヤー
	player->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 0.0f));
	player->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));

	auto meshComponent = player->AddComponent<MeshComponent>();
	meshComponent->SetMesh(modelData->mesh);
	auto materialComponent = player->AddComponent<MaterialComponent>();
	materialComponent->SetMaterial(modelData->material);
	materialComponent->SetMaterial(modelData->material);
	auto animationComponent = player->AddComponent<AnimationComponent>();
	animationComponent->SetSkeletonCache(modelData->GetSkeletonCache());

	// アニメーターの設定
	std::unique_ptr<Animator<CharacterController::PlayerAnimState>> playerAnimator = std::make_unique<Animator<CharacterController::PlayerAnimState>>();
	playerAnimator->Initialize(
		modelData->GetSkeletonCache(),
		playerStateTable,
		CharacterController::PlayerAnimState::Idle);
	animationComponent->SetAnimator(std::move(playerAnimator));

	player->AddComponent<SkinnedMeshRenderer>();
	player->AddComponent<TestDodge>();

	// キャラクターコントローラーを追加する
	auto characterController = player->AddComponent<CharacterController>();
	player->AddComponent<AttackComponent>();
	player->AddComponent<DodgeComponent>();
	player->AddComponent<MoveComponent>();

	auto collider = player->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	collider->SetCapsule(10.0f, 16.0f);
	collider->SetisTrigger(true);

	collider = player->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	collider->SetCapsule(10.0f, 16.0f);

	auto rigidbody = player->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	rigidbody->SetMotionTypeKinematic();
	rigidbody->SetUseGravity(true);

	auto colliderDebugRenderer = player->AddComponent<ColliderDebugRenderer>();

	// カメラピボットオブジェクトを生成する
	auto cameraPivot = gameObjectManager.Instantiate("CameraPivot");
	meshComponent = cameraPivot->AddComponent<MeshComponent>();
	meshComponent->SetMesh(meshManager.Get("Box"));

	//// カメラ注視コンポーネントを追加する
	//auto cameraLook = cameraPivot->AddComponent<CameraLookComponent>();
	//cameraLook->SetTarget(player->transform);
	//cameraLook->SetOffset(DX::Vector3(12.0f, 6.0f, -10.0f)); // 少し右上にオフセット

	//// カメラ追従コンポーネントを追加する
	//auto followCamera = camera3D->AddComponent<FollowCamera>();
	//followCamera->SetTarget(player->transform);
	//followCamera->SetPivot(cameraPivot->transform);
	//followCamera->SetSmoothSpeed(5.0f);

	//敵
	enemy->transform->SetLocalPosition(DX::Vector3(10.0f, -10.0f, 0.0f));
	enemy->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));
	meshComponent = enemy->AddComponent<MeshComponent>();
	meshComponent->SetMesh(modelData->mesh);
	materialComponent = enemy->AddComponent<MaterialComponent>();
	materialComponent->SetMaterial(modelData->material);
	animationComponent = enemy->AddComponent<AnimationComponent>();
	animationComponent->SetSkeletonCache(modelData->GetSkeletonCache());

	// アニメーターの設定
	std::unique_ptr<Animator<TestEnemy::EnemyAnimState>> enemyAnimator = std::make_unique<Animator<TestEnemy::EnemyAnimState>>();
	enemyAnimator->Initialize(
		modelData->GetSkeletonCache(),
		enemyStateTable,
		TestEnemy::EnemyAnimState::Idle);
	animationComponent->SetAnimator(std::move(enemyAnimator));

	enemy->AddComponent<SkinnedMeshRenderer>();

	// 敵のコライダー・リジッドボディ
	auto enemyTriggerCollider = enemy->AddComponent<Framework::Physics::Collider3DComponent>();
	enemyTriggerCollider->SetShape(Framework::Physics::ColliderShapeType::Box);
	enemyTriggerCollider->SetCenterOffset(DX::Vector3(0.0f, 0.0f, -5.0f));
	enemyTriggerCollider->SetisTrigger(true);
	enemyTriggerCollider->SetBoxHalfExtent(DX::Vector3(20.0f, 20.0f, 20.0f));

	collider = enemy->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	collider->SetCapsule(10.0f, 16.0f);
	colliderDebugRenderer = enemy->AddComponent<ColliderDebugRenderer>();

	rigidbody = enemy->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetMotionTypeKinematic();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	rigidbody->SetUseGravity(true);

	enemy->AddComponent<TestEnemy>();
	enemy->AddComponent<AttackComponent>();

	// 球体オブジェクト
	auto sphere = this->gameObjectManager.Instantiate("Sphere");
	sphere->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 10.0f));
	sphere->transform->SetLocalScale(DX::Vector3(2.0f, 2.0f, 2.0f));
	meshComponent = sphere->AddComponent<MeshComponent>();
	meshComponent->SetMesh(meshManager.Get("Sphere"));
	sphere->AddComponent<MeshRenderer>();

	collider = sphere->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Sphere);

	collider = sphere->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Sphere);
	collider->SetCenterOffset(DX::Vector3(0.0f, 0.0f, 5.0f));
	collider->SetisTrigger(true);

	rigidbody = sphere->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	rigidbody->SetMotionTypeKinematic();
	rigidbody->SetUseGravity(true);

	sphere->AddComponent<ColliderDebugRenderer>();

	//// トリガー用球体オブジェクト
	//auto sphereTrigger = this->gameObjectManager.Instantiate("SphereTrigger");
	//sphereTrigger->SetParent(player);
	//collider = sphereTrigger->AddComponent<Framework::Physics::Collider3DComponent>();
	//collider->SetShape(Framework::Physics::ColliderShapeType::Sphere);
	//collider->SetCenterOffset(DX::Vector3(0.0f, 0.0f, 5.0f));
	//collider->SetisTrigger(true);

	////rigidbody = sphereTrigger->AddComponent<Framework::Physics::Rigidbody3D>();
	////rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	////sphereTrigger->AddComponent<ColliderDebugRenderer>();

	//rigidbody = sphere->AddComponent<Framework::Physics::Rigidbody3D>();
	//rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	//rigidbody->SetMotionTypeKinematic();
	//rigidbody->SetUseGravity(true);
	//collider = sphere->AddComponent<Framework::Physics::Collider3DComponent>();
	//collider->SetShape(Framework::Physics::ColliderShapeType::Sphere);
	//sphere->AddComponent<ColliderDebugRenderer>();


	// 平面オブジェクト
	auto ground = this->gameObjectManager.Instantiate("obj_4");
	ground->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	ground->transform->SetLocalScale(DX::Vector3(500.0f, 1.0f, 500.0f));
	meshComponent = ground->AddComponent<MeshComponent>();
	meshComponent->SetMesh(meshManager.Get("Plane"));
	ground->AddComponent<MeshRenderer>();
	collider = ground->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Box);
	rigidbody = ground->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetMotionTypeStatic();
}