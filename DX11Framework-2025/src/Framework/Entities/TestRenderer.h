#pragma once
#include "Framework/Entities/Component.h"
#include "Framework/Entities/PhaseInterfaces.h"
#include "Framework/Entities/Transform.h"
#include "Framework/Entities/Camera2D.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <chrono>
#include <SimpleMath.h>

class TestRenderer : public Component, public IDrawable
{
public:
    TestRenderer(GameObject* owner, bool isActive = true);
    ~TestRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

private:
    Camera2D* camera;
	Transform* transform;

    struct Vertex {
    	DirectX::SimpleMath::Vector3 position;
    	DirectX::SimpleMath::Vector4 color;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    std::chrono::steady_clock::time_point startTime;
};