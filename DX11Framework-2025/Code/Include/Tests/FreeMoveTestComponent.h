/** @file   FreeMoveTestComponent.h
 *  @brief  自由移動の挙動を検証するテスト用コンポーネント
 *  @date   2025/11/12
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"

 /** @class FreeMoveTestComponent
  *  @brief 自由移動の挙動を検証するテスト用コンポーネント
  *  @details - Componentを継承し、Updateフェーズで移動テストを行う
  */
class FreeMoveTestComponent : public Component, public IUpdatable
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _active コンポーネントの有効/無効
	 */
	FreeMoveTestComponent(GameObject* _owner, bool _active = true);

	/// @brief デストラクタ
	virtual ~FreeMoveTestComponent() = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 前フレームからの経過時間（秒）
	 */
	void Update(float _deltaTime) override;

private:
	Transform* transform;
	float speed;			///< 移動速度
	DX::Vector3 targetPos;	///< 目標地点
	bool hasTarget;			///< 目標地点が設定されているか
};

