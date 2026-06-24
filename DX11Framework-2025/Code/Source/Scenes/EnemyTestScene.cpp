/** @file EnemyTestScene.cpp
 *  @brief 敵のAI行動をテストするシーン
 *  @date 2026/06/23
 */
#include "Include/Scenes/EnemyTestScene.h"

#include "Include/Framework/Core/ResourceHub.h"
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/AnimationStateMachine.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Entities/Collider3DComponent.h"
#include "Include/Framework/Entities/ColliderDebugRenderer.h"
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Entities/MeshComponent.h"
#include "Include/Framework/Entities/MeshRenderer.h"
#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Entities/SkinnedMeshRenderer.h"
#include "Include/Framework/Graphics/AnimationClipManager.h"
#include "Include/Framework/Graphics/Animator.h"
#include "Include/Framework/Graphics/MeshManager.h"
#include "Include/Framework/Graphics/ModelManager.h"

#include "Include/Game/Entities/AttackComponent.h"
#include "Include/Game/Entities/CharacterController.h"
#include "Include/Game/Entities/DebugFreeMoveComponent.h"
#include "Include/Game/Entities/DodgeComponent.h"
#include "Include/Game/Entities/EnemyController.h"
#include "Include/Game/Entities/InputAdapterAI.h"
#include "Include/Game/Entities/MoveComponent.h"
#include "Include/Game/Entities/NormalBehavior.h"

#include <iostream>
#include <memory>

EnemyTestScene::EnemyTestScene(
	GameObjectManager& _gameObjectManager,
	RenderSystem& _renderSystem)
	: BaseScene(_gameObjectManager, _renderSystem)
{
}

EnemyTestScene::~EnemyTestScene() = default;

