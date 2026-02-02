/**@file   CommonTypes.h
 * @date   2025/09/19
 */
#pragma once
#include	<wrl/client.h>
#include	<cstdint>
#include	<numbers>
#include	<cmath>
#include	<algorithm>
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

//-----------------------------------------------------------------------------
// DX::Quaternion 補間（分かりやすさ優先）
//-----------------------------------------------------------------------------
namespace DX
{
	/** @brief クォータニオンの内積
	 *  @param _a クォータニオンA
	 *  @param _b クォータニオンB
	 *  @return 内積
	 */
	static float DotQuat(const DX::Quaternion& _a, const DX::Quaternion& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z) + (_a.w * _b.w);
	}

	/** @brief クォータニオンの正規化（安全版）
	 *  @param _q 入力クォータニオン
	 *  @return 正規化されたクォータニオン（長さ0の場合は単位クォータニオンを返す）
	 */
	static DX::Quaternion NormalizeQuatSafe(const DX::Quaternion& _q)
	{
		DX::Quaternion q = _q;

		const float lenSq = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
		if (!std::isfinite(lenSq) || lenSq <= 1.0e-12f)
		{
			return DX::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}

		const float invLen = 1.0f / std::sqrt(lenSq);
		q.x *= invLen;
		q.y *= invLen;
		q.z *= invLen;
		q.w *= invLen;
		return q;
	}

	/** @brief クォータニオンの符号反転
	 *  @param _q 入力クォータニオン
	 *  @return 符号反転したクォータニオン
	 */
	static DX::Quaternion NegateQuat(const DX::Quaternion& _q)
	{
		return DX::Quaternion(-_q.x, -_q.y, -_q.z, -_q.w);
	}

	/** @brief 直線補間
	 *  @param _a 開始回転
	 *  @param _b 終了回転
	 *  @param _t 0～1
	 *  @return 補間された回転
	 */
	static DX::Quaternion LerpQuat(const DX::Quaternion& _a, const DX::Quaternion& _b, float _t)
	{
		return DX::Quaternion(
			_a.x + (_b.x - _a.x) * _t,
			_a.y + (_b.y - _a.y) * _t,
			_a.z + (_b.z - _a.z) * _t,
			_a.w + (_b.w - _a.w) * _t);
	}

	/** @brief 球面補間
	 *  @param _q0 開始回転
	 *  @param _q1 終了回転
	 *  @param _t  0～1
	 *  @return 補間された回転（正規化済み）
	 */
	static DX::Quaternion SlerpQuatSimple(const DX::Quaternion& _q0, const DX::Quaternion& _q1, float _t)
	{
		// 正規化を行う
		DX::Quaternion q0 = NormalizeQuatSafe(_q0);
		DX::Quaternion q1 = NormalizeQuatSafe(_q1);

		// 内積を計算する
		float dot = DotQuat(q0, q1);

		// 符号反転が混ざると補間がゼロ付近を通って潰れやすいので、
		// 同じ半球に揃える
		if (dot < 0.0f)
		{
			q1 = NegateQuat(q1);
			dot = -dot;
		}

		// ほぼ同じ回転なら直線補間で十分とみなす
		if (dot > 0.9995f)
		{
			return NormalizeQuatSafe(LerpQuat(q0, q1, _t));
		}

		// 球面補間を行う
		dot = std::clamp(dot, -1.0f, 1.0f);

		const float theta = std::acos(dot);	
		const float sinTheta = std::sin(theta);

		if (std::abs(sinTheta) <= 1.0e-6f)
		{
			// sinTheta が0に近い場合は直線補間で代替する
			return NormalizeQuatSafe(LerpQuat(q0, q1, _t));
		}

		// 補間係数を計算する
		const float w0 = std::sin((1.0f - _t) * theta) / sinTheta;
		const float w1 = std::sin(_t * theta) / sinTheta;

		DX::Quaternion out(
			(q0.x * w0) + (q1.x * w1),
			(q0.y * w0) + (q1.y * w1),
			(q0.z * w0) + (q1.z * w1),
			(q0.w * w0) + (q1.w * w1));

		// 正規化して返す
		return NormalizeQuatSafe(out);
	}
}