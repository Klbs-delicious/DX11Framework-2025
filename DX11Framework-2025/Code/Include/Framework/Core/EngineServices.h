/** @file   EngineServices.h
 *  @date   2025/10/13
 */
#pragma once
// 重いヘッダを避ける
template<class T> class IResourceManager;
struct Sprite;
//struct Material;
//struct Mesh;
class ShaderBase;

/** @struct     EngineServices
 *  @brief      リソース関連の依存を保持
 *  @details    - 読み取り専用前提
 */
struct EngineServices final
{
    IResourceManager<Sprite>* sprites = nullptr;
    //IResourceManager<Material>* materials = nullptr;
    //IResourceManager<Mesh>* meshes = nullptr;
    IResourceManager<ShaderBase>* shaders = nullptr;
};
