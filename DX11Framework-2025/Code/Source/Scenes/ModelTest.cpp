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
#include"Include/Framework/Graphics/ModelManager.h"
#include"Include/Framework/Graphics/AnimationClipManager.h"

#include"Include/Tests/TestMoveComponent.h"
#include"Include/Tests/TimeScaleTestComponent.h"
#include"Include/Tests/FreeMoveTestComponent.h"
#include"Include/Tests/TestCollisionHandler.h"
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
	
	// モデルデータの取得テスト
	modelManager.Register("Player");
	auto modelData = modelManager.Get("Player");

	auto player = this->gameObjectManager.Instantiate("Player", GameTags::Tag::Player);
	player->transform->SetLocalPosition(DX::Vector3(0.0f, 0.0f, 0.0f));
	player->transform->SetLocalScale(DX::Vector3(0.001f, 0.001f, 0.001f));
	auto meshComp = player->AddComponent<MeshComponent>();
	meshComp->SetMesh(modelData->mesh);
	auto materialComp = player->AddComponent<MaterialComponent>();
	materialComp->SetMaterial(modelData->material);
	player->AddComponent<MeshRenderer>();

	// アニメーションデータの取得テスト
	animationClipManager.Register("Walk");
	animationClipManager.Register("Run");
	animationClipManager.Register("Jump");
	auto clip = animationClipManager.Get("Run");
	if (clip)
	{
		std::cout << "[ModelTest] Animation Clip Loaded: " << clip->name << std::endl;
		std::cout << " Duration (ticks): " << clip->durationTicks << std::endl;
		std::cout << " Ticks Per Second: " << clip->ticksPerSecond << std::endl;
		std::cout << " Number of Tracks: " << clip->tracks.size() << std::endl;
	}
	else
	{
		std::cerr << "[ModelTest] Failed to load animation clip." << std::endl;
	}
}