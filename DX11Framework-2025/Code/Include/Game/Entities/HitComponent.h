/** @file   HitComponent.h
 *  @brief  被弾処理（攻撃を受けた側の共通処理）
 *  @date   2026/02/17
 */
#pragma once

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/Component.h"

#include "Include/Framework/Entities/GameObject.h"

#include "Include/Framework/Entities/Rigidbody3D.h"

#include "Include/Game/Entities/AttackDef.h" // AttackType がある想定（無ければ適切な場所へ）

#include <iostream>

//-----------------------------------------------------------------------------
// HitInfo
//-----------------------------------------------------------------------------

struct HitInfo
{
	GameObject* attacker = nullptr;
	AttackType attackType = AttackType::Melee;
	float damage = 0.0f;
	DX::Vector3 hitDirWorld = DX::Vector3::Zero;
	float knockback = 0.0f;
};

//-----------------------------------------------------------------------------
// HitComponent class
//-----------------------------------------------------------------------------

/** @class  HitComponent
 *  @brief  攻撃を受けた側の処理をまとめる（敵：吹っ飛び／プレイヤー：被弾）
 */
class HitComponent : public Component
{
public:
	/** @brief コンストラクタ
	 *  @param _owner このコンポーネントがアタッチされるオブジェクト
	 *  @param _isActive コンポーネントの有効/無効
	 */
	HitComponent(GameObject* _owner, bool _isActive = true);

	/// @brief デストラクタ
	~HitComponent() override = default;

	/// @brief 初期化
	void Initialize() override;

	/** @brief 被弾処理を行う
	 *  @param _hit 当たった情報
	 */
	void OnHit(const HitInfo& _hit);

private:
	Framework::Physics::Rigidbody3D* rigidbody = nullptr; ///< 吹っ飛びに使う（無い場合は簡易処理へ）
};

