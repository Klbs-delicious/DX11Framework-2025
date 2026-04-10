/**	@file	SepiaEffectPass.h
*	@date	2026/04/10
*/
#pragma once
#include"Include/Framework/Core/D3D11System.h"

#include"Include/Framework/Graphics/PostProcess/IPostProcessPass.h"
#include"Include/Framework/Graphics/PostProcess/ICondition.h"

#include"Include/Framework/Shaders/ShaderCommon.h"
#include"Include/Framework/Shaders/ShaderManager.h"

/**	@class		SepiaPassCondition
 *	@brief		SepiaEffectPassが有効になる条件を定義するクラス
 *	@details
 *		- ここでは単純に回避状態かどうかを判定する
 */
class SepiaPassCondition :public ICondition
{
public:
	SepiaPassCondition() :isDodge(false) {}
	~SepiaPassCondition() = default;

	/**	@brief	条件を評価する
	 *	@return	bool 条件が満たされている場合は true、そうでない場合は false
	 */
	[[nodiscard]] bool Check()const override
	{
		// ここでは単純に回避状態かどうかを判定して返す
		return this->isDodge;
	}

	/**	@brief	回避状態を設定する
	 *	@param _isDodge 回避中かどうか
	 */
	void SetDodgeState(const bool _isDodge) { this->isDodge = _isDodge; }
private:
	bool isDodge = false;	///< 回避中かどうか
};

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
	void SetCondition(std::unique_ptr<ICondition> _condition) { this->sepiaPassCondition = std::move(_condition); }

private:
	ShaderCommon::ShaderProgram* sepiaShaderProgram;	///< セピア調のシェーダープログラム
	std::unique_ptr<ICondition> sepiaPassCondition;		///< SepiaEffectPassの条件クラス
};