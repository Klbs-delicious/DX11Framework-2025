/** @file   CommonTypes.cpp
 *  @date   2025/09/19
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Utils/CommonTypes.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

//-----------------------------------------------------------------------------
// Namespace : DX
//-----------------------------------------------------------------------------
namespace DX
{
	DX::Matrix4x4 CreateWorldLH(const DX::Vector3& _position, const DX::Vector3& _forward, const DX::Vector3& _up)
	{
		const XMFLOAT3 forwardFloat = DX::ToXMFLOAT3(_forward);
		const XMFLOAT3 upFloat = DX::ToXMFLOAT3(_up);

		XMVECTOR zAxis = XMVector3Normalize(XMLoadFloat3(&forwardFloat));
		XMVECTOR yAxis = XMLoadFloat3(&upFloat);
		XMVECTOR xAxis = XMVector3Normalize(XMVector3Cross(yAxis, zAxis));
		yAxis = XMVector3Cross(zAxis, xAxis);

		DX::Matrix4x4 result = DX::Matrix4x4::Identity;

		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._11), xAxis);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._21), yAxis);
		XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result._31), zAxis);

		result._14 = 0.0f;
		result._24 = 0.0f;
		result._34 = 0.0f;

		result._41 = _position.x;
		result._42 = _position.y;
		result._43 = _position.z;
		result._44 = 1.0f;

		return result;
	}

	XMMATRIX LoadXMMATRIX(const DX::Matrix4x4& _m)
	{
		const XMFLOAT4X4 floatMatrix = DX::ToXMFLOAT4X4(_m);
		return XMLoadFloat4x4(&floatMatrix);
	}

	DX::Matrix4x4 StoreDXMatrix(const XMMATRIX& _m)
	{
		XMFLOAT4X4 floatMatrix{};
		XMStoreFloat4x4(&floatMatrix, _m);
		return DX::ToDXMatrix4x4(floatMatrix);
	}

	DX::Quaternion SlerpQuaternionSimple(const DX::Quaternion& _from, const DX::Quaternion& _to, float _t)
	{
		DX::Quaternion from = DX::NormalizeQuaternionSafe(_from);
		DX::Quaternion to = DX::NormalizeQuaternionSafe(_to);

		float dot = DX::DotQuaternion(from, to);

		if (dot < 0.0f)
		{
			to = DX::NegateQuaternion(to);
			dot = -dot;
		}

		if (dot > 0.9995f)
		{
			return DX::NormalizeQuaternionSafe(DX::LerpQuaternion(from, to, _t));
		}

		dot = std::clamp(dot, -1.0f, 1.0f);

		const float theta = std::acos(dot);
		const float sinTheta = std::sin(theta);

		if (std::abs(sinTheta) <= 1.0e-6f)
		{
			return DX::NormalizeQuaternionSafe(DX::LerpQuaternion(from, to, _t));
		}

		const float w0 = std::sin((1.0f - _t) * theta) / sinTheta;
		const float w1 = std::sin(_t * theta) / sinTheta;

		DX::Quaternion result(
			(from.x * w0) + (to.x * w1),
			(from.y * w0) + (to.y * w1),
			(from.z * w0) + (to.z * w1),
			(from.w * w0) + (to.w * w1));

		return DX::NormalizeQuaternionSafe(result);
	}

	DX::Matrix4x4 TransposeMatrix(const DX::Matrix4x4& _m)
	{
		const XMFLOAT4X4 sourceFloat = DX::ToXMFLOAT4X4(_m);
		const XMMATRIX sourceMatrix = XMLoadFloat4x4(&sourceFloat);
		const XMMATRIX transposedMatrix = XMMatrixTranspose(sourceMatrix);
		return DX::StoreDXMatrix(transposedMatrix);
	}

	DX::Matrix4x4 InverseMatrix(const DX::Matrix4x4& _m)
	{
		const DirectX::XMFLOAT4X4 sourceFloat = ToXMFLOAT4X4(_m);
		const DirectX::XMMATRIX sourceMatrix = DirectX::XMLoadFloat4x4(&sourceFloat);

		DirectX::XMVECTOR det{};
		const DirectX::XMMATRIX invMatrix = DirectX::XMMatrixInverse(&det, sourceMatrix);

		return StoreDXMatrix(invMatrix);
	}
}
