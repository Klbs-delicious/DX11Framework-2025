/**	@file	TitleScene.cpp
*	@date	2025/07/04
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Scenes/TitleScene.h"

#include<iostream>

//-----------------------------------------------------------------------------
// TitleScene Class
//-----------------------------------------------------------------------------

/**	@brief コンストラクタ
 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
TitleScene::TitleScene(GameObjectManager& _gameObjectManager) :BaseScene(_gameObjectManager) {}

/// @brief	デストラクタ
TitleScene::~TitleScene() {}

/// @brief	オブジェクトの生成、登録等を行う
void TitleScene::SetupObjects()
{
	std::cout << "シーン名" << "TitleScene" << std::endl;
}