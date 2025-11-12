/**	@file	InputSystem.cpp
*	@date	2025/09/14
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Core/InputSystem.h"

#include <typeinfo>

//-----------------------------------------------------------------------------
// InputSystem Class
//-----------------------------------------------------------------------------

InputSystem::InputSystem() :devices(std::vector<std::unique_ptr<IInputDevice>>()), keyMap(std::unordered_map<std::string, int>()) {}
InputSystem::~InputSystem() { this->Dispose(); }

/// @brief	リソースの解放処理
void InputSystem::Dispose()
{
    // 入力デバイスのリソースを解放
    if (!this->devices.empty()) {
        for (const auto& device : this->devices) {
            device->Dispose();
        }
        this->devices.clear();
    }
}

/**	@brief	デバイスの登録
*  @param std::unique_ptr<IInputDevice>	_device 登録するデバイス（std::unique_ptr による所有権移動）
 */
void InputSystem::RegisterDevice(std::unique_ptr<IInputDevice> _device)
{
    for (const auto& dev : this->devices)
    {
        // すでに同じ型のデバイスが登録されている場合登録しない
        if (typeid(*dev) == typeid(*_device)) { return; }
    }
    this->devices.push_back(std::move(_device));
}

/**	@brief	キーバインドの登録
 *	@param	const std::string& _action アクション名
 *	@param	int _keyCode	キーボタンの入力コード
 */
void InputSystem::RegisterKeyBinding(const std::string& _action, int _keyCode)
{
    // 既に登録済なら無視する
    if (this->keyMap.contains(_action)) { return; }

    this->keyMap[_action] = _keyCode;
}

/// @brief	入力デバイスの更新
void InputSystem::Update()
{
    for (const auto& device : this->devices)
    {
        device->Update();
    }
}

/**	@brief	アクションに対応したキー、ボタンが押された状態か取得する
 *	@param	const std::string& _action	キーアクション名（例: "Jump"）
 *  @return bool いずれかのデバイスで押されていれば true
 */
bool InputSystem::IsActionPressed(const std::string& _action) const
{
    // キーマップに対応した入力コードの取得
    auto it = this->keyMap.find(_action);
    if (it == this->keyMap.end()) { return false; }   // [TODO] 未定義のアクションの場合Logを出す
    
    int code = it->second;

    for (const auto& device :this->devices)
    {
        // 入力があった
        if (device->IsPressed(code)) { return true; }
    }

    // 入力が無かった
    return false;
}

/**	@brief	アクションに対応したキー、ボタンがトリガー状態か取得する
 *	@param	const std::string& _action	キーアクション
 * @return bool 前フレームから押された瞬間であれば true
 */
bool InputSystem::IsActionTriggered(const std::string& _action) const
{
    // キーマップに対応した入力コードの取得
    auto it = this->keyMap.find(_action);
    if (it == this->keyMap.end()) { return false; }   // [TODO] 未定義のアクションの場合Logを出す

    int code = it->second;

    for (const auto& device : this->devices)
    {
        // 入力があった
        if (device->IsTriggered(code)) { return true; }
    }

    // 入力が無かった
    return false;
}

/** @brief マウスの現在座標を取得
 *  @param _x 取得したX座標を格納
 *  @param _y 取得したY座標を格納
 *  @return 対応デバイスから座標を取得できた場合は true、それ以外は false
 */
bool InputSystem::GetMousePosition(int& _x, int& _y) const
{
    for (const auto& device : this->devices)
    {
        int x = device->GetMouseX();
        int y = device->GetMouseY();

		// マウスに対応していない場合は無効値を返す
        if (x >= 0 && y >= 0)
        {
            _x = x;
            _y = y;
            return true;
        }
    }

    _x = _y = -1;
    return false;
}

/** @brief マウスの移動量（Δ）を取得
 *  @param _dx X方向の変化量を格納
 *  @param _dy Y方向の変化量を格納
 *  @return いずれかのデバイスで移動を検出した場合は true
 */
bool InputSystem::GetMouseDelta(int& _dx, int& _dy) const
{
    _dx = _dy = 0;

    for (const auto& device : this->devices)
    {
        int dx = 0, dy = 0;
        device->GetMouseDelta(dx, dy);

		// 移動があった
        if (dx != 0 || dy != 0)
        {
            _dx = dx;
            _dy = dy;
            return true;
        }
    }

    return false;
}