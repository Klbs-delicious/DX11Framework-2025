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

#include "Include/Framework/Core/ITimeProvider.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/TimeScaleSystem.h"

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
	Ranged      ///< 遠距離攻撃（投げ物など）
};

/** @struct AttackDef
 *  @brief  攻撃定義
 */
struct AttackDef
{
	std::string attackClip;		///< 攻撃アニメーションクリップ
	AttackType attackType;		///< 攻撃タイプ
	float damage;				///< 与えるダメージ量
};

/** @class  AttackComponent
 *  @brief  攻撃中かどうかの状態と簡易的な攻撃時間管理を行う
 */
class AttackComponent : public Component, public IUpdatable, public BaseColliderDispatcher3D
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	AttackComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~AttackComponent() override = default;

	/// @brief 初期化処理
	void Initialize() override;

	/// @brief 終了処理
	void Dispose() override;

	/** @brief 毎フレーム更新
	 *  @param _deltaTime フレーム間の経過時間
	 */
	void Update(float _deltaTime) override;

	/** @brief 攻撃開始
	 *  @param _attackDef 攻撃定義
	 */
	void StartAttack(AttackDef _attackDef);

	/// @brief 攻撃終了
	void EndAttack();

	/// @brief 現在攻撃中か
	bool IsAttacking() const { return isAttacking; }

	void OnTriggerEnter(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)override;

	void OnTriggerExit(Framework::Physics::Collider3DComponent* _self, Framework::Physics::Collider3DComponent* _other)override;

private:
	AnimationClipManager* animClipManager;		///< アニメーションクリップ管理参照
	AnimationComponent* animationComponent;		///< アニメーションコンポーネント参照
	bool  isAttacking;							///< 攻撃中かどうか

	AttackDef currentAttackDef;								 ///< 現在の攻撃定義
	Graphics::Import::ClipEventWatcher clipEventWatcher;	 ///< 攻撃のクリップイベント監視を行う
	std::vector<Graphics::Import::ClipEventId> passedEvents; ///< 通過したイベントID一時保管用

	GameObject* attackObj;			///< 攻撃を受けるオブジェクト
	DodgeComponent* dodgeComponent;	///< 攻撃対象の回避コンポーネント参照

	float slowDuration = 1.0f;					///< スロー持続（秒・raw）
	float slowRemainingRawSec = 0.0f;			///< スロー残り時間（秒・raw）
	bool isSlowing = false;						///< 現在スロー中か

	ITimeProvider* timeProvider = nullptr;			///< rawDelta取得元（Initializeで取得）
	TimeScaleSystem* timeScaleSystem = nullptr;		///< TimeScaleSystem 参照（Initializeで SystemLocator から取得）
};