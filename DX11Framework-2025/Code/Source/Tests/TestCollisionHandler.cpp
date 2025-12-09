/** @file   TestCollisionHandler.cpp
 *  @brief  自由視点移動デバッグコンポーネントの実装
 *  @date   2025/11/18
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include<iostream>

#include "Include/Tests/TestCollisionHandler.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/DirectInputDevice.h"
#include "Include/Framework/Utils/CommonTypes.h"

//-----------------------------------------------------------------------------
// TestCollisionHandler class
//-----------------------------------------------------------------------------

TestCollisionHandler::TestCollisionHandler(GameObject* _owner, bool _active)
    : Component(_owner, _active)
{}

void TestCollisionHandler::Initialize()
{
}

void TestCollisionHandler::Dispose()
{}

void TestCollisionHandler::OnCollisionEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	std::cout << "[TestCollisionHandler] OnCollisionEnter: " << this->Owner()->GetName() << " collided with " << _other->Owner()->GetName() << std::endl;
}

void TestCollisionHandler::OnCollisionStay(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	std::cout << "[TestCollisionHandler] OnCollisionStay: " << this->Owner()->GetName() << " is colliding with " << _other->Owner()->GetName() << std::endl;
}

void TestCollisionHandler::OnCollisionExit(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)
{
	std::cout << "[TestCollisionHandler] OnCollisionExit: " << this->Owner()->GetName() << " ended collision with " << _other->Owner()->GetName() << std::endl;
}