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

/// @brief	コンストラクタ
TitleScene::TitleScene() {}

/// @brief	デストラクタ
TitleScene::~TitleScene() {}

/// @brief	オブジェクトの生成、登録等を行う
void TitleScene::SetupObjects()
{
	std::cout << "シーン名" << "TitleScene" << std::endl;
}
