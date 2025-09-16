#pragma once
#include"Framework/Scenes/Component.h"
#include"Framework/Scenes/PhaseInterfaces.h"

#include<iostream>

class HogeComponent:public Component,public IUpdatable, public IDrawable
{
public:
	HogeComponent(GameObject* _owner, bool _isActive = true) :Component(_owner, _isActive) { std::cout << "HogeComponentを作成！！" << std::endl; }
	virtual ~HogeComponent()final {}

	void Update(float _deltaTime)override
	{
		std::cout << "HogeComponentを更新" << std::endl;
	}

	void Draw()override
	{
		std::cout << "HogeComponentを描画" << std::endl;
	}

	/// @brief 初期化処理
	void Initialize()override { std::cout << "HogeComponentを生成した！" << std::endl; }

	/// @brief 終了処理
	void Dispose()override { std::cout << "HogeComponentを削除！" << std::endl; }
};
