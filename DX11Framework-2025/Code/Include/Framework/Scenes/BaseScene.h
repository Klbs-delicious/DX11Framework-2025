/**	@file	BaseScene.h
*	@date	2025/07/03
*/
#pragma once

#include"Include/Framework/Entities/GameObjectManager.h"
#include"Include/Framework/Core/RenderSystem.h"
#include"Include/Framework/Graphics/PostProcess/PostProcessPipeline.h"

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
	 *	@param RenderSystem&	_renderSystem		レンダリングシステムの管理
	 */
	BaseScene(GameObjectManager& _gameObjectManager, RenderSystem& _renderSystem);

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

	/**	@brief		ゲームオブジェクトマネージャの参照を取得する
	 *	@return		GameObjectManager&	ゲームオブジェクトマネージャの参照
	 */
	[[nodiscard]] GameObjectManager& GetGameObjectManager() { return this->gameObjectManager;; }

protected:
	GameObjectManager& gameObjectManager;			///< ゲームオブジェクトの管理
	RenderSystem& renderSystem;						///< レンダリングシステムの管理
	PostProcessPipeline* postProcessPipeline;		///< ポスト処理の管理
};
