/** @file   FollowCamera.h
 *  @brief  ターゲットを追従するカメラコンポーネント
 *  @date   2025/11/12
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include"Include/Framework/Entities/GameObject.h"

 /** @class FollowCamera
  *  @brief ターゲットを追従するカメラコンポーネント
  *  @details - Componentを継承し、Updateフェーズで追従処理を行う
  */
class FollowCamera : public Component, public IFixedUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	FollowCamera(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~FollowCamera() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 固定更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void FixedUpdate(float _deltaTime) override;

	void SetPivot(Transform* _pivot) { this->pivot = _pivot; }

	/** @brief 追従対象のオブジェクトを設定する
	 *  @param Transform* _target 追従対象のオブジェクト
	 */
	void SetTarget(Transform* _target) { this->target = _target; }

	/// @brief 追従対象のオブジェクトをクリアする
	void ClearTarget() { this->target = nullptr; }

	/**@brief 距離と高さを設定
	 * @param _distance 
	 * @param _height 
	 */
	void SetOffset(float _distance, float _height)
	{
		this->distance = _distance;
		this->height = _height;
	}

	/**@brief 追従補間速度を設定
	 * @param _speed 
	 */
	void SetSmoothSpeed(float _speed) { this->smoothSpeed = _speed; }

private:
	Transform* pivot;       ///< 追従基準となるTransform（CameraPivot）
	Transform* target;      ///< 注視対象（Playerや敵など）

	float distance;         ///< Pivotからの後方距離
	float height;           ///< Pivotからの高さ
	float smoothSpeed;      ///< 追従補間速度
};
