/** @file   FollowCamera.cpp
 *  @brief  ターゲットを追従するカメラコンポーネントの実装
 *  @date   2025/11/12
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Game/Entities/FollowCamera.h"

#include<iostream>
//-----------------------------------------------------------------------------
// FollowCamera class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
FollowCamera::FollowCamera(GameObject* _owner, bool _active)
	: Component(_owner, _active),
	pivot(nullptr),
	target(nullptr),
	distance(6.0f),
	height(2.0f),
	smoothSpeed(5.0f)
{}

/// @brief 初期化処理
void FollowCamera::Initialize()
{
	if (!this->target)
	{
        // 追跡対象が設定されていない場合、オーナーオブジェクトを追跡対象に設定する
		std::cout << "[FollowCamera]: No target specified. Defaulting to owner object." << std::endl;
		this->target = this->Owner()->transform;
	}
}

/// @brief 終了処理
void FollowCamera::Dispose()
{}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void FollowCamera::Update(float _deltaTime)
{
    if (!this->pivot) { return; }

    //---------------------------------------------------------
    // 注視点を決定する
    //---------------------------------------------------------
    DX::Vector3 lookTarget;

    if (this->target)
    {
        // targetが存在する場合はその座標を注視する
        lookTarget = this->target->GetWorldPosition();
    }
    else
    {
        // target未指定ならPivotの前方方向を見る
        lookTarget = this->pivot->GetWorldPosition() + this->pivot->Forward() * 10.0f;
    }

    //---------------------------------------------------------
    // 目標位置を算出する（Pivot後方＋高さオフセット分）
    //---------------------------------------------------------
    DX::Vector3 desiredPos = this->pivot->GetWorldPosition()
        - this->pivot->Forward() * this->distance
        + DX::Vector3(0.0f, this->height, 0.0f);

    //---------------------------------------------------------
    // 現在位置を取得して追従する
    //---------------------------------------------------------
    DX::Vector3 currentPos = this->Owner()->transform->GetWorldPosition();
    DX::Vector3 newPos = DX::Vector3::Lerp(currentPos, desiredPos, _deltaTime * this->smoothSpeed);
    this->Owner()->transform->SetWorldPosition(newPos);

    //---------------------------------------------------------
    // 注視方向を更新する
    //---------------------------------------------------------
    this->Owner()->transform->LookAt(lookTarget);
}