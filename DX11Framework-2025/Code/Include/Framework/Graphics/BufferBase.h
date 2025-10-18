/** @file   BufferBase.h
 *  @date   2025/10/18
 */
#pragma once
#include <d3d11.h>
#include <wrl/client.h>

 /** @class BufferBase
  *  @brief DirectX11 の GPU バッファ基底クラス
  *  @details
  *  - 頂点 / インデックス / 定数 / 構造化バッファなどに共通する処理を定義
  *  - ID3D11Buffer の生成・更新・破棄などの基本機能を提供
  *  - 継承クラスで用途に応じたバインド処理を実装する
  */
class BufferBase
{
public:
    BufferBase();
    virtual ~BufferBase();

    /**	@brief バッファ内容を更新する
     *	@param ID3D11DeviceContext* _context	D3D11のデバイスコンテキスト
     *	@param const void* _data				書き込み元データ
     *	@param size_t _size						データサイズ（バイト）
     *	@note D3D11_USAGE_DYNAMIC の場合のみ有効
     */
    virtual void Update(ID3D11DeviceContext* _context, const void* _data, size_t _size);

    /// @brief バッファの破棄
    virtual void Release();

    /// @brief バッファ本体の取得 
    ID3D11Buffer* GetBuffer() const { return this->buffer.Get(); }

protected:
    /**	@brief バッファを作成する
     *	@param ID3D11Device* _device		D3D11のデバイス
     *	@param UINT _byteWidth				バッファの総バイトサイズ
     *	@param UINT _bindFlags				バインドフラグ（例: D3D11_BIND_VERTEX_BUFFER）
     *	@param D3D11_USAGE _usage			使用モード（例: D3D11_USAGE_DEFAULT）
     *	@param UINT _cpuAccess				CPUアクセス権限（例: D3D11_CPU_ACCESS_WRITE）
     *	@param const void* _initData		初期データ（nullptr可）
     *	@return bool						生成に成功したら true
     */
    virtual bool Create(
        ID3D11Device* _device,
        UINT _byteWidth,
        UINT _bindFlags,
        D3D11_USAGE _usage = D3D11_USAGE_DEFAULT,
        UINT _cpuAccess = 0,
        const void* _initData = nullptr
    );

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;    ///< バッファ本体

    UINT byteWidth;                                 ///< バッファサイズ
    UINT bindFlags;                                 ///< バインド種別
    D3D11_USAGE usage;                              ///< 使用モード
    UINT cpuAccess;                                 ///< CPUアクセス権限
};
