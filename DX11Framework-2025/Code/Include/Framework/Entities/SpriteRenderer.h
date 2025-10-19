/** @file   SpriteRenderer.h
 *  @date   2025/10/19
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Camera2D.h"
#include "Include/Framework/Entities/SpriteComponent.h"
#include "Include/Framework/Entities/MaterialComponent.h"

#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/IndexBuffer.h"

#include "Include/Framework/Utils/CommonTypes.h"

 /** @class	SpriteRenderer
  *	 @brief	２D描画用コンポーネント
  */
class SpriteRenderer : public Component, public IDrawable
{
public:
    SpriteRenderer(GameObject* owner, bool isActive = true);
    ~SpriteRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

private:
    Camera2D* camera;
    Transform* transform;   
    SpriteComponent* spriteComponent;       ///< 画像情報
    MaterialComponent* materialComponent;   ///< マテリアル情報

    struct Vertex {
        DX::Vector3 position;
        DX::Vector4 color;
        DX::Vector2 uv;
    };
    std::unique_ptr<VertexBuffer>vertexBuffer;
    std::unique_ptr<IndexBuffer>indexBuffer;
};