void EnemyTestScene::SetupObjects()
{
	std::cout << "[EnemyTestScene] Setup\n";

	// リソースマネージャの取得
	auto& meshManager = ResourceHub::Get<MeshManager>();
	auto& modelManager = ResourceHub::Get<ModelManager>();
	auto& animationClipManager = ResourceHub::Get<AnimationClipManager>();

	// モデルの登録
	modelManager.Register("Player");
	auto modelData = modelManager.Get("Player");

	animationClipManager.Register("Player_Walk");
	animationClipManager.Register("Player_Run");
	animationClipManager.Register("Player_Jump");
	animationClipManager.Register("Player_Idle");
	animationClipManager.Register("Player_Dodge");
	animationClipManager.Register("Player_Punch");

	modelManager.Register("Enemy_Man");
	auto enemyModelData = modelManager.Get("Enemy_Man");

	animationClipManager.Register("Enemy_Man_Walk");
	animationClipManager.Register("Enemy_Man_Run");
	animationClipManager.Register("Enemy_Man_Jump");
	animationClipManager.Register("Enemy_Man_Idle");
	animationClipManager.Register("Enemy_Man_Punch");

	modelManager.Register("Enemy_Woman");
	auto enemyWomanModelData = modelManager.Get("Enemy_Woman");

	animationClipManager.Register("Enemy_Woman_Walk");
	animationClipManager.Register("Enemy_Woman_Run");
	animationClipManager.Register("Enemy_Woman_Jump");
	animationClipManager.Register("Enemy_Woman_Idle");
	animationClipManager.Register("Enemy_Woman_Punch");

	// アニメーションステートテーブルの設定
	using PlayerAnimState = CharacterController::PlayerAnimState;
	using EnemyAnimState = EnemyController::EnemyAnimState;

	static Graphics::Animation::StateTable<PlayerAnimState> playerStateTable;
	static Graphics::Animation::StateTable<EnemyAnimState> enemyStateTable;

	auto clip = animationClipManager.Get("Player_Idle");
	playerStateTable.Set(PlayerAnimState::Idle, { clip, 1.0f, true, 0.15f });
	clip = animationClipManager.Get("Enemy_Man_Idle");
	enemyStateTable.Set(EnemyAnimState::Idle, { clip, 1.0f, true, 0.15f });

	clip = animationClipManager.Get("Player_Walk");
	playerStateTable.Set(PlayerAnimState::Walk, { clip, 1.0f, true, 0.15f });
	clip = animationClipManager.Get("Enemy_Man_Walk");
	enemyStateTable.Set(EnemyAnimState::Walk, { clip, 1.0f, true, 0.15f });

	clip = animationClipManager.Get("Player_Punch");
	playerStateTable.Set(PlayerAnimState::Punching, { clip, 1.0f, false, 0.10f });
	clip = animationClipManager.Get("Enemy_Man_Punch");
	enemyStateTable.Set(EnemyAnimState::Attack, { clip, 1.0f, false, 0.10f });

	clip = animationClipManager.Get("Player_Dodge");
	playerStateTable.Set(PlayerAnimState::Dodging, { clip, 1.0f, false, 0.10f });

	clip = animationClipManager.Get("Player_Jump");
	playerStateTable.Set(PlayerAnimState::Jumping, { clip, 1.0f, false, 0.10f });
	playerStateTable.Set(PlayerAnimState::Run, { clip, 1.0f, true, 0.15f });

	// カメラ
	auto camera = this->gameObjectManager.Instantiate("Camera3D", GameTags::Tag::Camera);
	camera->transform->SetLocalPosition(DX::Vector3(0.0f, -5.0f, -25.0f));
	camera->AddComponent<Camera3D>();
	auto freeCamera = camera->AddComponent<DebugFreeMoveComponent>();
	freeCamera->SetSpeed(12.0f);

	// プレイヤーのアニメーションステートマシンの初期化
	auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	player->transform->SetLocalPosition(DX::Vector3(0.0f, -10.0f, 0.0f));
	player->transform->SetLocalScale(DX::Vector3(0.1f, 0.1f, 0.1f));
	player->TimeScale()->SetGroupName("EnemyTest");

	// プレイヤーのコンポーネントを追加
	auto meshComponent = player->AddComponent<MeshComponent>();
	meshComponent->SetMesh(modelData->mesh);
	auto materialComponent = player->AddComponent<MaterialComponent>();
	materialComponent->SetMaterial(modelData->material);
	auto animationComponent = player->AddComponent<AnimationComponent>();
	animationComponent->SetSkeletonCache(modelData->GetSkeletonCache());

	auto playerAnimator = std::make_unique<Animator<PlayerAnimState>>();
	playerAnimator->Initialize(modelData->GetSkeletonCache(), &playerStateTable, PlayerAnimState::Idle);
	animationComponent->SetAnimator(std::move(playerAnimator));

	player->AddComponent<SkinnedMeshRenderer>();
	player->AddComponent<AnimationStateMachine<PlayerAnimState>>();
	player->AddComponent<CharacterController>();
	player->AddComponent<AttackComponent>();
	player->AddComponent<DodgeComponent>();
	player->AddComponent<MoveComponent>();

	auto collider = player->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	collider->SetCapsule(10.0f, 16.0f);
	collider->SetCenterOffset(DX::Vector3(0.0f, 2.6f, 0.0f));

	auto rigidbody = player->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Player);
	rigidbody->SetMotionTypeKinematic();
	rigidbody->SetUseGravity(true);
	player->AddComponent<ColliderDebugRenderer>();

	// 敵のアニメーションステートマシンの初期化
	auto enemy = this->gameObjectManager.Instantiate("Enemy", GameTags::Tag::Enemy);
	enemy->transform->SetLocalPosition(DX::Vector3(12.0f, -10.0f, 0.0f));
	enemy->transform->SetLocalScale(DX::Vector3(0.01f, 0.01f, 0.01f));
	enemy->TimeScale()->SetGroupName("EnemyTest");

	meshComponent = enemy->AddComponent<MeshComponent>();
	meshComponent->SetMesh(enemyModelData->mesh);
	materialComponent = enemy->AddComponent<MaterialComponent>();
	materialComponent->SetMaterial(enemyModelData->material);
	animationComponent = enemy->AddComponent<AnimationComponent>();
	animationComponent->SetSkeletonCache(enemyModelData->GetSkeletonCache());

	auto enemyAnimator = std::make_unique<Animator<EnemyAnimState>>();
	enemyAnimator->Initialize(enemyModelData->GetSkeletonCache(), &enemyStateTable, EnemyAnimState::Idle);
	animationComponent->SetAnimator(std::move(enemyAnimator));

	enemy->AddComponent<SkinnedMeshRenderer>();
	enemy->AddComponent<AnimationStateMachine<EnemyAnimState>>();
	enemy->AddComponent<NormalBehavior>();
	enemy->AddComponent<InputAdapterAI>();
	enemy->AddComponent<EnemyController>();
	enemy->AddComponent<AttackComponent>();
	enemy->AddComponent<MoveComponent>();

	auto attackTrigger = enemy->AddComponent<Framework::Physics::Collider3DComponent>();
	attackTrigger->SetShape(Framework::Physics::ColliderShapeType::Box);
	attackTrigger->SetCenterOffset(DX::Vector3(0.0f, 2.6f, -5.0f));
	attackTrigger->SetBoxHalfExtent(DX::Vector3(20.0f, 20.0f, 20.0f));
	attackTrigger->SetisTrigger(true);

	collider = enemy->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Capsule);
	collider->SetCapsule(10.0f, 16.0f);
	collider->SetCenterOffset(DX::Vector3(0.0f, 2.6f, 0.0f));

	rigidbody = enemy->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Enemy);
	rigidbody->SetMotionTypeKinematic();
	rigidbody->SetUseGravity(true);
	enemy->AddComponent<ColliderDebugRenderer>();

	// 地面の作成
	auto ground = this->gameObjectManager.Instantiate("EnemyTestGround");
	ground->transform->SetLocalPosition(DX::Vector3(0.0f, -20.0f, 0.0f));
	ground->transform->SetLocalScale(DX::Vector3(100.0f, 1.0f, 100.0f));
	meshComponent = ground->AddComponent<MeshComponent>();
	meshComponent->SetMesh(meshManager.Get("Plane"));
	ground->AddComponent<MeshRenderer>();
	collider = ground->AddComponent<Framework::Physics::Collider3DComponent>();
	collider->SetShape(Framework::Physics::ColliderShapeType::Box);
	rigidbody = ground->AddComponent<Framework::Physics::Rigidbody3D>();
	rigidbody->SetObjectLayer(Framework::Physics::PhysicsLayer::Ground);
	rigidbody->SetMotionTypeStatic();
}

