/**	@file	TitleScene.h
*	@date	2025/07/04
*/
#pragma once
#include"Include/Framework/Scenes/BaseScene.h"

/**	@class	TitleScene
 *	@brief	テスト用シーン
 */
class TitleScene :public BaseScene
{
public:
	/**	@brief コンストラクタ
	 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
	 */
	TitleScene(GameObjectManager& _gameObjectManager);

	/// @brief	デストラクタ
	~TitleScene()override;

	 /// @brief	オブジェクトの生成、登録等を行う
	void SetupObjects()override;
private:

};
