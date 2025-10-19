/** @file   MaterialManager.h
 *  @date   2025/10/19
 */
#pragma once
#include "Include/Framework/Core/IResourceManager.h"
#include "Include/Framework/Graphics/Material.h"
#include <unordered_map>
#include <memory>

 /** @class MaterialManager
  *  @brief Materialを管理するクラス
  *  @details
  *     - Material の登録・取得・解除・デフォルト取得を提供
  */
class MaterialManager : public IResourceManager<Material>
{
public:
    MaterialManager();
    ~MaterialManager() override;

    /** @brief マテリアルを登録する
     *  @param _key 識別キー
     *  @return 成功したら true
     */
    Material* Register(const std::string& _key) override;

    /** @brief マテリアルを登録解除する
     *  @param  const std::string& _key マテリアル名
     */ 
    void Unregister(const std::string& _key) override;

    /** @brief 指定キーのマテリアルを取得する 
     *  @param  const std::string& _key マテリアル名
     */
    Material* Get(const std::string& _key) override;

    /** @brief デフォルトマテリアルを取得
     *  @return  Material* 
     */
    Material* Default() const override;

private:
    void InitializeDefaultMaterial(Material& _material);

    std::unordered_map<std::string, std::unique_ptr<Material>> materialMap;   ///< マテリアルマップ
    std::unique_ptr<Material> defaultMaterial;                              ///< デフォルト設定のマテリアル
};