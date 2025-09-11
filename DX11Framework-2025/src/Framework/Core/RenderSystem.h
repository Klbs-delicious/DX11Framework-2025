/**	@file	RenderSystem.h
*	@date	2025/06/20
*/
#pragma once
#include"Framework/Core/D3D11System.h"
#include"Framework/Utils/NonCopyable.h"

#include <SimpleMath.h>
#include<vector>

/** @enum BlendStateType
 *	@brief ブレンドステートの種類
 */
enum class BlendStateType {
	BS_NONE = 0,      ///< 半透明合成無し
	BS_ALPHABLEND,    ///< 半透明合成
	BS_ADDITIVE,      ///< 加算合成
	BS_SUBTRACTION,   ///< 減算合成
	MAX_BLENDSTATE    ///< ブレンドステートの最大値
};

/**	@class		RenderSystem
 *	@brief		D3D11の描画周りを取りまとめたクラス
 *	@details	このクラスはコピー、代入を禁止している
 */
class RenderSystem :private NonCopyable
{
public:
	/// @brief	コンストラクタ
	RenderSystem();

	/// @brief	デストラクタ
	~RenderSystem();

	/**	@brief	初期化処理
	 *	@return	bool 初期化に成功したかどうか
	 */
	static bool  Initialize();

	/// @brief	終了処理
	static void Finalize();

	/// @brief	描画開始時の処理
	static void BeginRender();

	/// @brief	描画終了時の処理
	static void EndRender();

	/**	@brief	ワールド変換行列をGPUに送る
	 *	@param	DirectX::SimpleMath::Matrix*	_worldMatrix	ワールド変換行列
	 */
	static void SetWorldMatrix(DirectX::SimpleMath::Matrix*	_worldMatrix);

	/**	@brief	プロジェクション変換行列をGPUに送る
	 *	@param	DirectX::SimpleMath::Matrix*	_projectionMatrix	プロジェクション変換行列
	 */
	static void SetProjectionMatrix(DirectX::SimpleMath::Matrix*	_projectionMatrix);

	/**	@brief	ビュー変換行列をGPUに送る
	 *	@param	DirectX::SimpleMath::Matrix*	_viewMatrix	ビュー変換行列
	 */
	static void SetViewMatrix(DirectX::SimpleMath::Matrix*	_viewMatrix);

	///**	@brief	ビューポートを追加
	// *	@param	const D3D11_VIEWPORT& _viewport	追加するビューポート
	// */
	//static void AddViewport(const D3D11_VIEWPORT& _viewport);

	///**	@brief	指定のビューポートを削除
	// *	@param	const int _viewportType	ビューポートの番号
	// */
	//static void RemoveViewport(const int _viewportType);

	///**	@brief	指定のビューポートを
	// *	@param	const int _viewportType	ビューポートの番号
	// */
	//static void RemoveViewport(const int _viewportType);

	/**	@brief 指定したブレンドステートを設定
	 *	@param BlendStateType _blendState 使用するブレンドステートの種類
	 */
	static void SetBlendState(BlendStateType _blendState);

	/** @brief Alpha To Coverage（マルチサンプリング対応の透明処理）用のON/OFFを切り替える
	 *  @param bool	_enable	true：ATC有効	false：無効
	 *  @details    マルチサンプリング＋アルファブレンドの高度な合成を行う
	 */
	static void SetATCEnable(bool _enable);

	/**	@brief 面の除外（カリング）を無効または有効にする
	 *	@param bool _cullflag = false true：カリングON（通常）　false：カリングOFF（両面描画）
	 */
	static void DisableCulling(bool _cullflag = false);

	/**	@brief ラスタライザステートのフィルモード（塗りつぶし/ワイヤーフレーム）を設定する
	 *	@param D3D11_FILL_MODE _fillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
	 */
	static void SetFillMode(D3D11_FILL_MODE _fillMode);


	/** @brief 深度テストを常にパスさせる設定に変更する
	 *  @details
	 * - 深度テストは有効（DepthEnable = TRUE）
	 * - ただし、常に「描画OK」（DepthFunc = D3D11_COMPARISON_ALWAYS）
	 * - 深度バッファにも書き込む（DepthWriteMask = ALL）
	 */
	static void SetDepthAllwaysWrite();

private:
	//static std::vector<D3D11_VIEWPORT>		viewportList;		///< ビューポートのリスト
	static ComPtr<ID3D11RenderTargetView>	renderTargetView;		///< 描画ターゲット
	static ComPtr<ID3D11DepthStencilView>	depthStencilView;		///< 深度、ステンシル用のバッファ

	static ComPtr<ID3D11Buffer>	worldBuffer;		///< ワールド変換行列を保持するバッファ
	static ComPtr<ID3D11Buffer>	projectionBuffer;	///< プロジェクション変換行列を保持するバッファ
	static ComPtr<ID3D11Buffer>	viewBuffer;			///< ビュー変換行列を保持するバッファ

	static ComPtr<ID3D11DepthStencilState> depthStateEnable;	///< 深度テストを有効にした状態
	static ComPtr<ID3D11DepthStencilState> depthStateDisable;	///< 深度テストを無効にした状態

	static ComPtr<ID3D11BlendState> blendState[static_cast<int>(BlendStateType::MAX_BLENDSTATE)];	///< 各種ブレンドステートを保持する配列
	static ComPtr<ID3D11BlendState> blendStateATC;													///< Alpha To Coverage（マルチサンプリング対応の透明処理）用の専用ブレンドステート
};	