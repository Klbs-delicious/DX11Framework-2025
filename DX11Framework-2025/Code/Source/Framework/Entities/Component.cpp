/**@file   Component.cpp
 * @date   2025/09/16
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------

#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/GameObject.h"

//-----------------------------------------------------------------------------
// Component - Active Control
//-----------------------------------------------------------------------------

/** @brief コンポーネントの有効状態を設定する
 *  @param _active コンポーネントの有効 / 無効
 */
void Component::SetActive(bool _active)
{
	if (this->isActive == _active)
	{
		return;
	}

	// 状態変更の通知
	GameObjectEventContext eventContext =
	{
		this->Owner()->GetName(),
		this,	// ← 変更点: nullptr ではなく this を渡す方が正しい
		_active ? GameObjectEvent::ComponentEnabled
				: GameObjectEvent::ComponentDisabled
	};

	this->Owner()->NotifyEvent(eventContext);

	this->isActive = _active;
}

//-----------------------------------------------------------------------------
// Component - Engine Services
//-----------------------------------------------------------------------------

/** @brief リソース関連の参照を取得する
 *  @return EngineServices*
 */
const EngineServices* Component::Services() const
{
	return this->Owner() ? this->Owner()->Services() : nullptr;
}
