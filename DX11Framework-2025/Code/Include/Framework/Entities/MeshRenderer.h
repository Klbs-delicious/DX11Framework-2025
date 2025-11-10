/** @file   MeshRenderer.h
 *  @date   2025/11/05
 */
#pragma once
#include "Include/Framework/Entities/Component.h"
#include "Include/Framework/Entities/PhaseInterfaces.h"

#include "Include/Framework/Entities/Transform.h"
#include "Include/Framework/Entities/MaterialComponent.h"
#include "Include/Framework/Entities/MeshComponent.h"
#include "Include/Framework/Entities/Camera3D.h"

#include "Include/Framework/Graphics/ModelImporter.h"
#include "Include/Framework/Graphics/ModelData.h"
#include "Include/Framework/Graphics/DynamicConstantBuffer.h"

#include "Include/Framework/Utils/CommonTypes.h"

#include <d3d11.h>
#include <SimpleMath.h>
#include <memory>

 // 簡易ライト設定
struct LightBuffer
{
    DX::Vector3 lightDir;
    float pad1;
    DX::Vector4 baseColor;
};

 /** @class	MeshRenderer
  *	 @brief	 静的メッシュ描画コンポーネント
  */
class MeshRenderer : public Component, public IDrawable
{
    using ModelData_t = Graphics::Import::ModelData;
    using ModelImporter_t = Graphics::Import::ModelImporter;
    using Vertex_t = Graphics::Import::Vertex;

public:
    MeshRenderer(GameObject* _owner, bool _active = true);
    ~MeshRenderer();

    void Initialize() override;
    void Draw() override;
    void Dispose() override;

private:
    Transform* transform = nullptr;
    Camera3D* camera = nullptr;

    MeshComponent*  meshComponent;          ///< メッシュ情報
    MaterialComponent* materialComponent;   ///< マテリアル情報

    LightBuffer light = {};
	std::unique_ptr<DynamicConstantBuffer<LightBuffer>> lightBuffer; ///< ライト用バッファ
};