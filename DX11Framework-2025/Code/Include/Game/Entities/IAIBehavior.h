/** @file   IAIBehavior.h
 *  @date   2026/06/22
 */
#pragma once
#include"Include/Framework/Utils/CommonTypes.h"

 /** @brief 敵AIの行動の種類を定義する列挙型
 */
enum class EnemyDecisionType
{
	None=0,
	Attack,	
	Stay,
	Chase,
};

/** @brief 敵AIの行動を表す構造体
 */
struct EnemyDecision
{
	EnemyDecisionType type = EnemyDecisionType::None;		///< 敵AIの行動の種類
	DX::Vector3 direction = DX::Vector3(0.0f, 0.0f, 0.0f);	///< 敵AIの行動の方向や位置などの情報
};

 /** @class  IAIBehavior
 *  @brief  敵AIの行動を定義するインターフェース
 */
class IAIBehavior
{
public:
	/** @brief 敵AIの行動を取得する
	 *  @return 敵AIの行動を表すEnemyDecision構造体
	 */
	virtual EnemyDecision GetDecision() const = 0;	
};