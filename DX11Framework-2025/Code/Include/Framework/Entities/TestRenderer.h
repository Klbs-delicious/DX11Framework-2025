/** @file   TestRenderer.h
 *  @brief  Assimpモデル描画テスト用コンポーネント
 *  @date   2025/10/26
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/IndexBuffer.h"
#include "Include/Framework/Graphics/ModelInporter.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Shaders/ShaderManager.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>

class TestRenderer : public Component, public IDrawable
{
    using ModelData_t = Graphics::Import::ModelData;
    using ModelImporter_t = Graphics::Import::ModelImporter;
    using Vertex_t = Graphics::Import::Vertex;

public:
    TestRenderer(GameObject* _owner, bool _active = true);
    ~TestRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

private:
    Transform* transform = nullptr;
    Camera3D* camera = nullptr;

    ModelData_t modelData;
    bool modelLoaded = false;

    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<IndexBuffer> indexBuffer;

    MaterialComponent* materialComponent;   ///< マテリアル情報

    Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer; ///< b3: ライト用バッファ
};