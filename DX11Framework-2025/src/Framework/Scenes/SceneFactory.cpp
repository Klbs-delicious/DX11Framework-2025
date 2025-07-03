/**	@file	SceneFactory.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Scenes/SceneFactory.h"

//-----------------------------------------------------------------------------
// SceneFactory Class
//-----------------------------------------------------------------------------

/// @brief	コンストラクタ
SceneFactory::SceneFactory() {}
/// @brief	デストラクタ
SceneFactory::~SceneFactory() {}

/**	@brief	指定のSceneTypeに対応する生成関数を登録する
 *	@param	SceneType	_type		生成するシーンの種類
 *	@param	Creator		_creator	生成するシーンの種類
 */
void SceneFactory::Register(SceneType _type, Creator _creator)
{
	this->registry[_type] = std::move(_creator);
}

/**	@brief　指定されたSceneTypeに対応するシーンを生成して返す
 *	@param	SceneType					_type	生成するシーンの種類
 *	@return	std::unique_ptr<BaseScene>			シーンのインスタンス
 */
std::unique_ptr<BaseScene> SceneFactory::Create(SceneType _type) const
{
    auto it = this->registry.find(_type);
    if (it != this->registry.end()) {
        return it->second();
    }

    // 未登録のシーンタイプなら null を返す
    return nullptr; 
}

