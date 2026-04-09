/**	@file	IPostProcessPass.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/RenderSystem.h"
#include"Include/Framework/Graphics/RenderTargetResource.h"

/**	@class		IPostProcessPass
 *	@brief		ポストプロセスパスのインターフェース
 */
class IPostProcessPass
{
public:
	virtual ~IPostProcessPass() = default;

	/**	@brief	ポストプロセスパスの処理を実行する
	 *	@param	RenderSystem&			_renderSystem	描画システムの参照
	 *	@param	RenderTargetResource*	_inputRT		入力となるレンダーターゲットのリソース
	 *	@param	RenderTargetResource*	_outputRT		出力先のレンダーターゲットのリソース
	 */
	virtual void Execute(RenderSystem& _renderSystem, 
		RenderTargetResource* _inputRT, 
		RenderTargetResource* _outputRT
	) = 0;

	/**	@brief	このポストプロセスパスが有効かどうかを返す
	 *	@return	bool 有効な場合は true、無効な場合は false
	 */
	virtual bool IsActive() = 0;
};