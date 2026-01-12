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

    /** @brief スプライトの乗算カラー（ティント）を設定する */
    void SetColor(const DX::Color& _color) { this->tintColor = _color; }

    /** @brief 現在の乗算カラーを取得する */
    DX::Color GetColor() const { return this->tintColor; }

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

    DX::Color tintColor { 1,1,1,1 }; ///< 乗算カラー（UI色）
};
