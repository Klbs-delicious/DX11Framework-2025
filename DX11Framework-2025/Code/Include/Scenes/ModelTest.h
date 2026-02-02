/**	@file	ModelTest.h
*	@date	2025/07/03
*/
#pragma once
#include"Include/Framework/Scenes/BaseScene.h"

/**	@class	ModelTest
 *	@brief	テスト用シーン
 */
class ModelTest :public BaseScene
{
public:
	/**	@brief コンストラクタ
	 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
	 */
	ModelTest(GameObjectManager& _gameObjectManager);

	/// @brief	デストラクタ
	~ModelTest()override;

	 /// @brief	オブジェクトの生成、登録等を行う
	void SetupObjects()override;
private:

};