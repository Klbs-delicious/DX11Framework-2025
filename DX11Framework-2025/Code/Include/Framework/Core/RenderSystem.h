/** @file   RenderSystem.h
*   @date   2025/06/20
*/
#pragma once
#include"Include/Framework/Utils/NonCopyable.h"
#include"Include/Framework/Core/D3D11System.h"
#include"Include/Framework/Graphics/ConstantBuffer.h"
#include"Include/Framework/Utils/CommonTypes.h"

#include <memory>
//#include<vector>

/** @enum BlendStateType
 *  @brief ブレンドステートの種類
 */
enum class BlendStateType {
    BS_NONE = 0,      ///< 半透明合成無し
    BS_ALPHABLEND,    ///< 半透明合成
    BS_ADDITIVE,      ///< 加算合成
    BS_SUBTRACTION,   ///< 減算合成
    MAX_BLENDSTATE    ///< ブレンドステートの最大値
};

/** @class      RenderSystem
 *  @brief      D3D11の描画周りを取りまとめたクラス
 *  @details    このクラスはコピー、代入を禁止している
 */
class RenderSystem :private NonCopyable
{
public:
    /** @brief  コンストラクタ
     *  @param  D3D11System*    _d3d11  DirectX11デバイス関連の参照
     *  @param  WindowSystem*   _window ウィンドウ作成等を行うクラスの参照
     */
    RenderSystem(D3D11System* _d3d11, WindowSystem* _window);

    /// @brief  デストラクタ
    ~RenderSystem();

    /** @brief  初期化処理
     *  @return bool 初期化に成功したかどうか
     */
    bool  Initialize();

    /// @brief  終了処理
    void Finalize();

    /// @brief  描画開始時の処理
    void BeginRender();

    /// @brief  描画終了時の処理
    void EndRender();

    /** @brief  ワールド変換行列をGPUに送る
     *  @param  DX::Matrix4x4*    _worldMatrix    ワールド変換行列
     */
    void SetWorldMatrix(DX::Matrix4x4* _worldMatrix);

    /** @brief  プロジェクション変換行列をGPUに送る
     *  @param  DX::Matrix4x4*    _projectionMatrix   プロジェクション変換行列
     */
    void SetProjectionMatrix(DX::Matrix4x4* _projectionMatrix);

    /** @brief  ビュー変換行列をGPUに送る
     *  @param  DX::Matrix4x4*    _viewMatrix ビュー変換行列
     */
    void SetViewMatrix(DX::Matrix4x4* _viewMatrix);

    ///**   @brief  ビューポートを追加
    // *    @param  const D3D11_VIEWPORT& _viewport 追加するビューポート
    // */
    //void AddViewport(const D3D11_VIEWPORT& _viewport);

    ///**   @brief  指定のビューポートを削除
    // *    @param  const int _viewportType ビューポートの番号
    // */
    //void RemoveViewport(const int _viewportType);

    ///**   @brief  指定のビューポートを
    // *    @param  const int _viewportType ビューポートの番号
    // */
    //void RemoveViewport(const int _viewportType);

    /** @brief 指定したブレンドステートを設定
     *  @param BlendStateType _blendState 使用するブレンドステートの種類
     */
    void SetBlendState(BlendStateType _blendState);

    /** @brief Alpha To Coverage（マルチサンプリング対応の透明処理）用のON/OFFを切り替える
     *  @param bool _enable true：ATC有効  false：無効
     *  @details    マルチサンプリング＋アルファブレンドの高度な合成を行う
     */
    void SetATCEnable(bool _enable);

    /** @brief 面の除外（カリング）を無効または有効にする
     *  @param bool _cullflag = false true：カリングON（通常）　false：カリングOFF（両面描画）
     */
    void DisableCulling(bool _cullflag = false);

    /** @brief ラスタライザステートのフィルモード（塗りつぶし/ワイヤーフレーム）を設定する
     *  @param D3D11_FILL_MODE _fillMode D3D11_FILL_SOLID または D3D11_FILL_WIREFRAME
     */
    void SetFillMode(D3D11_FILL_MODE _fillMode);

    /** @brief 深度テストを常にパスさせる設定に変更する
     *  @details
     * - 深度テストは有効（DepthEnable = TRUE）
     * - ただし、常に「描画OK」（DepthFunc = D3D11_COMPARISON_ALWAYS）
     * - 深度バッファにも書き込む（DepthWriteMask = ALL）
     */
    void SetDepthAllwaysWrite();

private:
    D3D11System* d3d11;     ///< DirectX11のデバイス関連の参照
    WindowSystem* window;   ///< ウィンドウ作成等を行うクラスの参照

    //std::vector<D3D11_VIEWPORT>       viewportList;       ///< ビューポートのリスト
    DX::ComPtr<ID3D11RenderTargetView>  renderTargetView;       ///< 描画ターゲット
    DX::ComPtr<ID3D11DepthStencilView>  depthStencilView;       ///< 深度、ステンシル用のバッファ

    std::unique_ptr<ConstantBuffer<DX::Matrix4x4>>  worldBuffer;        ///< ワールド変換行列を保持するバッファ
    std::unique_ptr<ConstantBuffer<DX::Matrix4x4>>  projectionBuffer;   ///< プロジェクション変換行列を保持するバッファ
    std::unique_ptr<ConstantBuffer<DX::Matrix4x4>>  viewBuffer;         ///< ビュー変換行列を保持するバッファ


    DX::ComPtr<ID3D11DepthStencilState> depthStateEnable;   ///< 深度テストを有効にした状態
    DX::ComPtr<ID3D11DepthStencilState> depthStateDisable;  ///< 深度テストを無効にした状態

    DX::ComPtr<ID3D11BlendState> blendState[static_cast<int>(BlendStateType::MAX_BLENDSTATE)];  ///< 各種ブレンドステートを保持する配列
    DX::ComPtr<ID3D11BlendState> blendStateATC;                                                 ///< Alpha To Coverage（マルチサンプリング対応の透明処理）用の専用ブレンドステート
};