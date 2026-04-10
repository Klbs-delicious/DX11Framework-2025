/**	@file	SepiaEffectPass.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/D3D11System.h"
#include"Include/Framework/Graphics/PostProcess/IPostProcessPass.h"
#include"Include/Framework/Shaders/ShaderCommon.h"
#include"Include/Framework/Shaders/ShaderManager.h"

/**	@class		SepiaEffectPass
 *	@brief		セピア調のポスト処理を行うクラス
 */
class SepiaEffectPass :public IPostProcessPass
{
public:
	/**	@brief	コンストラクタ
	 * @param _shaderManager シェーダーマネージャーのポインタ
	 */
	SepiaEffectPass(ShaderManager* _shaderManager);
	~SepiaEffectPass();

	/**	@brief	ポストプロセスパスの処理を実行する
	 *	@param	RenderSystem&			_renderSystem	描画システムの参照
	 *	@param	RenderTargetResource*	_inputRT		入力となるレンダーターゲットのリソース
	 *	@param	RenderTargetResource*	_outputRT		出力先のレンダーターゲットのリソース
	 */
	void Execute(
		RenderSystem& _renderSystem, 
		RenderTargetResource* _inputRT, 
		RenderTargetResource* _outputRT
	) override;

	/**	@brief	このポストプロセスパスが有効かどうかを返す
	 *	@return	const bool 有効な場合は true、無効な場合は false
	 */
	const bool IsActive() override;

private:
	ShaderCommon::ShaderProgram* sepiaShaderProgram;	///< セピア調のシェーダープログラム
};