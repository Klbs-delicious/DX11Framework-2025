/** @file   MeshManager.cpp
 *  @date   2025/11/05
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/MeshManager.h"

#include <iostream>

//-----------------------------------------------------------------------------
// MeshManager class
//-----------------------------------------------------------------------------

Graphics::Mesh* MeshManager::Register(const std::string& _key)
{
    // 既に存在する場合はそのまま返す
    auto it =this-> meshTable.find(_key);
    if (it != this->meshTable.end()) { return it->second.get(); }    

    // 新規メッシュを生成して登録
    auto mesh = std::make_unique<Graphics::Mesh>();
    Graphics::Mesh* ptr = mesh.get();
    this->meshTable[_key] = std::move(mesh);

    std::cout << "[MeshManager] Registered mesh: " << _key << std::endl;
    return ptr;
}

void MeshManager:: Register(const std::string& _key, Graphics::Mesh* _mesh)
{
    if (!_mesh) { return; }

    // 所有を移す or 参照保持に切替
    this->meshTable[_key] = std::unique_ptr<Graphics::Mesh>(_mesh);
}

void MeshManager::Unregister(const std::string& _key)
{
    auto it = this->meshTable.find(_key);
    if (it != this->meshTable.end())
    {
        this->meshTable.erase(it);
        std::cout << "[MeshManager] Unregistered mesh: " << _key << std::endl;
    }
}

Graphics::Mesh* MeshManager::Get(const std::string& _key)
{
    auto it = this->meshTable.find(_key);
    if (it != this->meshTable.end()) { return it->second.get(); }
        
    return nullptr;
}

Graphics::Mesh* MeshManager::Default() const
{
    return this->defaultMesh.get();
}

void MeshManager::Clear()
{
    this->meshTable.clear();
    this->defaultMesh.reset();
    std::cout << "[MeshManager] Cleared all meshes.\n";
}
