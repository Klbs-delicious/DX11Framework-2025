/**	@file	D3D11System.h
*	@date	2025/06/16
*/
#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include"Framework/Utils/NonCopyable.h"

// リンクすべき外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

using Microsoft::WRL::ComPtr;

/**@class	D3D11System
 * @brief	D3D11の初期化、後始末を行うクラス
 * @details	このクラスはコピー、代入を禁止している
 */
class D3D11System :private NonCopyable
{
public:
	/** @brief	コンストラクタ
	*/
	D3D11System();

	/** @brief	デストラクタ
	*/
	~D3D11System();

	/** @brief	DX11の初期化
	*/
	static void Initialize();

	/** @brief	DX11の終了処理
	*/
	static void Finalize();

	/** @brief	デバイスの取得
	*	@return	ID3D11Device*
	*/
	inline static ID3D11Device* GetDevice() { return D3D11System::device.Get(); }

	/** @brief	デバイスコンテキストの取得
	*	@return	ID3D11DeviceContext*
	*/
	inline static ID3D11DeviceContext* GetContext() { return D3D11System::deviceContext.Get(); }

private:
	static D3D_FEATURE_LEVEL			featureLevel;		// DX11の機能レベル
	static ComPtr<ID3D11Device>			device;				// デバイス
	static ComPtr<ID3D11DeviceContext>	deviceContext;		// 描画コマンドを出す
	static ComPtr<IDXGISwapChain>		swapChain;			// フレームバッファの管理
	static ComPtr<IDXGIFactory>			factory;			// アダプタ(GPU)情報
};
/*
	static ComPtr<ID3D11RenderTargetView>	targetView;			// 描画ターゲット
	static ComPtr<ID3D11DepthStencilView>	depthStencilView;	// 深度、ステンシル用のバッファ
*/
