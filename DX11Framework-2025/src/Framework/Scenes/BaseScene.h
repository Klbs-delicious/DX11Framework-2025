/**	@file	BaseScene.h
*	@date	2025/07/03
*/
#pragma once

/**	@class		BaseScene
 *	@brief		シーン基底クラス
 */
class BaseScene
{
protected:
	/**	@brief		オブジェクトの生成、登録等を行う
	 *	@details	純粋仮想関数
	 */
	virtual void SetupObjects() = 0;

public:
	/// @brief	コンストラクタ
	BaseScene();
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

	/**	@brief		ゲームオブジェクトの描画処理を行う
	 *	@param		float _deltaTime	デルタタイム
	 *	@details	継承を禁止する
	 */
	virtual void Draw()final;

	/**	@brief		終了処理を行う
	 *	@details	継承を禁止する
	 */
	virtual void Finalize()final;
};
