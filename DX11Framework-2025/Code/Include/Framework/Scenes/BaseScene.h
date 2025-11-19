/**	@file	BaseScene.h
*	@date	2025/07/03
*/
#pragma once

//-----------------------------------------------------------------------------
// Test
//-----------------------------------------------------------------------------
#include"Include/Framework/Entities/GameObjectManager.h"

//-----------------------------------------------------------------------------

/**	@class		BaseScene
 *	@brief		シーン基底クラス
 */
class BaseScene
{
public:
	/**	@brief		オブジェクトの生成、登録等を行う
	 *	@details	純粋仮想関数
	 */
	virtual void SetupObjects() = 0;

	/**	@brief コンストラクタ
	 *	@param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
	 */
	BaseScene(GameObjectManager& _gameObjectManager);

	/// @brief	デストラクタ
	virtual ~BaseScene();

	/**	@brief		ゲームオブジェクトの初期化処理を行う
	 *	@details	継承を禁止する
	 */
	virtual void Initialize()final;

	/**	@brief		オブジェクトの更新を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	virtual void Update(float _deltaTime)final;

	/**	@brief 		オブジェクトの固定更新を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	virtual void FixedUpdate(float _deltaTime)final;

	/**	@brief		ゲームオブジェクトの描画処理を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	virtual void Draw()final;

	/**	@brief		終了処理を行う
	 *	@details	継承を禁止する
	 */
	virtual void Finalize()final;

protected:
	GameObjectManager& gameObjectManager;	///< ゲームオブジェクトの管理
};
