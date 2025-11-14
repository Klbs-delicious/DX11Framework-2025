/**	@file	DirectInputDevice.cpp
*	@date	2025/09/14
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Core/DirectInputDevice.h"

//-----------------------------------------------------------------------------
// DirectInputDevice Class
//-----------------------------------------------------------------------------

/**
 * @brief コンストラクタ
 * @details メンバ変数は初期化リストでゼロ初期化されているため、特別な処理は不要
 */
DirectInputDevice::DirectInputDevice() = default;

/**@brief デストラクタ
 * @details 明示的に Dispose() を呼び出す必要あり
 */
DirectInputDevice::~DirectInputDevice() = default;

/**@brief DirectInput の初期化
 * @param hInst アプリケーションインスタンス
 * @param hwnd ウィンドウハンドル
 * @return 初期化に成功すれば true
 * @details キーボード・マウスのデバイス生成と設定を行う
 */
bool DirectInputDevice::Initialize(HINSTANCE _hInst, HWND _hwnd) {
    this->hwnd = _hwnd;

    // DirectInput オブジェクトの生成
    if (FAILED(DirectInput8Create(_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&this->dinput, nullptr))) {
        return false;
    }

    // キーボードデバイスの生成
    if (FAILED(this->dinput->CreateDevice(GUID_SysKeyboard, &this->keyboard, nullptr))) {
        return false;
    }
    this->keyboard->SetDataFormat(&c_dfDIKeyboard);
    this->keyboard->SetCooperativeLevel(this->hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    this->keyboard->Acquire();

    // マウスデバイスの生成
    if (FAILED(this->dinput->CreateDevice(GUID_SysMouse, &this->mouse, nullptr))) {
        return false;
    }
    this->mouse->SetDataFormat(&c_dfDIMouse2);
    this->mouse->SetCooperativeLevel(this->hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    this->mouse->Acquire();

    return true;
}

/// @brief DirectInput の解放処理
void DirectInputDevice::Dispose() {
    if (this->keyboard) {
        this->keyboard->Unacquire();
        this->keyboard->Release();
        this->keyboard = nullptr;
    }
    if (this->mouse) {
        this->mouse->Unacquire();
        this->mouse->Release();
        this->mouse = nullptr;
    }
    if (this->dinput) {
        this->dinput->Release();
        this->dinput = nullptr;
    }
}

/**@brief 入力状態の更新
 * @details 毎フレーム呼び出し、キーボード・マウスの状態を取得・保存する
 */
void DirectInputDevice::Update() {
    // 前フレームの状態を保存
    memcpy(this->oldKeyBuffer, this->keyBuffer, sizeof(this->keyBuffer));
    this->mouseStateOld = this->mouseState;

    // キーボード状態取得
    if (this->keyboard) {
        this->keyboard->Acquire();
        this->keyboard->GetDeviceState(sizeof(this->keyBuffer), this->keyBuffer);
    }

    // マウス状態取得
    if (this->mouse) {
        this->mouse->Acquire();
        this->mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &this->mouseState);
    }

    // マウス座標取得（Windows API）
    POINT screenPos{};
    if (GetCursorPos(&screenPos) && ScreenToClient(this->hwnd, &screenPos)) {
        this->mousePoint = screenPos;
    }
    else {
        this->mousePoint = { -1, -1 };
    }
}

/**@brief 押下状態の取得
 * @param int _code 入力コード（DIK_* またはマウスボタンID）
 * @return 押されていれば true
 */
bool DirectInputDevice::IsPressed(int _code) const {
    if (_code >= 0 && _code < 256) {
        return (this->keyBuffer[_code] & 0x80) != 0;
    }
    if (_code >= 0 && _code < 8) {
        return (this->mouseState.rgbButtons[_code] & 0x80) != 0;
    }
    return false;
}

/**@brief トリガー状態の取得
 * @param int _code 入力コード（DIK_* またはマウスボタンID）
 * @return bool 前フレームから押された瞬間であれば true
 */
bool DirectInputDevice::IsTriggered(int _code) const {
    if (_code >= 0 && _code < 256) {
        return !(this->oldKeyBuffer[_code] & 0x80) && (this->keyBuffer[_code] & 0x80);
    }
    if (_code >= 0 && _code < 8) {
        return !(this->mouseStateOld.rgbButtons[_code] & 0x80) && (this->mouseState.rgbButtons[_code] & 0x80);
    }
    return false;
}

/**@brief リリース状態の取得
 * @param int _code 入力コード（DIK_* またはマウスボタンID）
 * @return bool 前フレームから離された瞬間であれば true
 */
bool DirectInputDevice::IsReleased(int _code) const {
    if (_code >= 0 && _code < 256) {
        return (this->oldKeyBuffer[_code] & 0x80) && !(this->keyBuffer[_code] & 0x80);
    }
    if (_code >= 0 && _code < 8) {
        return (this->mouseStateOld.rgbButtons[_code] & 0x80) && !(this->mouseState.rgbButtons[_code] & 0x80);
    }
	return false;
}

/**@brief マウスX座標の取得
 * @return int クライアント座標系でのX位置。対応していない場合は -1
 */
int DirectInputDevice::GetMouseX() const {
    return this->mousePoint.x;
}

/**@brief マウスY座標の取得
 * @return int クライアント座標系でのY位置。対応していない場合は -1
 */
int DirectInputDevice::GetMouseY() const {
    return this->mousePoint.y;
}

void DirectInputDevice::GetMouseDelta(int& _dx, int& _dy) const
{
    _dx = static_cast<int>(mouseState.lX);
    _dy = static_cast<int>(mouseState.lY);
}