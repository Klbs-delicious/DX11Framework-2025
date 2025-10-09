#pragma once
#include"Include/Framework/Entities/Component.h"
#include"Include/Framework/Entities/PhaseInterfaces.h"

#include<iostream>

class HogeComponent:public Component,public IUpdatable, public IDrawable
{
public:
	HogeComponent(GameObject* _owner, bool _isActive = true) :Component(_owner, _isActive) {
		std::cout << "HogeComponentを作成！！" << std::endl; 
		this->transform = this->owner->GetComponent<Transform>();
		transform->SetWorldPosition({ 1.0f,2.0f,3.0f });
	}
	virtual ~HogeComponent()final {}

	void Update(float _deltaTime)override
	{
		std::cout << "HogeComponentを更新" << std::endl;
		auto pos = this->transform->GetLocalPosition();
		this->transform->SetLocalPosition(DX::Vector3(pos.x + 10, pos.y + 10, pos.z + 10.5f));
		std::cout << this->owner->GetName() << " : " <<
			std::to_string(this->transform->GetWorldPosition().x) <<
			std::to_string(this->transform->GetWorldPosition().y) <<
			std::to_string(this->transform->GetWorldPosition().z) <<
			std::endl;
	}

	void Draw()override
	{
		std::cout << "HogeComponentを描画" << std::endl;
	}

	/// @brief 初期化処理
	void Initialize()override { std::cout << "HogeComponentを生成した！" << std::endl; }

	/// @brief 終了処理
	void Dispose()override { std::cout << "HogeComponentを削除！" << std::endl; }

private:
	Transform* transform = nullptr;
};