#include"Include/Framework/Entities/Component.h"
#include"Include/Framework/Entities/GameObject.h"

/**	@brief リソース関連の参照を取得する
 *	@return EngineServices*
 */
const EngineServices* Component::Services() const { return this->Owner() ? this->Owner()->Services() : nullptr; }
