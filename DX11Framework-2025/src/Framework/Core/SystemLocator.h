/**	@file	SystemLocator.h
*	@date	2025/09/11
*/
#pragma once
#include"Framework/Utils/NonCopyable.h"

#include<unordered_map>
#include<typeindex>
#include<cassert>

/**	@class	SystemLocator
 *	@brief	システムの一元管理を行う
 *	@details
 *  - このクラスはコピー、代入を禁止している
 *  - システムのインスタンスを型ごとに一元管理し、このクラスを介してどこからでも取得できるようにする
 */
class SystemLocator :private NonCopyable
{
public:
	template<typename T>
	/** @brief  システムの登録
	 *	@param	T* _system	登録したいシステム
	 */
	static void Register(T* _system) {
		// システムが未登録の場合そのシステムを登録する
		auto key = std::type_index(typeid(T));	
		assert(_system && !SystemLocator::systems.contains(key) && "システムが既に登録されているか、nullです。");
		SystemLocator::systems[key] = _system;
	}

	template<typename T>
	/** @brief  システムの取得
	 *	@return	T& システムの参照
	 */
	static T& Get() {
		// システムが登録済の場合そのシステムの参照を返す
		auto key = std::type_index(typeid(T));
		assert(SystemLocator::systems.contains(key) && "この型のシステムは登録されていません。");
		return	*static_cast<T*>(SystemLocator::systems[key]);
	}

	template<typename T>
	///	@brief システムの解除
	static void Unregister() {
		SystemLocator::systems.erase(std::type_index(typeid(T)));
	}

private:

	/**	@brief	システムの参照を保持する
	 *	@details - ヘッダ内 inline static は C++17 準拠で安全だが、初期化順序・スレッド・ライフタイムに注意が必要
	 */
	inline static std::unordered_map<std::type_index, void*> systems;	 
};