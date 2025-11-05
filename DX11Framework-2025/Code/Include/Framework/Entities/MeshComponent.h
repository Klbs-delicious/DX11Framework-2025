/** @file   MeshComponent.h
 *  @date   2025/11/02
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Graphics/Mesh.h"

class MeshComponent : public Component
{
public:
    MeshComponent(GameObject* _owner, bool _isActive = true);
    ~MeshComponent() override;

    void Initialize() override;
    void Dispose() override;

    /**@brief メッシュの設定 
     * @param _mesh メッシュ情報
     */
    void SetMesh(Graphics::Mesh* _mesh) { this->mesh = _mesh; }

    /**@brief メッシュの取得
     * @return 
     */
    Graphics::Mesh* GetMesh() const { return this->mesh; }

private:
    Graphics::Mesh* mesh; ///< モデル情報
};
