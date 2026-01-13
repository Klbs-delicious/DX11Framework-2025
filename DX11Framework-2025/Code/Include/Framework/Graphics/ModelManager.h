/** @file   MeshManager.h
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


 /** @brief モデルで使用するデータのエントリ
 */
struct ModelEntry
{
public:
	Graphics::Mesh* mesh = nullptr; 
	Material* material = nullptr;   

    void SetModelData(std::unique_ptr<Graphics::Import::ModelData> _modelData)
    {
        this->modelData = std::move(_modelData);
	}

    Graphics::Import::ModelData* GetModelData() const
    {
        return this->modelData.get();
	}

private:
	std::unique_ptr<Graphics::Import::ModelData> modelData = nullptr;
};

 /** @brief モデル情報
 */
struct ModelInfo
{
	const std::string filename;     ///< モデルファイル名
    const std::string textureDir;   ///< テクスチャディレクトリ
};

 /** @class ModelManager
  *  @brief 名前でモデルリソースを管理するクラス
  */
class ModelManager : public IResourceManager<ModelEntry>
{
public:
    ModelManager();
    ~ModelManager() override = default;

    /** @brief モデルを登録
     *  @param _key 登録名
     *  @return 既に存在する場合は既存のものを返す
     */
    ModelEntry* Register(const std::string& _key) override;

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
    ModelEntry* Get(const std::string& _key) override;

    /** @brief デフォルト情報を取得
     *  @return デフォルト情報（nullptrの可能性あり）
     */
    ModelEntry* Default() const override;

    /// @brief 全モデルを削除
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<ModelEntry>> modelTable;    ///< 名前で管理するモデル辞書
    std::unordered_map<std::string, ModelInfo> modelInfoTable;                  ///< モデル情報辞書
    std::unique_ptr<ModelEntry> defaultModel;                                   ///< デフォルトモデル

    Graphics::Import::ModelImporter modelImporter;
};
