/** @file   DirectInputDevice.h
 *  @date   2025/09/14
 */
#pragma once
#include "Include/Framework/Core/IInputDevice.h"

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
    void Dispose()override;

    /**@brief 入力状態の更新
     * @details 毎フレーム呼び出し、キーボード・マウスの状態を取得・保存する
     */
    void Update() override;

    /**@brief 押下状態の取得
     * @param int _code 入力コード（DIK_* またはマウスボタンID）
     * @return bool	押されていれば true
     */
    bool IsPressed(int _code) const override;

    /**@brief トリガー状態の取得
     * @param int _code 入力コード（DIK_* またはマウスボタンID）
     * @return bool 前フレームから押された瞬間であれば true
     */
    bool IsTriggered(int _code) const override;

    /**@brief リリース状態の取得
     * @param int _code 入力コード（DIK_* またはマウスボタンID）
     * @return bool 前フレームから離された瞬間であれば true
	 */
	bool IsReleased(int _code) const override;

    /**@brief マウスX座標の取得
     * @return int クライアント座標系でのX位置。対応していない場合は -1
     */
    int GetMouseX() const override;

    /**@brief マウスY座標の取得
     * @return int クライアント座標系でのY位置。対応していない場合は -1
     */
    int GetMouseY() const override;

    /** @brief マウスの移動量（Δ）を取得
     *  @param int& _dx X方向の変化量
     *  @param int& _dy Y方向の変化量
     *  @return なし
     */
    void GetMouseDelta(int& _dx, int& _dy) const override;

    /**@brief 振動の制御
     * @param const MotorForce& _force 振動の強さ（左右モーター）
     * @details DirectInput では振動制御に対応していないため、空実装
     */
    void SetVibration(const MotorForce& _force) override {}

    /**@enum KeyboardKey制御
     * @brief DirectInput におけるキーボードの各キーに対応するコード
     * @details DIK_* に対応するキーコードを型安全に扱うための列挙型
     */
    enum class KeyboardKey : int {
        Escape = 0x01,
        Num1 = 0x02,
        Num2 = 0x03,
        Num3 = 0x04,
        Num4 = 0x05,
        Num5 = 0x06,
        Num6 = 0x07,
        Num7 = 0x08,
        Num8 = 0x09,
        Num9 = 0x0A,
        Num0 = 0x0B,

        A = 0x1E,
        B = 0x30,
        C = 0x2E,
        D = 0x20,
        E = 0x12,
        F = 0x21,
        G = 0x22,
        H = 0x23,
        I = 0x17,
        J = 0x24,
        K = 0x25,
        L = 0x26,
        M = 0x32,
        N = 0x31,
        O = 0x18,
        P = 0x19,
        Q = 0x10,
        R = 0x13,
        S = 0x1F,
        T = 0x14,
        U = 0x16,
        V = 0x2F,
        W = 0x11,
        X = 0x2D,
        Y = 0x15,
        Z = 0x2C,

        Space = 0x39,
        Enter = 0x1C,
        Tab = 0x0F,
        Backspace = 0x0E,
        LShift = 0x2A,
        RShift = 0x36,
        LCtrl = 0x1D,
        RCtrl = 0x9D,
        LAlt = 0x38,
        RAlt = 0xB8,

        F1 = 0x3B,
        F2 = 0x3C,
        F3 = 0x3D,
        F4 = 0x3E,
        F5 = 0x3F,
        F6 = 0x40,
        F7 = 0x41,
        F8 = 0x42,
        F9 = 0x43,
        F10 = 0x44,
        F11 = 0x57,
        F12 = 0x58,

        UpArrow = 0xC8,
        DownArrow = 0xD0,
        LeftArrow = 0xCB,
        RightArrow = 0xCD
    };

    /**@enum MouseButton制御
     * @brief DirectInput におけるマウスの各ボタンに対応するコード
     * @details DIK_* に対応するキーコードを型安全に扱うための列挙型
     */
    enum class MouseButton : int {
        Left = 0,  // rgbButtons[0]
        Right = 1,  // rgbButtons[1]
        Middle = 2,  // rgbButtons[2]
        XButton1 = 3,  // rgbButtons[3]
        XButton2 = 4   // rgbButtons[4]
    };

    /**@enum GamepadButton制御
     * @brief DirectInput におけるゲームパッドの各ボタンに対応するコード
     * @details DIK_* に対応するキーコードを型安全に扱うための列挙型
     */
    enum class GamepadButton : int {
        A = 0x200,
        B = 0x201,
        X = 0x202,
        Y = 0x203,
        Start = 0x204,
        Back = 0x205,
        LB = 0x206,
        RB = 0x207,
        LT = 0x208,
        RT = 0x209,
        DPadUp = 0x20A,
        DPadDown = 0x20B,
        DPadLeft = 0x20C,
        DPadRight = 0x20D
    };

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

