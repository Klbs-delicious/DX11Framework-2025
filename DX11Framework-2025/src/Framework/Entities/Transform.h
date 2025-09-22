/**@file   Transform.h
 * @date   2025/09/19
 */
#pragma once

#include"Framework/Entities/Component.h"
#include"Framework/Utils/CommonTypes.h"

#include<vector>

/**	@class	Transform
 *	@brief	座標系を管理するコンポーネント
 */
class Transform :public Component
{
public:

	/** @brief  コンストラクタ
	 *  @param GameObject* _owner	このコンポーネントがアタッチされるオブジェクト
	 *  @param bool _active	コンポーネントの有効/無効
	 */
	Transform(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	virtual ~Transform()final;

	/// @brief 初期化処理
	void Initialize()final;

	/// @brief 終了処理
	void Dispose()final;

	/**	@brief ワールド変換行列を更新
	 *	@details 親Transformがいる場合は親のワールド変換行列も考慮して更新する
	 */
	void UpdateWorldMatrix();

	/// -------------------------------------------------------------

	/**@brief ワールド空間の座標を設定
	 * @param const DX::Vector3& _position 
	 */
	void SetWorldPosition(const DX::Vector3& _position);

	/**@brief ワールド空間の座標を取得
	 * @return DX::Vector3	ワールド空間の座標
	 */
	DX::Vector3 GetWorldPosition()const;

	/**@brief ワールド空間の回転を設定
	 * @param const DX::Quaternion& _rotation 
	 */
	void SetWorldRotation(const DX::Quaternion& _rotation);

	/**@brief ワールド空間の回転を取得
	 * @return DX::Quaternion	ワールド空間の回転
	 */
	DX::Quaternion GetWorldRotation()const;

	/**@brief ワールド空間のスケールを設定
	 * @param const DX::Vector3& _scale 
	 */	
	void SetWorldScale(const DX::Vector3& _scale);

	/**@brief ワールド空間のスケールを取得
	 * @return DX::Vector3	ワールド空間のスケール
	 */
	DX::Vector3 GetWorldScale()const;

	/// -------------------------------------------------------------

	/**@brief ローカル空間の座標を設定
	 * @param const DX::Vector3& _localPosition 
	 */
	void SetLocalPosition(const DX::Vector3& _localPosition);

	/**@brief ローカル空間の座標を取得
	 * @return DX::Vector3	ローカル空間の座標
	 */
	DX::Vector3 GetLocalPosition()const;

	/**@brief ローカル空間の回転を設定
	 * @param const DX::Quaternion& _localRotation 
	 */
	void SetLocalRotation(const DX::Quaternion& _localRotation);

	/**@brief ローカル空間の回転を取得
	 * @return DX::Quaternion	ローカル空間の回転
	 */
	DX::Quaternion GetLocalRotation()const;

	/**@brief ローカル空間のスケールを設定
	 * @param const DX::Vector3& _localScale 
	 */
	void SetLocalScale(const DX::Vector3& _localScale);

	/**@brief ローカル空間のスケールを取得
	 * @return DX::Vector3	ローカル空間のスケール
	 */
	DX::Vector3 GetLocalScale()const;

	/// -------------------------------------------------------------

	/**@brief このコンポーネントがアタッチされているオブジェクトを取得
	 * @return GameObject* このコンポーネントがアタッチされているオブジェクト
	 */
	GameObject* GetOwner()const { return this->owner; }

	/**@brief 親Transformを設定
	 * @param Transform* _parent 親Transform
	 */
	void SetParent(Transform* _parent);

	/**@brief 親Transformを取得
	 * @return Transform* 親Transform
	 */
	Transform* GetParent()const { return this->parent; }

	/**@brief 子Transformを追加
	 * @param Transform* _child 追加する子Transform
	 */
	void AddChild(Transform* _child);

	/**@brief 子Transformを削除
	 * @param Transform* _child 削除する子Transform
	 */
	void RemoveChild(Transform* _child);

	/**@brief 子Transformのリストを取得
	 * @return const std::vector<Transform*>& 子Transformのリスト
	 */
	const std::vector<Transform*>& GetChildren()const { return this->children; }

	/// -------------------------------------------------------------

	/**@brief ローカル空間の座標をワールド空間に変換する
	 * @param const DX::Vector3& _localPoint
	 * @return DX::Vector3 ワールド空間座標
	 */
	DX::Vector3 TransformPoint(const DX::Vector3& _localPoint) const;

	/**@brief ワールド空間の座標をローカル空間に変換する
	 * @param const DX::Vector3& _worldPoint
	 * @return DX::Vector3	ローカル空間座標
	 */
	DX::Vector3 InverseTransformPoint(const DX::Vector3& _worldPoint) const;

	/**@brief ワールド空間の座標をローカル空間に変換する
	 * @param const DX::Vector3& worldPos ワールド空間の座標
	 * @return DX::Vector3 ローカル空間の座標
	 */
	DX::Vector3 WorldToLocalPosition(const DX::Vector3& worldPos) const;

	/**@brief ワールド空間の回転をローカル空間に変換する
	 * @param const DX::Quaternion& worldRot ワールド空間の回転
	 * @return DX::Quaternion ローカル空間の回転
	 */
	DX::Quaternion WorldToLocalRotation(const DX::Quaternion& worldRot) const;

	/**@brief ワールド空間のスケールをローカル空間に変換する
	 * @param const DX::Vector3& worldScale ワールド空間のスケール
	 * @return DX::Vector3 ローカル空間のスケール
	 */
	DX::Vector3 WorldToLocalScale(const DX::Vector3& worldScale) const;

	/**@brief ローカル空間 → ワールド空間への変換行列を取得する
	 * @return const DX::Matrix4x4&	ワールド空間への変換行列
	 */
	const DX::Matrix4x4& GetLocalToWorldMatrix() const;

	/**@brief ワールド空間 → ローカル空間への変換行列を取得する
	 * @return const DX::Matrix4x4&	ローカル空間への変換行列
	 */
	DX::Matrix4x4 GetWorldToLocalMatrix() const;

	/**@brief ワールド変換行列を取得
	 * @return const DX::Matrix4x4&	ワールド変換行列
	 */
	const DX::Matrix4x4& GetWorldMatrix() const;

	/// -------------------------------------------------------------
	/**@brief	前方向ベクトルを取得
	 * @return DX::Vector3	前方向ベクトル
	 */
	DX::Vector3 Forward() const;
	/**@brief	上方向ベクトルを取得
	 * @return DX::Vector3	上方向ベクトル
	 */
	DX::Vector3 Up() const;
	/**@brief	右方向ベクトルを取得
	 * @return DX::Vector3	右方向ベクトル
	 */
	DX::Vector3 Right() const;

	/**@brief	指定した位置を向くように回転を設定する
	 * @param const DX::Vector3& _target	注視点のワールド座標
	 * @param const DX::Vector3& _up		上方向ベクトル（デフォルトはY軸）
	 */
	void LookAt(const DX::Vector3& _target, const DX::Vector3& _up = DX::Vector3::UnitY);

	/**@brief	指定した軸の周りに回転する
	 * @param const DX::Vector3& _center	回転の中心点のワールド座標
	 * @param const DX::Vector3& _axis		回転軸（ワールド空間）
	 * @param float _angle				回転角（ラジアン）
	 */
	void RotateAround(const DX::Vector3& _center, const DX::Vector3& _axis, float _angle);

	/**@brief	クォータニオンをオイラー角に変換
	 * @param const DX::Quaternion& _quat	変換するクォータニオン
	 * @return DX::Vector3	オイラー角
	 */
	DX::Vector3 QuaternionToEuler(const DX::Quaternion& _quat) const;

private:
	bool isDirty;						///< 再計算するかフラグ
	Transform* parent;					///< 親Transform
	std::vector<Transform*> children;	///< 子Transformのリスト

	DX::Vector3		position;	///< ワールド空間での座標
	DX::Quaternion	rotation;	///< ワールド空間での回転
	DX::Vector3		scale;		///< ワールド空間でのスケール
	
	DX::Vector3		localPosition;	///< 親Transformからの相対座標
	DX::Quaternion	localRotation;	///< 親Transformからの相対回転
	DX::Vector3		localScale;		///< 親Transformからの相対スケール

	DX::Matrix4x4	worldMatrix;		///< ワールド変換行列
};