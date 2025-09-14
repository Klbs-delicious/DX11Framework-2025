/** @file   DirectInputDevice.h
 *  @date   2025/09/14
 */
#pragma once
#include "Framework/Core/IInputDevice.h"

#include <Windows.h>
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

 /**@class DirectInputDevice
  * @brief DirectInput を用いたキーボード・マウス入力の具象デバイス
  * @details
  * - IInputDevice を継承し、DirectInput API による入力取得を実装
  * - キーボードとマウスの押下・トリガー状態を毎フレーム更新
  * - マウス座標は Windows API を用いて取得
  * - 振動制御には対応していないため、SetVibration() は空実装
  */
class DirectInputDevice : public IInputDevice {
public:
    /**@brief コンストラクタ
     * @details メンバ変数の初期化のみを行う。DirectInput の初期化は Initialize() で行う。
     */
    DirectInputDevice();

    /**@brief デストラクタ
     * @details COMポインタの解放は Dispose() を明示的に呼び出すことで行う。
     */
    ~DirectInputDevice();

    /**@brief DirectInput の初期化
     * @param hInst アプリケーションインスタンス
     * @param hwnd ウィンドウハンドル
     * @return 初期化に成功すれば true
     * @details キーボード・マウスのデバイス生成と設定を行う
     */
    bool Initialize(HINSTANCE _hInst, HWND _hwnd);

    /**@brief 解放処理
     * @details COMポインタの Release を行い、リソースを破棄する
     */
    void Dispose();

    /**@brief 入力状態の更新
     * @details 毎フレーム呼び出し、キーボード・マウスの状態を取得・保存する
     */
    void Update() override;

    /**@brief 押下状態の取得
     * @param int _code 入力コード（DIK_* またはマウスボタンID）
     * @return 押されていれば true
     */
    bool IsPressed(int _code) const override;

    /**@brief トリガー状態の取得
     * @param int _code 入力コード（DIK_* またはマウスボタンID）
     * @return bool 前フレームから押された瞬間であれば true
     */
    bool IsTriggered(int _code) const override;

    /**@brief マウスX座標の取得
     * @return int クライアント座標系でのX位置。対応していない場合は -1
     */
    int GetMouseX() const override;

    /**@brief マウスY座標の取得
     * @return int クライアント座標系でのY位置。対応していない場合は -1
     */
    int GetMouseY() const override;

    /**@brief 振動の制御
     * @param const MotorForce& _force 振動の強さ（左右モーター）
     * @details DirectInput では振動制御に対応していないため、空実装
     */
    void SetVibration(const MotorForce& _force) override {}

private:
    
    LPDIRECTINPUT8 dinput = nullptr;          /// @brief DirectInput のルートオブジェクト
    LPDIRECTINPUTDEVICE8 keyboard = nullptr;  /// @brief キーボードデバイス 
    LPDIRECTINPUTDEVICE8 mouse = nullptr;     /// @brief マウスデバイス

    char keyBuffer[256]{};                    /// @brief キーボードの現在の状態    
    char oldKeyBuffer[256]{};                 /// @brief キーボードの前フレームの状態  

    DIMOUSESTATE2 mouseState{};               /// @brief マウスの現在の状態   
    DIMOUSESTATE2 mouseStateOld{};            /// @brief マウスの前フレームの状態
    POINT mousePoint{};                       /// @brief マウスのクライアント座標

    HWND hwnd = nullptr;                      /// @brief ウィンドウハンドル（座標変換に使用）
};

