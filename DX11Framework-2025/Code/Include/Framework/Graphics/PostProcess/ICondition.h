/**	@file	ICondition.h
*	@date	2026/04/10
*/
#pragma once

/**	@class		ICondition
 *	@brief		特定のデータ、状態などを評価して条件結果を返すインターフェース
 */
class ICondition
{
public:
	virtual ~ICondition() = default;

	/**	@brief	条件を評価する
	 *	@return	bool 条件が満たされている場合は true、そうでない場合は false
	 */
	virtual [[nodiscard]] bool Check()const = 0;
};