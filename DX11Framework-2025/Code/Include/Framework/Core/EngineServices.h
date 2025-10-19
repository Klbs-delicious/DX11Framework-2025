﻿/** @file   EngineServices.h
 *  @date   2025/10/13
 */
#pragma once
// 重いヘッダを避ける
class SpriteManager;
class MaterialManager;
//struct Mesh;
class ShaderManager;

/** @struct     EngineServices
 *  @brief      リソース関連の依存を保持
 *  @details    - 読み取り専用前提
 */
struct EngineServices final
{
    SpriteManager* sprites = nullptr;
    MaterialManager* materials = nullptr;
    //IResourceManager<Mesh>* meshes = nullptr;
    ShaderManager* shaders = nullptr;
};