/** @file   MaterialManager.cpp
 *  @date   2025/10/19
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Core/ResourceHub.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Graphics/SpriteManager.h"
#include "Include/Framework/Shaders/ShaderManager.h"
#include <iostream>

//-----------------------------------------------------------------------------
// MaterialManager class
//-----------------------------------------------------------------------------

MaterialManager::MaterialManager()
    : materialMap(),
    defaultMaterial(std::make_unique<Material>())
{
    // デフォルトマテリアルを設定する
    this->InitializeDefaultMaterial(*this->defaultMaterial);

    auto model = this->Register("ModelTest");
    model->shaders = ResourceHub::Get<ShaderManager>().GetShaderProgram("ModelTest");
}

MaterialManager::~MaterialManager()
{
    this->defaultMaterial.reset();
    this->materialMap.clear();
}

/** @brief マテリアルを登録する（デフォルト値が入る）
 *  @param _key 識別キー
 *  @return Material* 登録されていない場合nullptr
 */
Material* MaterialManager::Register(const std::string& _key)
{
    // 既に存在する場合は登録しない
    if (this->materialMap.contains(_key))
    {
        std::cerr << "[Info] MaterialManager::Register: Key already exists: " << _key << std::endl;
        return this->Get(_key);
    }

    // 新しいMaterialを生成してデフォルト設定で初期化
    auto mat = std::make_unique<Material>();
    this->InitializeDefaultMaterial(*mat);

    // 登録
    Material* rawPtr = mat.get();
    this->materialMap.emplace(_key, std::move(mat));
    return rawPtr;
}

/** @brief マテリアルを登録解除する
 *  @param _key マテリアル名
 */
void MaterialManager::Unregister(const std::string& _key)
{
    if (!this->materialMap.erase(_key))
    {
        std::cerr << "[Error] MaterialManager::Unregister: Key not found: " << _key << std::endl;
    }
}

/** @brief 指定キーのマテリアルを取得する
 *  @param _key マテリアル名
 *  @return Material* リソースポインタ（見つからなければデフォルト）
 */
Material* MaterialManager::Get(const std::string& _key)
{
    auto it = this->materialMap.find(_key);
    if (it != this->materialMap.end())
    {
        return it->second.get();
    }

    std::cerr << "[Error] MaterialManager::Get: Material not found, returning default: " << _key << std::endl;
    return this->Default();
}

/** @brief デフォルトマテリアルを取得
 *  @return Material* デフォルトマテリアル
 */
Material* MaterialManager::Default() const
{
    return this->defaultMaterial.get();
}

void MaterialManager::InitializeDefaultMaterial(Material& _material)
{
    auto& shaderManager = ResourceHub::Get<ShaderManager>();
    auto& spriteManager = ResourceHub::Get<SpriteManager>();

    _material.samplerType = SamplerType::LinearWrap;
    _material.shaders = shaderManager.DefaultProgram();
    _material.albedoMap = spriteManager.Default();
    if (!_material.materialBuffer)
    {
        _material.materialBuffer = std::make_unique<DynamicConstantBuffer<MaterialParams>>();
    }
    _material.materialBuffer->Create(SystemLocator::Get<D3D11System>().GetDevice());
}
