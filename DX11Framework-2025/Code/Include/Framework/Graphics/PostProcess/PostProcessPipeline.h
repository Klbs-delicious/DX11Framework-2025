/**	@file	PostProcessPipeline.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/RenderSystem.h"

#include"Include/Framework/Graphics/PostProcess/IPostProcessPass.h"
#include"Include/Framework/Graphics/RenderTargetResource.h"

#include<memory>
#include<vector>

/**	@class		PostProcessPipeline
 *	@brief		ポストプロセスの進行を管理するクラス
 */
class PostProcessPipeline
{
public:
	PostProcessPipeline();
	~PostProcessPipeline();

	/**	@brief	ポストプロセスパイプラインを初期化する
	 *	@param	ID3D11Device* _device Direct3Dデバイスのポインタ
	 *	@param	UINT _width レンダーターゲットの幅
	 *	@param	UINT _height レンダーターゲットの高さ
	 *	@return	bool 初期化に成功したかどうか
	 */
	bool Initialize(ID3D11Device* _device, UINT _width, UINT _height);

	/**	@brief	ポストプロセスを実行する
	 *	@param	RenderSystem& _renderSystem 描画システムの参照
	 */
	void Execute(RenderSystem& _renderSystem);

	/**	@brief	ポストプロセスパスを追加する
	 *	@param	std::unique_ptr<IPostProcessPass> _pass 追加するポストプロセスパスの所有権を持つユニークポインタ
	 */
	void AddPass(std::unique_ptr<IPostProcessPass> _pass);

	/// @brief	ポストプロセスパスを全てクリアする
	void ClearPasses() { this->passes.clear(); }
private:
	std::vector<std::unique_ptr<IPostProcessPass>> passes;	///< ポストプロセスパスのリスト
	std::unique_ptr<RenderTargetResource> workRtA;			///< パス間の入出力に使用するレンダーターゲットA
	std::unique_ptr<RenderTargetResource> workRtB;			///< パス間の入出力に使用するレンダーターゲットB
};