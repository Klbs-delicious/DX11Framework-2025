/** @file   D3D11System.h
*   @date   2025/06/16
*/
#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Core/WindowSystem.h"

// リンクすべき外部ライブラリ
#pragma comment(lib,"directxtk.lib")
#pragma comment(lib,"d3d11.lib")

using Microsoft::WRL::ComPtr;

/**@class   D3D11System
 * @brief   D3D11の初期化、後始末を行うクラス
 * @details このクラスはコピー、代入を禁止している
 */
class D3D11System :private NonCopyable
{
public:
    /** @brief  コンストラクタ
    *   @param  WindowSystem* _window   ウィンドウ作成等を行うクラスの参照
    */
    D3D11System(WindowSystem* _window);

    /// @brief  デストラクタ
    ~D3D11System();

    /** @brief DX11の初期化
    *   @return bool 初期化に成功したかどうかを返す
    */
    bool Initialize();

    /// @brief DX11の終了処理
    void Finalize();

    /** @brief  デバイスの取得
    *   @return ID3D11Device*
    */
    inline ID3D11Device* GetDevice() const { return device.Get(); }

    /** @brief  デバイスコンテキストの取得
    *   @return ID3D11DeviceContext*
    */
    inline ID3D11DeviceContext* GetContext() const { return deviceContext.Get(); }

    /** @brief  スワップチェーンの取得
    *   @return IDXGISwapChain*
    */
    inline IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }

private:
    WindowSystem* window;   ///< ウィンドウ作成等を行うクラスの参照

    D3D_FEATURE_LEVEL           featureLevel = D3D_FEATURE_LEVEL_11_0;  ///< DX11の機能レベル
    ComPtr<ID3D11Device>        device;                                 ///< デバイス
    ComPtr<ID3D11DeviceContext> deviceContext;                          ///< 描画コマンドを出す
    ComPtr<IDXGISwapChain>      swapChain;                              ///< フレームバッファの管理
    ComPtr<IDXGIFactory>        factory;                                ///< アダプタ(GPU)情報
};