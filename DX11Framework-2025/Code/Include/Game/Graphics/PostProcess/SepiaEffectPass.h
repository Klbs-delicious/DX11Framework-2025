/**	@file	SepiaEffectPass.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/D3D11System.h"

#include"Include/Framework/Graphics/PostProcess/IPostProcessPass.h"
#include"Include/Framework/Graphics/PostProcess/IEffectRequest.h"
#include"Include/Framework/Graphics/DynamicConstantBuffer.h"

#include"Include/Framework/Core/TimeScaleSystem.h"
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
// SepiaEffectRequest class
//-----------------------------------------------------------------------------

/**	@class		SepiaEffectRequest
 *	@brief		SepiaEffectPassが有効になる条件を定義するクラス
 *	@details
 *		- ここでは単純に回避状態かどうかを判定する
 */
class SepiaEffectRequest :public IEffectRequest
{
public:
	SepiaEffectRequest() :isDodge(false) {}
	~SepiaEffectRequest() = default;

	/**	@brief	条件を評価する
	 *	@return	bool 条件が満たされている場合は true、そうでない場合は false
	 */
	[[nodiscard]] bool Check()const override
	{
		// 回避状態で、
		// かつジャスト回避のタイムスケールエフェクトがアクティブ、
		// もしくは進行中であれば条件を満たす
		return this->isDodge && (this->justDodgeContext.isActive || this->justDodgeContext.progress > 0.0f);
	}

	/**	@brief	ジャスト回避のタイムスケールエフェクトのコンテキストを取得する
	 *	@return	const TimeScaleEffectContext& ジャスト回避のタイムスケールエフェクトのコンテキストの定数参照
	 */
	const TimeScaleEffectContext& GetSetJustDodgeContext() const { return this->justDodgeContext; }

	/**	@brief	ジャスト回避のタイムスケールエフェクトのコンテキストを設定する
	 *	@param _ctx 設定するジャスト回避のタイムスケールエフェクトのコンテキスト
	 */
	void SetJustDodgeContext(const TimeScaleEffectContext& _ctx) { this->justDodgeContext = _ctx; }

	/**	@brief	回避状態を設定する
	 *	@param _isDodge 回避中かどうか
	 */
	void SetDodgeState(const bool _isDodge) { this->isDodge = _isDodge; }
private:
	bool isDodge = false;						///< 回避中かどうか
	TimeScaleEffectContext justDodgeContext;	///< ジャスト回避のタイムスケールエフェクトのコンテキスト
};

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

	/**	@brief	SepiaPassConditionを設定する
	 *	@param	_condition 設定する条件
	 */
	void SetCondition(std::unique_ptr<SepiaEffectRequest> _condition) { this->sepiaPassCondition = std::move(_condition); }

private:
	ITimeProvider& timeProvider;	///< 時間情報の提供元
	float smoothIntensity;			///< フェード処理用のスムースな強度

	ShaderCommon::ShaderProgram* sepiaShaderProgram;										///< セピア調のシェーダープログラム
	std::unique_ptr<SepiaEffectRequest> sepiaPassCondition;									///< SepiaEffectPassの条件クラス

	std::unique_ptr<DynamicConstantBuffer<ShaderCommon::SepiaBuffer>> sepiaConstantBuffer;	///< セピア調の定数バッファ
	ShaderCommon::SepiaBuffer sepiaBufferData;												///< シェーダーに送るセピア調のパラメータデータ
};