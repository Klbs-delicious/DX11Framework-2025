/**	@file	BaseScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Scenes/BaseScene.h"

//-----------------------------------------------------------------------------
// RenderSystem Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
BaseScene::BaseScene() {}

/// @brief	デストラクタ
BaseScene::~BaseScene(){}

/**	@brief		ゲームオブジェクトの初期化処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Initialize()
{
	// オブジェクトの生成
	this->SetupObjects();
}

/**	@brief		オブジェクトの更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Update(float _deltaTime)
{

}

/**	@brief		ゲームオブジェクトの描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Draw()
{

}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Finalize()
{

}
