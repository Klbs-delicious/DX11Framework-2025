#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Camera3D.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Graphics/VertexBuffer.h"
#include "Include/Framework/Graphics/IndexBuffer.h"
#include "Include/Framework/Shaders/ShaderBase.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>
#include <vector>

class TestRenderer : public Component, public IDrawable
{
public:
    TestRenderer(GameObject* owner, bool active = true);
    ~TestRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

private:
    Transform* transform = nullptr;
    Camera3D* camera = nullptr;

    struct Vertex
    {
        DirectX::SimpleMath::Vector3 position;
        DirectX::SimpleMath::Vector3 normal;
        DirectX::SimpleMath::Vector2 uv;
    };

    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<IndexBuffer> indexBuffer;

    ShaderBase* vertexShader = nullptr;
    ShaderBase* pixelShader = nullptr;
};
