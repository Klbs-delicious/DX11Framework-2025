/**	@file	SceneFactory.h
*	@date	2025/07/03
*/
#pragma once
#include"Framework/Utils/NonCopyable.h"
#include"Framework/Scenes/SceneType.h"
#include"Framework/Scenes/BaseScene.h"

#include <memory>
#include <functional>
#include <unordered_map>

/**	@class		SceneFactory
 *	@brief		シーンの生成処理を行うファクトリパターン
 *	@details	このクラスはコピー、代入を禁止している
 */
class SceneFactory :private NonCopyable
{
public:
	using Creator = std::function<std::unique_ptr<BaseScene>()>;

	/// @brief	コンストラクタ
	SceneFactory();
	/// @brief	デストラクタ
	~SceneFactory();

	/**	@brief	指定のSceneTypeに対応する生成関数を登録する
	 *	@param	SceneType	_type		生成するシーンの種類
	 *	@param	Creator		_creator	生成するシーンの種類
	 */
	void Register(SceneType _type, Creator _creator);

	/**	@brief　指定されたSceneTypeに対応するシーンを生成して返す
	 *	@param	SceneType					_type	生成するシーンの種類
	 *	@return	std::unique_ptr<BaseScene>			シーンのインスタンス
	 */
	std::unique_ptr<BaseScene> Create(SceneType _type) const;

private:
	std::unordered_map<SceneType, Creator> registry;	///< SceneTypeとその生成関数の対応表
};
