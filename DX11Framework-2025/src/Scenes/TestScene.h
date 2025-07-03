/**	@file	TestScene.h
*	@date	2025/07/03
*/
#pragma once
#include"Framework/Scenes/BaseScene.h"

/**	@class	TestScene
 *	@brief	テスト用シーン
 */
class TestScene :public BaseScene
{
public:
	/// @brief	コンストラクタ
	TestScene();
	/// @brief	デストラクタ
	~TestScene()override;

	 /// @brief	オブジェクトの生成、登録等を行う
	void SetupObjects()override;
private:

};
