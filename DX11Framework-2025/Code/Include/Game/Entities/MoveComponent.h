/** @file   MoveComponent.h
 *  @brief  物理移動（速度・回転）を行う共通移動コンポーネント
 *  @date   2026/02/16
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/Rigidbody3D.h"
#include "Include/Framework/Utils/CommonTypes.h"

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
namespace Framework::Physics
{
	class Rigidbody3D;
}

//-----------------------------------------------------------------------------
// MoveComponent class
//-----------------------------------------------------------------------------

/** @class  MoveComponent
 *  @brief  移動方向に向きを合わせつつ、速度を設定する
 *  @details
 *          - コントローラー等が「毎フレーム」移動指示を与える想定
 *          - そのフレームで指示が無ければ停止する（意図を保持しない）
 */
class MoveComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	MoveComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~MoveComponent() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/** @brief 更新処理（そのフレームの移動指示を適用する）
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

	/** @brief 移動パラメータを設定する
	 *  @param _moveSpeed 移動速度
	 *  @param _turnSpeed 回転速度
	 */
	void SetMoveParams(float _moveSpeed, float _turnSpeed);

	/** @brief 移動指示を設定する（ワールド方向）
	 *  @details この関数は「毎フレーム呼ぶ」ことを前提とする
	 *  @param _moveDirWorld 進みたい方向（ワールド）/長さ0なら停止/Yは無視される
	 *  @param _speedScale 速度倍率（攻撃中の0.6など）
	 */
	void SetMoveIntentWorld(const DX::Vector3& _moveDirWorld, float _speedScale = 1.0f);

	/// @brief 明示的に停止させる（そのフレームの指示を無しにする）
	void ClearMoveIntent();

	/** @brief 移動を許可/禁止する
	 *  @param _enabled true で移動適用、false で停止固定
	 */
	void SetMoveEnabled(bool _enabled);

	/// @brief 現在移動しているか（水平速度から判定）
	bool IsMoving() const;

private:
	/// @brief 速度と回転をRigidbodyへ適用する
	void ApplyToRigidbody(float _deltaTime);

	/// @brief 水平速度のみ停止する
	void StopHorizontal() const;

private:
	Framework::Physics::Rigidbody3D* rigidbody = nullptr;	///< 移動対象

	float moveSpeed = 10.0f;		///< 移動速度
	float turnSpeed = 15.0f;		///< 回転速度

	bool moveEnabled = true;		///< 移動を適用するか

	bool hasPendingIntent = false;	///< このフレームに移動指示があるか
	DX::Vector3 pendingDirWorld{};	///< このフレームの移動方向（ワールド、水平のみ）
	float pendingSpeedScale = 1.0f;	///< このフレームの速度倍率
};
