#include "Include/Game/Graphics/PostProcess/SepiaEffectPass.h"
#include"Include/Framework/Core/RenderSystem.h"
#include"Include/Framework/Core/SystemLocator.h"
#include"Include/Framework/Core/D3D11System.h"

#include<iostream>

/**	@brief	コンストラクタ
 * @param _shaderManager シェーダーマネージャーのポインタ
 */
SepiaEffectPass::SepiaEffectPass(ShaderManager* _shaderManager) :
	timeProvider(SystemLocator::Get<ITimeProvider>()), 
	sepiaShaderProgram(nullptr),
	smoothIntensity(0.0f),
	sepiaConstantBuffer(std::make_unique<DynamicConstantBuffer<ShaderCommon::SepiaBuffer>>())
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

	// 定数バッファの作成（初期値は効果なし）
	this->sepiaConstantBuffer->Create(SystemLocator::Get<D3D11System>().GetDevice());
	this->sepiaBufferData.intensity = 0.0f;
	this->sepiaConstantBuffer->Update(SystemLocator::Get<D3D11System>().GetContext(), this->sepiaBufferData);
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
	float dt = this->timeProvider.RawDelta();

	// フェード処理用パラメータの計算
	auto justDodgeContext = this->sepiaPassCondition->GetSetJustDodgeContext();
	if (justDodgeContext.isActive)
	{
		// スロー中は常にセピア効果を最大にする
		this->smoothIntensity = 1.0f;
	}
	else
	{
		// スローが終わった後の余韻をフェードで表現 (0.5秒でフェードアウト)
		this->smoothIntensity -= dt * 5.0f;
		if (this->smoothIntensity < 0.0f) this->smoothIntensity = 0.0f;
	}

	if (this->smoothIntensity <= 0.0f) { return; }
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
	
	// セピアの適用度を更新
	this->sepiaBufferData.intensity = this->smoothIntensity;
	this->sepiaConstantBuffer->Update(context, this->sepiaBufferData);
	
	// セピアシェーダー、定数バッファをセット
	_renderSystem.GetRenderTarget(RenderTargetType::SceneRT).Bind(context, 0);
	this->sepiaShaderProgram->Bind(*context);
	this->sepiaConstantBuffer->BindPS(context, 0); //<- これを追加

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
	// 条件（スロー中か）をチェック
	bool isRequestActive = this->sepiaPassCondition && this->sepiaPassCondition->Check();

	// まだフェードアウト中（色が残っている）か
	bool isFadingOut = this->smoothIntensity > 0.0f;

	return isRequestActive || isFadingOut;
}