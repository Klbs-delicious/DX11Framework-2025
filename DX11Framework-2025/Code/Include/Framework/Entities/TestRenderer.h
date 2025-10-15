#pragma once
#include "Include/Framework/Core/ResourceHub.h"
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"
#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/Camera2D.h"
#include "Include/Framework/Graphics/Sprite.h"
#include "Include/Framework/Shaders/ShaderBase.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <vector>

class TestRenderer : public Component, public IDrawable
{
public:
    TestRenderer(GameObject* owner, bool isActive = true);
    ~TestRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

    /** @brief Spriteの設定
     *  @param std::string _spriteName    Spriteの情報
     *  @return bool 設定出来たら true
     */
    bool SetSprite(const std::string& _spriteName);

private:
    Camera2D* camera;
	Transform* transform;
    const Sprite* sprite;         ///< 画像情報

    struct Vertex {
    	DirectX::SimpleMath::Vector3 position;
    	DirectX::SimpleMath::Vector4 color;
        DirectX::SimpleMath::Vector2 uv;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    std::vector<ShaderBase*> shaders;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};