/** @file   DirectInputDevice.cpp
 *  @brief  DirectInput によるキーボード・マウス入力デバイス
 *  @date   2025/09/14
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Core/DirectInputDevice.h"

#include <cstring>

//-----------------------------------------------------------------------------
// DirectInputDevice class
//-----------------------------------------------------------------------------

DirectInputDevice::DirectInputDevice() = default;

DirectInputDevice::~DirectInputDevice() = default;

bool DirectInputDevice::Initialize(HINSTANCE _hInst, HWND _hwnd)
{
	this->hwnd = _hwnd;

	if (FAILED(DirectInput8Create(_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&this->dinput), nullptr)))
	{
		return false;
	}

	if (FAILED(this->dinput->CreateDevice(GUID_SysKeyboard, &this->keyboard, nullptr)))
	{
		return false;
	}

	this->keyboard->SetDataFormat(&c_dfDIKeyboard);
	this->keyboard->SetCooperativeLevel(this->hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	this->keyboard->Acquire();

	if (FAILED(this->dinput->CreateDevice(GUID_SysMouse, &this->mouse, nullptr)))
	{
		return false;
	}

	this->mouse->SetDataFormat(&c_dfDIMouse2);
	this->mouse->SetCooperativeLevel(this->hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	this->mouse->Acquire();

	return true;
}

void DirectInputDevice::Dispose()
{
	if (this->keyboard)
	{
		this->keyboard->Unacquire();
		this->keyboard->Release();
		this->keyboard = nullptr;
	}

	if (this->mouse)
	{
		this->mouse->Unacquire();
		this->mouse->Release();
		this->mouse = nullptr;
	}

	if (this->dinput)
	{
		this->dinput->Release();
		this->dinput = nullptr;
	}
}

void DirectInputDevice::Update()
{
	std::memcpy(this->oldKeyBuffer, this->keyBuffer, sizeof(this->keyBuffer));
	this->mouseStateOld = this->mouseState;

	if (this->keyboard)
	{
		this->keyboard->Acquire();
		this->keyboard->GetDeviceState(sizeof(this->keyBuffer), this->keyBuffer);
	}

	if (this->mouse)
	{
		this->mouse->Acquire();
		this->mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &this->mouseState);
	}

	POINT screenPos{};
	if (GetCursorPos(&screenPos) && ScreenToClient(this->hwnd, &screenPos))
	{
		this->mousePoint = screenPos;
	}
	else
	{
		this->mousePoint = { -1, -1 };
	}
}

bool DirectInputDevice::IsPressed(int _code) const
{
	if (this->IsKeyboardCode(_code))
	{
		return (this->keyBuffer[_code] & 0x80) != 0;
	}

	if (this->IsMouseCode(_code))
	{
		const int mouseIndex = this->ToMouseIndex(_code);
		return (this->mouseState.rgbButtons[mouseIndex] & 0x80) != 0;
	}

	return false;
}

bool DirectInputDevice::IsTriggered(int _code) const
{
	if (this->IsKeyboardCode(_code))
	{
		return !(this->oldKeyBuffer[_code] & 0x80) && (this->keyBuffer[_code] & 0x80);
	}

	if (this->IsMouseCode(_code))
	{
		const int mouseIndex = this->ToMouseIndex(_code);
		return !(this->mouseStateOld.rgbButtons[mouseIndex] & 0x80) && (this->mouseState.rgbButtons[mouseIndex] & 0x80);
	}

	return false;
}

bool DirectInputDevice::IsReleased(int _code) const
{
	if (this->IsKeyboardCode(_code))
	{
		return (this->oldKeyBuffer[_code] & 0x80) && !(this->keyBuffer[_code] & 0x80);
	}

	if (this->IsMouseCode(_code))
	{
		const int mouseIndex = this->ToMouseIndex(_code);
		return (this->mouseStateOld.rgbButtons[mouseIndex] & 0x80) && !(this->mouseState.rgbButtons[mouseIndex] & 0x80);
	}

	return false;
}

int DirectInputDevice::GetMouseX() const
{
	return this->mousePoint.x;
}

int DirectInputDevice::GetMouseY() const
{
	return this->mousePoint.y;
}

void DirectInputDevice::GetMouseDelta(int& _dx, int& _dy) const
{
	_dx = static_cast<int>(this->mouseState.lX);
	_dy = static_cast<int>(this->mouseState.lY);
}

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------

bool DirectInputDevice::IsKeyboardCode(int _code) const
{
	return (_code >= KeyCodeMin) && (_code < KeyCodeMaxExclusive);
}

bool DirectInputDevice::IsMouseCode(int _code) const
{
	return (_code >= MouseCodeBase) && (_code < MouseCodeMaxExclusive);
}

int DirectInputDevice::ToMouseIndex(int _code) const
{
	return _code - MouseCodeBase;
}