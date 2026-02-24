/** @file   AttackDef.h
 *  @brief  攻撃定義
 *  @date   2026/02/17
 */
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>

//-----------------------------------------------------------------------------
// AttackDef
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

