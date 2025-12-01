/** @file   StagedTransform.h
 *  @brief  ゲームロジック専用のワールド座標を保持する軽量構造体
 *  @date   2025/11/27
 */
#pragma once
#include "Include/Framework/Utils/CommonTypes.h"

 /** @class StagedTransform
  *  @brief 階層やローカル概念を持たない “ロジック専用の絶対座標”
  */
class StagedTransform
{
public:
    DX::Vector3    position;   ///< ワールド位置（ロジック専用）
    DX::Quaternion rotation;   ///< ワールド回転（ロジック専用）
    DX::Vector3    scale;      ///< ワールドスケール（必要なら）
public:

	/// @brief デフォルトコンストラクタ（位置原点、回転なし、スケール1）
    StagedTransform()
        : position(DX::Vector3::Zero)
        , rotation(DX::Quaternion::Identity)
        , scale(DX::Vector3::One)
    {}

    /**@brief 位置と回転とスケールを指定する
     * @param _pos 
     * @param _rot 
     * @param _scale 
     */
    StagedTransform(const DX::Vector3& _pos, const DX::Quaternion& _rot, const DX::Vector3& _scale = DX::Vector3::One)
        : position(_pos)
        , rotation(_rot)
        , scale(_scale)
    {}

    /**@brief 行列を生成（必要な時だけ使う）
	 * @return DX::Matrix4x4 変換行列
     */
    [[nodiscard]] DX::Matrix4x4 ToMatrix() const
    {
        return
            DX::Matrix4x4::CreateScale(scale) *
            DX::Matrix4x4::CreateFromQuaternion(rotation) *
            DX::Matrix4x4::CreateTranslation(position);
    }

    /**@brief 行列から情報を抽出して設定（必要な時だけ使う）
     * @param _mat 変換行列
	 */
    void FromMatrix(const DX::Matrix4x4& _mat)
    {
        this->position = _mat.Translation();
        this->rotation = DX::Quaternion::CreateFromRotationMatrix(_mat);
        // scaleも一応行う
		this->scale.x = DX::Vector3(_mat._11, _mat._12, _mat._13).Length();
		this->scale.y = DX::Vector3(_mat._21, _mat._22, _mat._23).Length();
		this->scale.z = DX::Vector3(_mat._31, _mat._32, _mat._33).Length();
    }
};
