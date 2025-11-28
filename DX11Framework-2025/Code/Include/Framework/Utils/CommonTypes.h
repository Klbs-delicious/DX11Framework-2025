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
	constexpr float TWO_PI = std::numbers::pi_v<float> *2.0f;

	/// @brief 度数をラジアンに変換
	constexpr float ToRadians(float degrees)
	{
		return degrees * (PI / 180.0f);
	}

	/// @brief ラジアンを度数に変換
	constexpr float ToDegrees(float radians)
	{
		return radians * (180.0f / PI);
	}

	/** @brief 左手座標系用のワールド行列を作成
	 *  @param const DX::Vector3& _position  平行移動
	 *  @param const DX::Vector3& _forward   前方向（Z+）
	 *  @param const DX::Vector3& _up        上方向（Y+）
	 *  @return DX::Matrix4x4 左手系ワールド行列
	 */
	inline DX::Matrix4x4 CreateWorldLH(const DX::Vector3& _position, const DX::Vector3& _forward, const DX::Vector3& _up)
	{
		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		XMVECTOR zaxis = XMVector3Normalize(XMLoadFloat3(&_forward)); // Negateなし！
		XMVECTOR yaxis = XMLoadFloat3(&_up);
		XMVECTOR xaxis = XMVector3Normalize(XMVector3Cross(yaxis, zaxis));
		yaxis = XMVector3Cross(zaxis, xaxis);

		Matrix result;
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._11), xaxis);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._21), yaxis);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._31), zaxis);
		result._14 = result._24 = result._34 = 0.0f;
		result._41 = _position.x;
		result._42 = _position.y;
		result._43 = _position.z;
		result._44 = 1.0f;

		return result;
	}
}
