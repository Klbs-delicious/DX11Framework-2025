/** @file   CameraLookComponent.h
 *  @brief  カメラの注視点制御を行うコンポーネント
 *  @date   2025/11/12
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Core/InputSystem.h"

 /** @class CameraLookComponent
  *  @brief カメラの注視点制御を行うコンポーネント
  *  @details - Componentを継承し、Updateフェーズで注視方向の補正や回転処理を行う
  */
class CameraLookComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	CameraLookComponent(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~CameraLookComponent() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/**@brief  入力感度を設定
	 * @param _sensitivity 感度
	 */
	void SetSensitivity(float _sensitivity) { this->sensitivity = _sensitivity; }

	/**@brief  回転補間速度を設定
	 * @param _speed 補間速度
	 */
	void SetSmoothSpeed(float _speed) { this->smoothSpeed = _speed; }

	/**@brief  注視対象のTransformを設定
	 * @param _target 注視対象のTransform
	 */
	void SetTarget(Transform* _target) { this->target = _target; }

	/**@brief  注視点からのオフセット位置を設定
	 * @param _offset オフセット位置
	 */
	void SetOffset(const DX::Vector3& _offset) { this->offset = _offset; }

private:
	InputSystem& inputSystem;	///< 入力処理の管理

	Transform* target;	///< 注視対象のTransform
	DX::Vector3 offset; ///< 注視点からのオフセット位置

	float yaw;           ///< 現在のYaw角
	float pitch;         ///< 現在のPitch角
	float sensitivity;   ///< 入力感度
	float smoothSpeed;   ///< 回転補間速度
};