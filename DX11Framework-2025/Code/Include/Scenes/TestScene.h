/**	@file	TestScene.h
*	@date	2025/07/03
*/
#pragma once
#include"Include/Framework/Scenes/BaseScene.h"

/**	@class	TestScene
 *	@brief	テスト用シーン
 */
class TestScene :public BaseScene
{
public:
	/**	@brief コンストラクタ
	 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
	 */
	TestScene(GameObjectManager& _gameObjectManager);

	/// @brief	デストラクタ
	~TestScene()override;

	 /// @brief	オブジェクトの生成、登録等を行う
	void SetupObjects()override;

	//-----------------------------------------------------------------------------
	// 大量オブジェクト生成テスト
	//-----------------------------------------------------------------------------
	void SpawnManyBoxes(const int _countX = 50, const int _countZ = 50, const float _spacing = 3.0f);
private:

};
