/**	@file	GameScene.h
*	@date	2025/07/04
*/
#pragma once
#include"Include/Framework/Scenes/BaseScene.h"

/**	@class	GameScene
 *	@brief	テスト用シーン
 */
class GameScene :public BaseScene
{
public:
	/**	@brief コンストラクタ
	 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
	 */
	GameScene(GameObjectManager& _gameObjectManager);

	/// @brief	デストラクタ
	~GameScene()override;

	 /// @brief	オブジェクトの生成、登録等を行う
	void SetupObjects()override;
private:
};