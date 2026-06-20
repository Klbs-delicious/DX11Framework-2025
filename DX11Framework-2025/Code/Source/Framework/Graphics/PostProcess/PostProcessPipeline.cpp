/**	@file	PostProcessPipeline.cpp
*	@date	2026/04/10
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Include/Framework/Graphics/PostProcess/PostProcessPipeline.h"
#include"Include/Framework/Core/RenderSystem.h"

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
	// パスがない場合はデフォルトの SceneRT をそのまま最終出力
	ID3D11DeviceContext* context = _renderSystem.GetD3D11System()->GetContext();
	if (!context) { return; }

	// バックバッファとシーン描画用RTのリソースを取得
	const RenderTargetResource& backBuffer = _renderSystem.GetRenderTarget(RenderTargetType::DefaultBackBuffer);
	const RenderTargetResource& sceneRt = _renderSystem.GetRenderTarget(RenderTargetType::SceneRT);

	if (!backBuffer.texture2D || !sceneRt.texture2D) { return; }

	if (this->passes.empty())
	{
		context->CopyResource(backBuffer.texture2D.Get(), sceneRt.texture2D.Get());
		return;
	}

	bool executedAnyPass = false;

	for (auto& pass : this->passes)
	{
		// パスが有効な場合のみ実行する
		if (!pass || !pass->IsActive()) { continue; }

		// SceneRT直参照を許容する方針なので inputRT はワークRTのまま
		RenderTargetResource* inputRT = this->workRtA.get();
		RenderTargetResource* outputRT = this->workRtB.get();
		if (!inputRT || !outputRT) { continue; }

		pass->Execute(_renderSystem, inputRT, outputRT);

		// 実行結果を workRtA 側に寄せる
		std::swap(this->workRtA, this->workRtB); 
		executedAnyPass = true;
	}

	if (!executedAnyPass)
	{
		// 1つも実行されなかった場合は SceneRT をそのまま出す
		context->CopyResource(backBuffer.texture2D.Get(), sceneRt.texture2D.Get());
		return;
	}

	if (this->workRtA && this->workRtA->texture2D)
	{
		// 最終結果(workRtA)をバックバッファへ転送する
		context->CopyResource(backBuffer.texture2D.Get(), this->workRtA->texture2D.Get());
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