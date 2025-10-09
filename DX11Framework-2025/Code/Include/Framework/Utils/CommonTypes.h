/**@file   CommonTypes.h
 * @date   2025/09/19
 */
#pragma once
#include	<wrl/client.h>
#include	<cstdint>
#include	<numbers>
#include	"SimpleMath.h"

/**@namespace	DX
 * @brief		DirectXの型エイリアスを簡略化した定義を纏めている
 */
namespace DX
{
	// 基本型のエイリアス
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector2 = DirectX::SimpleMath::Vector2;

	using Matrix4x4 = DirectX::SimpleMath::Matrix;

	using Color = DirectX::SimpleMath::Color;

	using Quaternion = DirectX::SimpleMath::Quaternion;

	using Microsoft::WRL::ComPtr;

	constexpr float PI = std::numbers::pi_v<float>;
}