/**	@file	PostProcessPipeline.cpp
*	@date	2026/04/10
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Graphics/PostProcess/PostProcessPipeline.h"

#include <stdexcept>
//-----------------------------------------------------------------------------
// PostProcessPipeline Class
//-----------------------------------------------------------------------------

PostProcessPipeline::PostProcessPipeline() : workRtA(nullptr), workRtB(nullptr) {}

PostProcessPipeline::~PostProcessPipeline()
{
	this->passes.clear();
	this->workRtA.reset();
	this->workRtB.reset();
}

/**	@brief	ポストプロセスパイプラインを初期化する
 *	@param	ID3D11Device* _device Direct3Dデバイスのポインタ
 *	@param	UINT _width レンダーターゲットの幅
 *	@param	UINT _height レンダーターゲットの高さ
 *	@return	bool 初期化に成功したかどうか
 */
bool PostProcessPipeline::Initialize(ID3D11Device* _device, UINT _width, UINT _height)
{
	// ワーク用のレンダーターゲットを作成する
	this->workRtA = std::make_unique<RenderTargetResource>();
	this->workRtB = std::make_unique<RenderTargetResource>();

	bool success = this->workRtA->CreateRenderTarget(_device, _width, _height);
	success = this->workRtB->CreateRenderTarget(_device, _width, _height);

	if (!success)
	{
		throw std::runtime_error("Failed to create SceneRT.");
		return false;
	}
	return true;
}

/**	@brief	ポストプロセスを実行する
 *	@param	RenderSystem& _renderSystem 描画システムの参照
 */
void PostProcessPipeline::Execute(RenderSystem& _renderSystem)
{
	// パスがない場合は何もしない
	if (this->passes.empty()) { return; }

	for (auto& pass : this->passes)
	{
		// パスが有効な場合のみ実行する
		if (!pass->IsActive()) { continue; }

		// 入力と出力のレンダーターゲットを切り替える
		RenderTargetResource* inputRT = this->workRtA.get();
		RenderTargetResource* outputRT = this->workRtB.get();

		// ポスト処理を実行する
		pass->Execute(_renderSystem, inputRT, outputRT);

		// 入力と出力を入れ替える
		std::swap(this->workRtA, this->workRtB);
	}
}

/**	@brief	ポストプロセスパスを追加する
 *	@param	std::unique_ptr<IPostProcessPass> _pass 追加するポストプロセスパスの所有権を持つユニークポインタ
 */
void PostProcessPipeline::AddPass(std::unique_ptr<IPostProcessPass> _pass)
{
	if(!_pass) { return; }
	this->passes.push_back(std::move(_pass));
}