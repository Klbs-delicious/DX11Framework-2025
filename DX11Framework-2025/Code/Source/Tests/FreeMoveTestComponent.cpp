/** @file   FreeMoveTestComponent.cpp
 *  @brief  自由移動の挙動を検証するテスト用コンポーネントの実装
 *  @date   2025/11/12
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Tests/FreeMoveTestComponent.h"
#include "Include/Framework/Entities/GameObject.h"

#include<random>
//-----------------------------------------------------------------------------
// FreeMoveTestComponent class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param _owner このコンポーネントがアタッチされるオブジェクト
 *  @param _active コンポーネントの有効/無効
 */
FreeMoveTestComponent::FreeMoveTestComponent(GameObject* _owner, bool _active)
	: Component(_owner, _active), transform(nullptr), speed(10.0f), targetPos(0.0f, 0.0f, 0.0f), hasTarget(false)
{}

/// @brief 初期化処理
void FreeMoveTestComponent::Initialize()
{
	// Transformコンポーネントを取得
	this->transform = this->Owner()->transform;
}

/// @brief 終了処理
void FreeMoveTestComponent::Dispose()
{}

/** @brief 更新処理
 *  @param _deltaTime 前フレームからの経過時間（秒）
 */
void FreeMoveTestComponent::Update(float _deltaTime)
{
    // 乱数生成器
    static std::mt19937 engine(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

    // 目標地点がない場合はランダムに決める
    if (!this->hasTarget)
    {
        this->targetPos = DX::Vector3(
            dist(engine),
            dist(engine),
            dist(engine)
        );
        this->hasTarget = true;
    }

    DX::Vector3 current = this->transform->GetLocalPosition();
    DX::Vector3 diff = this->targetPos - current;
    float distLen = diff.Length();

    // 近づいたら新しい地点を選ぶ
    if (distLen < 0.1f)
    {
        this->hasTarget = false;
        return;
    }

	// 方向ベクトルを正規化する
    DX::Vector3 dir = diff / distLen;

    // 移動処理
    DX::Vector3 newPos = current + dir * this->speed * _deltaTime;
    this->transform->SetLocalPosition(newPos);
}