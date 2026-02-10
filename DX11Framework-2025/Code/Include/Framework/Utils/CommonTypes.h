/** @file   CommonTypes.h
 *  @date   2025/09/19
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include <wrl/client.h>
#include <cstdint>
#include <numbers>
#include <cmath>
#include <algorithm>
#include <cstring>

#include <DirectXMath.h>
#include "SimpleMath.h"

//-----------------------------------------------------------------------------
// Namespace : DX
//-----------------------------------------------------------------------------
/** @namespace DX
 *  @brief DirectX の型エイリアスと、補助関数をまとめる
 */
namespace DX
{
	//-------------------------------------------------------------------------
	// 型エイリアス
	//-------------------------------------------------------------------------
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector2 = DirectX::SimpleMath::Vector2;

	using Matrix4x4 = DirectX::SimpleMath::Matrix;
	using Color = DirectX::SimpleMath::Color;
	using Quaternion = DirectX::SimpleMath::Quaternion;

	using Microsoft::WRL::ComPtr;

	//-------------------------------------------------------------------------
	// 定数
	//-------------------------------------------------------------------------
	constexpr float PI = std::numbers::pi_v<float>;
	constexpr float TWO_PI = std::numbers::pi_v<float> *2.0f;

	//-------------------------------------------------------------------------
	// 角度変換関数
	//-------------------------------------------------------------------------
	/// @brief 度数をラジアンに変換
	constexpr float ToRadians(float _degrees)
	{
		return _degrees * (PI / 180.0f);
	}

	/// @brief ラジアンを度数に変換
	constexpr float ToDegrees(float _radians)
	{
		return _radians * (180.0f / PI);
	}

	//-------------------------------------------------------------------------
	// 行列作成関数
	//-------------------------------------------------------------------------
	/** @brief 左手座標系用のワールド行列を作成
	 *  @param _position 平行移動
	 *  @param _forward  前方向（Z+）
	 *  @param _up       上方向（Y+）
	 *  @return 左手系ワールド行列
	 */
	DX::Matrix4x4 CreateWorldLH(const DX::Vector3& _position, const DX::Vector3& _forward, const DX::Vector3& _up);

	//-------------------------------------------------------------------------
	// DX型 → XM系型 変換関数（軽いのでヘッダ inline）
	//-------------------------------------------------------------------------
	/** @brief DX::Vector3 を XMFLOAT3 に変換
	 *  @param _v 入力
	 *  @return 変換結果
	 */
	inline DirectX::XMFLOAT3 ToXMFLOAT3(const DX::Vector3& _v)
	{
		return DirectX::XMFLOAT3(_v.x, _v.y, _v.z);
	}

	/** @brief XMFLOAT3 を DX::Vector3 に変換
	 *  @param _v 入力
	 *  @return 変換結果
	 */
	inline DX::Vector3 ToDXVector3(const DirectX::XMFLOAT3& _v)
	{
		return DX::Vector3(_v.x, _v.y, _v.z);
	}

	/** @brief DX::Quaternion を XMFLOAT4 に変換
	 *  @param _q 入力
	 *  @return 変換結果
	 */
	inline DirectX::XMFLOAT4 ToXMFLOAT4(const DX::Quaternion& _q)
	{
		return DirectX::XMFLOAT4(_q.x, _q.y, _q.z, _q.w);
	}

	/** @brief XMFLOAT4 を DX::Quaternion に変換
	 *  @param _q 入力
	 *  @return 変換結果
	 */
	inline DX::Quaternion ToDXQuaternion(const DirectX::XMFLOAT4& _q)
	{
		return DX::Quaternion(_q.x, _q.y, _q.z, _q.w);
	}

