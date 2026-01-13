/** @file   ModelManager.h
 *  @date   2025/11/05
 */
#pragma once
#include "Include/Framework/Graphics/Mesh.h"
#include "Include/Framework/Graphics/ModelData.h"

#include "Include/Framework/Core/IResourceManager.h"
#include "Include/Framework/Graphics/MeshManager.h"
#include "Include/Framework/Graphics/MaterialManager.h"
#include "Include/Framework/Graphics/ModelImporter.h"

#include <unordered_map>
#include <memory>
#include <string>

 /** @class ModelManager
  *  @brief 名前でモデルリソースを管理するクラス
  */
class ModelManager : public IResourceManager<Graphics::ModelEntry>
{
public:
    ModelManager();
    ~ModelManager() override;

    /** @brief モデルを登録
     *  @param _key 登録名
     *  @return 既に存在する場合は既存のものを返す
     */
    Graphics::ModelEntry* Register(const std::string& _key) override;

    /** @brief 外部生成済みモデルデータを登録
     *  @param _key 登録名
     *  @param _model 登録対象
     */
    void Register(const std::string& _key, std::unique_ptr<Graphics::Import::ModelData> _model);

    /** @brief 登録を解除
     *  @param _key 登録名
     */
    void Unregister(const std::string& _key) override;

    /** @brief モデルデータを取得
     *  @param _key 登録名
     *  @return 存在しない場合はnullptr
     */
    Graphics::ModelEntry* Get(const std::string& _key) override;

    /** @brief デフォルト情報を取得
     *  @return デフォルト情報（nullptrの可能性あり）
     */
    Graphics::ModelEntry* Default() const override;

    /// @brief 全モデルを削除
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Graphics::ModelEntry>> modelTable;    ///< 名前で管理するモデル辞書
    std::unordered_map<std::string, Graphics::ModelInfo> modelInfoTable;                  ///< モデル情報辞書
    std::unique_ptr<Graphics::ModelEntry> defaultModel;                                   ///< デフォルトモデル

    Graphics::Import::ModelImporter modelImporter;
};
