/**	@file	SepiaEffectPass.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/D3D11System.h"

#include"Include/Framework/Graphics/PostProcess/IPostProcessPass.h"
#include"Include/Framework/Graphics/DynamicConstantBuffer.h"

#include"Include/Framework/Core/ITimeProvider.h"

#include"Include/Framework/Shaders/ShaderCommon.h"
#include"Include/Framework/Shaders/ShaderManager.h"

//-----------------------------------------------------------------------------
// ShaderCommon namespace
//-----------------------------------------------------------------------------
namespace ShaderCommon
{
	/// @brief セピア調のシェーダーに渡す定数バッファの構造体
	struct SepiaBuffer 
	{
		float intensity;	///< セピア調の強さ（0.0f - 1.0f）
		float padding[3];	///< パディング（16バイト境界に合わせるため）
	};
}

//-----------------------------------------------------------------------------
// SepiaEffectPass class
//-----------------------------------------------------------------------------

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

	/**	@brief	セピア効果の強度を設定する
	 *	@param	_intensity	強度（0.0f～1.0f）
	 */
	void SetIntensity(float _intensity);

private:
	float intensity;			///< エフェクトの強度

	ShaderCommon::ShaderProgram* sepiaShaderProgram;										///< セピア調のシェーダープログラム

	std::unique_ptr<DynamicConstantBuffer<ShaderCommon::SepiaBuffer>> sepiaConstantBuffer;	///< セピア調の定数バッファ
	ShaderCommon::SepiaBuffer sepiaBufferData;												///< シェーダーに送るセピア調のパラメータデータ
};