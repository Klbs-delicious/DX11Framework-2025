/** @file   ResourceHub.h
 *  @date   2025/09/25
 */
#pragma once
#include "Framework/Utils/NonCopyable.h"
#include "Framework/Core/SystemLocator.h"
#include "Framework/Core/IResourceManager.h"

#include <typeindex>
#include <unordered_map>
#include <cassert>

 /** @class ResourceHub
  *  @brief リソース管理インスタンスの一元取得窓口
  *  @details
  *  - 各種 IResourceManager<T> を型ごとに登録・取得できる
  *  - SystemLocatorと同様の構造で、どこからでもアクセス可能
  */
class ResourceHub : private NonCopyable {
public:
    template<typename T>
    /** @brief リソースマネージャの登録
     *  @param IResourceManager<T>* _manager 登録するマネージャ
     */
    static void Register(IResourceManager<T>* _manager) {
        auto key = std::type_index(typeid(T));
        assert(_manager && !ResourceHub::managers.contains(key) && "既に登録済み、またはnullです。");
        ResourceHub::managers[key] = _manager;
    }

    template<typename T>
    /** @brief リソースマネージャの取得
     *  @return IResourceManager<T>& マネージャの参照
     */
    static IResourceManager<T>& Get() {
        auto key = std::type_index(typeid(T));
        assert(ResourceHub::managers.contains(key) && "この型のリソースマネージャは登録されていません。");
        return *static_cast<IResourceManager<T>*>(ResourceHub::managers[key]);
    }

    template<typename T>
    /** @brief リソースマネージャの解除 */
    static void Unregister() {
        ResourceHub::managers.erase(std::type_index(typeid(T)));
    }

private:
    inline static std::unordered_map<std::type_index, void*> managers;
};