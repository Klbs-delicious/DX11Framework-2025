/** @file   FreeMoveTestComponent.cpp
 *  @brief  自由移動の挙動を検証するテスト用コンポーネントの実装
 *  @date   2025/11/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Tests/FreeMoveTestComponent.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/Rigidbody3D.h"

#include <random>

//-----------------------------------------------------------------------------
// FreeMoveTestComponent class
//-----------------------------------------------------------------------------

FreeMoveTestComponent::FreeMoveTestComponent(GameObject* _owner, bool _active)
	: Component(_owner, _active),
	transform(nullptr),
	rigidbody(nullptr),
	speed(10.0f),
	targetPos(0.0f, 0.0f, 0.0f),
	hasTarget(false)
{
}

void FreeMoveTestComponent::Initialize()
{
	this->transform = this->Owner()->transform;
	this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
}

void FreeMoveTestComponent::Dispose()
{
}

void FreeMoveTestComponent::Update(float _deltaTime)
{
	if (!this->rigidbody || !this->transform)
	{
		return;
	}

	static std::mt19937 engine(std::random_device{}());
	static std::uniform_real_distribution<float> dist(-20.0f, 20.0f);

	if (!this->hasTarget)
	{
		this->targetPos = DX::Vector3(
			dist(engine),
			dist(engine),
			dist(engine)
		);
		this->hasTarget = true;
	}

	DX::Vector3 current = this->transform->GetWorldPosition();
	DX::Vector3 diff = this->targetPos - current;
	float distLen = diff.Length();

	if (distLen < 0.1f)
	{
		this->hasTarget = false;
		return;
	}

	DX::Vector3 dir = diff / distLen;

    // 移動処理
    DX::Vector3 newPos = current + dir * this->speed * _deltaTime;
    this->transform->SetLocalPosition(newPos);
}
