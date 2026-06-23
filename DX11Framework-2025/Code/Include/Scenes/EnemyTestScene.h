/** @file EnemyTestScene.h
 *  @date 2026/06/23
 */
#pragma once

#include "Include/Framework/Scenes/BaseScene.h"

/** @class EnemyTestScene
 *  @brief 敵のAI行動をテストするシーン
 */
class EnemyTestScene : public BaseScene
{
public:
	EnemyTestScene(GameObjectManager& _gameObjectManager, RenderSystem& _renderSystem);
	~EnemyTestScene() override;

	void SetupObjects() override;
};