	/** @brief DX::Matrix4x4 を XMFLOAT4X4 に変換
	 *  @param _m 入力
	 *  @return 変換結果
	 */
	inline DirectX::XMFLOAT4X4 ToXMFLOAT4X4(const DX::Matrix4x4& _m)
	{
		DirectX::XMFLOAT4X4 out{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(DirectX::XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&out, &_m, sizeof(DirectX::XMFLOAT4X4));
		return out;
	}

	/** @brief XMFLOAT4X4 を DX::Matrix4x4 に変換
	 *  @param _m 入力
	 *  @return 変換結果
	 */
	inline DX::Matrix4x4 ToDXMatrix4x4(const DirectX::XMFLOAT4X4& _m)
	{
		DX::Matrix4x4 out{};
		static_assert(sizeof(DX::Matrix4x4) == sizeof(DirectX::XMFLOAT4X4), "DX::Matrix4x4 と XMFLOAT4X4 のサイズが不一致です。");
		std::memcpy(&out, &_m, sizeof(DirectX::XMFLOAT4X4));
		return out;
	}

	//-------------------------------------------------------------------------
	// XMMATRIX ロード／ストア
	//-------------------------------------------------------------------------
	/** @brief DX::Matrix4x4 を XMMATRIX に読み込む
	 *  @param _m 入力
	 *  @return XMMATRIX
	 */
	DirectX::XMMATRIX LoadXMMATRIX(const DX::Matrix4x4& _m);

	/** @brief XMMATRIX を DX::Matrix4x4 に書き戻す
	 *  @param _m 入力
	 *  @return DX::Matrix4x4
	 */
	DX::Matrix4x4 StoreDXMatrix(const DirectX::XMMATRIX& _m);

	//-------------------------------------------------------------------------
	// クォータニオン補助関数
	//-------------------------------------------------------------------------
	/** @brief 四元数が有限値（NaN / Inf を含まない）かどうかを判定する
	 *  @param _q 判定対象の四元数
	 *  @return 全成分が有限値であれば true、そうでなければ false
	 */
	inline bool IsFiniteQuaternion(const DirectX::SimpleMath::Quaternion& _q)
	{
		return
			std::isfinite(_q.x) &&
			std::isfinite(_q.y) &&
			std::isfinite(_q.z) &&
			std::isfinite(_q.w);
	}

	/** @brief 四元数が正規化されているかどうかを判定する
	 *  @details 有限値であることが前提。NaN / Inf を含む場合は false となる
	 *  @param _q 判定対象の四元数
	 *  @param _epsilon |length^2 - 1| の許容誤差
	 *  @return 正規化されていれば true、そうでなければ false
	 */
	inline bool IsNormalizedQuaternion(const DirectX::SimpleMath::Quaternion& _q, float _epsilon = 1.0e-3f)
	{
		float lenSq = _q.LengthSquared();
		return std::fabs(lenSq - 1.0f) <= _epsilon;
	}

	/** @brief 四元数を安全に正規化する
	 *  @details
	 *  - NaN / Inf を含む場合は Identity を返す
	 *  - 長さが極端に小さい場合も Identity を返す
	 *  @param _q 正規化対象の四元数
	 *  @param _epsilon 長さ判定用の下限値
	 *  @return 正規化済み四元数、または安全な Identity
	 */
	inline DirectX::SimpleMath::Quaternion SafeNormalizeQuaternion(const DirectX::SimpleMath::Quaternion& _q, float _epsilon = 1.0e-6f)
	{
		if (!IsFiniteQuaternion(_q))
		{
			return DirectX::SimpleMath::Quaternion::Identity;
		}

		float lenSq = _q.LengthSquared();
		if (lenSq <= _epsilon)
		{
			return DirectX::SimpleMath::Quaternion::Identity;
		}

		DirectX::SimpleMath::Quaternion out = _q;
		out.Normalize();
		return out;
	}

	/** @brief 四元数の内積
	 *  @param _a 四元数A
	 *  @param _b 四元数B
	 *  @return 内積
	 */
	inline float DotQuaternion(const DX::Quaternion& _a, const DX::Quaternion& _b)
	{
		return (_a.x * _b.x) + (_a.y * _b.y) + (_a.z * _b.z) + (_a.w * _b.w);
	}

	/** @brief 四元数の正規化（安全版）
	 *  @param _q 入力四元数
	 *  @return 正規化された四元数（長さ0の場合は単位四元数を返す）
	 */
	inline DX::Quaternion NormalizeQuaternionSafe(const DX::Quaternion& _q)
	{
		DX::Quaternion q = _q;

		const float lengthSq = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
		if (!std::isfinite(lengthSq) || lengthSq <= 1.0e-12f)
		{
			return DX::Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}

		const float invLength = 1.0f / std::sqrt(lengthSq);
		q.x *= invLength;
		q.y *= invLength;
		q.z *= invLength;
		q.w *= invLength;
		return q;
	}

	/** @brief 四元数の符号反転
	 *  @param _q 入力四元数
	 *  @return 符号反転した四元数
	 */
	inline DX::Quaternion NegateQuaternion(const DX::Quaternion& _q)
	{
		return DX::Quaternion(-_q.x, -_q.y, -_q.z, -_q.w);
	}

	/** @brief 四元数の線形補間
	 *  @param _from 始点
	 *  @param _to 終点
	 *  @param _t 0 から 1
	 *  @return 補間結果
	 */
	inline DX::Quaternion LerpQuaternion(const DX::Quaternion& _from, const DX::Quaternion& _to, float _t)
	{
		return DX::Quaternion(
			_from.x + (_to.x - _from.x) * _t,
			_from.y + (_to.y - _from.y) * _t,
			_from.z + (_to.z - _from.z) * _t,
			_from.w + (_to.w - _from.w) * _t);
	}

	/** @brief 四元数の球面補間（簡易）
	 *  @param _from 始点
	 *  @param _to 終点
	 *  @param _t 0 から 1
	 *  @return 補間結果（正規化済み）
	 */
	DX::Quaternion SlerpQuaternionSimple(const DX::Quaternion& _from, const DX::Quaternion& _to, float _t);

	//-----------------------------------------------------------------------------
	// Vector3 utilities
	//-----------------------------------------------------------------------------

	/** @brief ベクトルが有限値（NaN / Inf を含まない）かどうかを判定する
	 *  @param _v 判定対象のベクトル
	 *  @return 全成分が有限値であれば true、そうでなければ false
	 */
	inline bool IsFiniteVector3(const DirectX::SimpleMath::Vector3& _v)
	{
		return
			std::isfinite(_v.x) &&
			std::isfinite(_v.y) &&
			std::isfinite(_v.z);
	}

	/** @brief ベクトルを安全に正規化する
	 *  @details
	 *  - NaN / Inf を含む場合は Zero を返す
	 *  - 長さが極端に小さい場合も Zero を返す
	 *  @param _v 正規化対象のベクトル
	 *  @param _epsilon 長さ判定用の下限値
	 *  @return 正規化済みベクトル、または安全な Zero
	 */
	inline DirectX::SimpleMath::Vector3 SafeNormalizeVector3(const DirectX::SimpleMath::Vector3& _v, float _epsilon = 1.0e-6f)
	{
		if (!IsFiniteVector3(_v))
		{
			return DirectX::SimpleMath::Vector3::Zero;
		}

		float lenSq = _v.LengthSquared();
		if (lenSq <= _epsilon)
		{
			return DirectX::SimpleMath::Vector3::Zero;
		}

		DirectX::SimpleMath::Vector3 out = _v;
		out.Normalize();
		return out;
	}

	//-------------------------------------------------------------------------
	// 行列補助関数
	//-------------------------------------------------------------------------
	/** @brief DX::Matrix4x4 の転置
	 *  @param _m 入力行列
	 *  @return 転置行列
	 */
	DX::Matrix4x4 TransposeMatrix(const DX::Matrix4x4& _m);

	/** @brief DX::Matrix4x4 の逆行列
	*  @param _m 入力行列
	*  @return 逆行列（行列が特異な場合は結果が不安定になる可能性がある）
	*/
	DX::Matrix4x4 InverseMatrix(const DX::Matrix4x4& _m);
}