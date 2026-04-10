#include "Include/Game/Graphics/PostProcess/SepiaEffectPass.h"
#include"Include/Framework/Core/RenderSystem.h"

#include<iostream>

/**	@brief	コンストラクタ
 * @param _shaderManager シェーダーマネージャーのポインタ
 */
SepiaEffectPass::SepiaEffectPass(ShaderManager* _shaderManager)
{
	// セピア表現のシェーダープログラムを読み込む
	this->sepiaShaderProgram = _shaderManager->GetShaderProgram("Sepia");

	if(this->sepiaShaderProgram)
	{
		std::cout << "[SepiaEffectPass] シェーダープログラムの読み込みに成功: " << "Sepia" << std::endl;
	}
	else
	{
		std::cerr << "[SepiaEffectPass] シェーダープログラムの読み込みに失敗: " << "Sepia" << std::endl;
	}
}

/// @brief デストラクタ
SepiaEffectPass::~SepiaEffectPass()
{
	this->sepiaShaderProgram = nullptr;
}

/**	@brief	ポストプロセスパスの処理を実行する
 *	@param	RenderSystem&			_renderSystem	描画システムの参照
 *	@param	RenderTargetResource*	_inputRT		入力となるレンダーターゲットのリソース
 *	@param	RenderTargetResource*	_outputRT		出力先のレンダーターゲットのリソース
 */
void SepiaEffectPass::Execute(
	RenderSystem& _renderSystem, 
	RenderTargetResource* _inputRT, 
	RenderTargetResource* _outputRT)
{
	if (!this->sepiaShaderProgram || !_inputRT || !_outputRT) { return; }
	if (!_inputRT->IsValid() || !_outputRT->renderTargetView) { return; }

	ID3D11DeviceContext* context = _renderSystem.GetD3D11System()->GetContext();
	if (!context) { return; }

	// 出力先のRTをセット
	context->RSSetViewports(1, &_outputRT->viewport);
	context->OMSetRenderTargets(1, _outputRT->renderTargetView.GetAddressOf(), nullptr);

	// ポスプロ向け設定
	_renderSystem.SetDepthEnable(false);
	_renderSystem.SetSampler(SamplerType::LinearClamp);
	
	// セピアシェーダーをセット
	//_inputRT->Bind(context, 0);
	_renderSystem.GetRenderTarget(RenderTargetType::SceneRT).Bind(context, 0);
	this->sepiaShaderProgram->Bind(*context);

	// 全画面にクワッドを描画
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->Draw(3, 0);

	// 次フレームの通常描画に影響しないように後片付け
	TextureResource::Unbind(context, 0);
	_renderSystem.SetDepthEnable(true);
}

/**	@brief	このポストプロセスパスが有効かどうかを返す
 *	@return	const bool 有効な場合は true、無効な場合は false
 */
const bool SepiaEffectPass::IsActive()
{
	return this->sepiaPassCondition && this->sepiaPassCondition->Check();
}