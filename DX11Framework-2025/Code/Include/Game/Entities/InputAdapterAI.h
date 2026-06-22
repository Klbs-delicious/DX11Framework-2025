/**@file InputAdapterAI.h
 * @date 2026/06/23
 */
#pragma once

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Utils/CommonTypes.h"

#include "Include/Game/Entities/IAIBehavior.h"

 //-----------------------------------------------------------------------------

/** @enum	InputType
 * @brief	入力の種類を定義する列挙型
 */
enum class InputType
{
	None = 0,
	Move,
	Attack,
	Dodge,
};

/** @struct	InputCommand
 *	@brief 入力コマンドを表す構造体
 */
struct InputCommand
{
	InputType type = InputType::None;						///< 入力の種類
	DX::Vector3 direction = DX::Vector3(0.0f, 0.0f, 0.0f);	///< 入力の方向や位置などの情報
};

/** @class InputAdapterAI
 * @brief AIの入力を中継するコンポーネント
 */
class InputAdapterAI : public Component
{
public:
	InputAdapterAI(GameObject* _owner, bool _isActive = true);
	virtual ~InputAdapterAI() override = default;

	virtual void Initialize() = 0;
	virtual void Dispose() = 0;

	/** @brief	AIの入力コマンドを取得する
	 *  @return	入力コマンド情報
	 */
	InputCommand GetCommand() const;

private:
	IAIBehavior* aiBehavior;	///< AIの行動を取得するためのインターフェース
};