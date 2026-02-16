/** @file   AttackComponent.h
 *  @brief  攻撃状態と攻撃判定を管理するコンポーネント
 *  @date   2026/02/12
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/AnimationComponent.h"
#include "Include/Framework/Entities/Collider3DComponent.h"

#include "Include/Framework/Graphics/AnimationClipManager.h"
#include "Include/Framework/Graphics/ClipEventWatcher.h"

#include "Include/Game/Entities/DodgeComponent.h"

#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// AttackComponent class
//-----------------------------------------------------------------------------

/** @enum AttackType
 *  @brief 攻撃タイプ
 */
enum class AttackType
{
	Melee,      ///< 近接攻撃
	Ranged      ///< 遠距離攻撃
};

/** @struct AttackDef
 *  @brief 攻撃定義
 */
struct AttackDef
{
	std::string attackClip;   ///< 攻撃アニメーション名
	AttackType attackType;    ///< 攻撃タイプ
	float damage;             ///< ダメージ量
};

/** @class AttackComponent
 *  @brief 攻撃状態とHitOn判定を管理する
 */
class AttackComponent
	: public Component
	, public IUpdatable
	, public BaseColliderDispatcher3D
{
public:

	/** @brief コンストラクタ
	 *  @param _owner 所有オブジェクト
	 *  @param _isActive 有効状態
	 */
	AttackComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~AttackComponent() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 解放
	void Dispose() override;

	/** @brief 更新処理
	 *  @param _deltaTime 経過時間
	 */
	void Update(float _deltaTime) override;

	/** @brief 攻撃開始
	 *  @param _attackDef 攻撃定義
	 */
	void StartAttack(AttackDef _attackDef);

	/// @brief 攻撃終了
	void EndAttack();

	/// @brief 攻撃中か
	bool IsAttacking() const { return this->isAttacking; }

	void OnTriggerEnter(
		Framework::Physics::Collider3DComponent* _self,
		Framework::Physics::Collider3DComponent* _other) override;

	void OnTriggerExit(
		Framework::Physics::Collider3DComponent* _self,
		Framework::Physics::Collider3DComponent* _other) override;

private:

	AnimationClipManager* animClipManager;      ///< クリップ管理
	AnimationComponent* animationComponent;     ///< アニメーション参照

	bool isAttacking;                            ///< 攻撃中フラグ
	bool justTriggered;                          ///< 同一攻撃での成立ガード

	AttackDef currentAttackDef;                  ///< 現在の攻撃定義

	Graphics::Import::ClipEventWatcher clipEventWatcher; ///< イベント監視
	std::vector<Graphics::Import::ClipEventId> passedEvents; ///< 通過イベント

	GameObject* attackObj;                       ///< 攻撃対象
	DodgeComponent* dodgeComponent;              ///< 回避参照
};