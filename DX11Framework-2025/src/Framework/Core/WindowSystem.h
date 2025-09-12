/** @file   WindowSystem.h
*   @date   2025/06/12
*/
#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <string_view>
#include"Framework/Utils/NonCopyable.h"

/**@class   WindowSystem
 * @brief   ウィンドウ作成の管理を行う
 * @details このクラスはコピー、代入を禁止している
 */
class WindowSystem :private NonCopyable
{
public:
    /** @brief  コンストラクタ
    */
    WindowSystem();

    /** @brief デストラクタ
    */
    ~WindowSystem();

    /** @brief ウィンドウの初期化処理
     *  @param  const uint32_t ウィンドウの縦幅
     *  @param  const uint32_t ウィンドウの横幅
     */
    bool Initialize(const uint32_t _width, const uint32_t _height);

    /** @brief ウィンドウの終了処理
    */
    void Finalize();

    /** @brief  ウィンドウタイトルの変更
    *   @param  const std::wstring_view _windowTitle    ウィンドウのタイトル
    */
    void SetWindowTitle(const std::wstring_view _windowTitle);

    /** @brief  ウィンドウサイズの設定
    *   @param  const uint32_t ウィンドウの縦幅
    *   @param  const uint32_t ウィンドウの横幅
    */
    void SetWindowSize(const uint32_t _width, const uint32_t _height);

    /** @brief  ウィンドウの横幅の取得
    *   @return uint32_t ウィンドウの横幅
    */
    inline uint32_t GetWidth() const { return this->width; }

    /** @brief  ウィンドウの縦幅の取得
    *   @return uint32_t ウィンドウの縦幅
    */
    inline uint32_t GetHeight() const { return this->height; }

    /** @brief  ウィンドウハンドルの取得
    *   @return HWND ウィンドウハンドル
    */
    inline HWND GetWindow() const { return this->hWnd; }

    /** @brief  インスタンスハンドルの取得
    *   @return HINSTANCE インスタンスハンドル
    */
    inline HINSTANCE GetHInstance() const { return this->hInstance; }

    /**@brief ウィンドウプロシージャ
     * @param   HWND    _hWnd   ウィンドウハンドル
     * @param   UINT    _msg    メッセージ
     * @param   WPARAM  _wp     パラメータ
     * @param   LPARAM  _lp     パラメータ
     * @return  LRESULT         処理結果
     * @details ウィンドウに送られたメッセージを処理する
     */
    static LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wp, LPARAM _lp);

private:
    //ウィンドウの初期背景色
    enum class BackColorBrush
    {
        WHITE = WHITE_BRUSH,    //0
        LTGRAY = LTGRAY_BRUSH,  //1
        GRAY = GRAY_BRUSH,      //2
        DKGRAY = DKGRAY_BRUSH,  //3
        BLACK = BLACK_BRUSH,    //4
        NOTHING = NULL_BRUSH,   //5
        HOLLOW = HOLLOW_BRUSH,  //=NULL_BRUSH
    };

    std::wstring    className;      // ウィンドウクラス名
    std::wstring    windowTitle;    // ウィンドウタイトル名

    uint32_t width;     // ウィンドウ横幅
    uint32_t height;    // ウィンドウ縦幅

    HINSTANCE   hInstance;  // インスタンスハンドル
    HWND        hWnd;       // ウィンドウハンドル
};