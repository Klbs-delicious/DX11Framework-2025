/** @file   MeshManager.h
 *  @date   2025/11/05
 */
#pragma once
#include "Include/Framework/Graphics/Mesh.h"
#include "Include/Framework/Core/IResourceManager.h"

#include <unordered_map>
#include <memory>
#include <string>

 /** @class MeshManager
  *  @brief 名前でメッシュリソースを管理するクラス
  */
class MeshManager : public IResourceManager<Graphics::Mesh>
{
public:
    MeshManager() = default;
    ~MeshManager() override = default;

    /** @brief メッシュを登録
     *  @param _key 登録名
     *  @return 登録されたメッシュポインタ、既に存在する場合は既存のものを返す
     */
    Graphics::Mesh* Register(const std::string& _key) override;

    /** @brief 外部生成済みメッシュを登録
     *  @param _key 登録名
     *  @param _mesh 登録対象
     */
    void Register(const std::string& _key, Graphics::Mesh* _mesh);

    /** @brief 登録を解除
     *  @param _key 登録名
     */
    void Unregister(const std::string& _key) override;

    /** @brief メッシュを取得
     *  @param _key 登録名
     *  @return メッシュポインタ、存在しない場合はnullptr
     */
    Graphics::Mesh* Get(const std::string& _key) override;

    /** @brief デフォルトメッシュを取得
     *  @return デフォルトメッシュ（nullptrの可能性あり）
     */
    Graphics::Mesh* Default() const override;

    /// @brief 全メッシュを削除
    void Clear();

private:
    std::unordered_map<std::string, std::unique_ptr<Graphics::Mesh>> meshTable; ///< 名前で管理するメッシュ辞書
    std::unique_ptr<Graphics::Mesh> defaultMesh;                                ///< デフォルトメッシュ
};
