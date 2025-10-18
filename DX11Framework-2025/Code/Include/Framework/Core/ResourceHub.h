/** @file   ResourceHub.h
 *  @date   2025/09/25
 */
#pragma once
#include "Include/Framework/Utils/NonCopyable.h"
#include "Include/Framework/Core/IResourceManager.h"

#include <typeindex>
#include <unordered_map>
#include <cassert>

 /** @class ResourceHub
  *  @brief リソース管理インスタンスの一元取得窓口
  *  @details
  *     - 各型 T に対して、IResourceManager<T> の派生を登録可能
  *     - ShaderManager, SpriteManager などを型安全に取得可能
  *     - IResourceManager<T> 経由でも取得可能（共通アクセス用途）
  */
class ResourceHub : private NonCopyable {
public:   
    /** @brief リソースマネージャの登録
     *  @param IResourceManager<T>* _manager 登録するマネージャ
     */
    template<typename T>
    static void Register(IResourceManager<T>* _manager) {
        auto key = std::type_index(typeid(T));
        assert(_manager && !ResourceHub::managers.contains(key) && "既に登録済み、またはnullです。");
        ResourceHub::managers[key] = _manager;
    }

    /** @brief リソースマネージャの取得
     *  @return ManagerT& リソースマネージャの参照
     */
    template<typename ManagerT>
    static ManagerT& Get() {
        // リソースの型名を使用する
        using ResourceT = typename ManagerT::ResourceType; 
        auto key = std::type_index(typeid(ResourceT));
        assert(ResourceHub::managers.contains(key) && "この型のリソースマネージャは登録されていません。");
        return *static_cast<ManagerT*>(ResourceHub::managers[key]);
    }

    /** @brief 共通インターフェース経由でリソースマネージャーを取得 
     *  @return T& リソースマネージャの参照
     */
    template<typename T>
    static IResourceManager<T>& GetInterface() {
        auto key = std::type_index(typeid(T));
        assert(managers.contains(key) && "この型のリソースマネージャは登録されていません。");
        return *static_cast<IResourceManager<T>*>(managers[key]);
    }
 
    /// @brief リソースマネージャの解除
    template<typename T>
    static void Unregister() {
        ResourceHub::managers.erase(std::type_index(typeid(T)));
    }

private:
	inline static std::unordered_map<std::type_index, void*> managers;  ///< 登録されたリソースマネージャのマップ
};