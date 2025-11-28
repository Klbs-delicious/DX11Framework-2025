/** @file   ColliderDebugRenderer.cpp
 *  @brief  ColliderDebugRenderer の実装
 *  @date   2025/11/29
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Entities/ColliderDebugRenderer.h"
#include "Include/Framework/Entities/GameObject.h"
#include "Include/Framework/Entities/GameObjectManager.h"

#include "Include/Framework/Core/D3D11System.h"
#include "Include/Framework/Core/RenderSystem.h"
#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/ResourceHub.h"
#include "Include/Framework/Shaders/ShaderManager.h"

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace DirectX;
using namespace DirectX::SimpleMath;

//-----------------------------------------------------------------------------
// Constructor / Destructor
//-----------------------------------------------------------------------------
ColliderDebugRenderer::ColliderDebugRenderer(GameObject* _owner, bool _active)
    : Component(_owner, _active)
    , transform(nullptr)
    , camera(nullptr)
    , collider(nullptr)
    , vertexBuffer(nullptr)
    , linePoints()
    , shaders(nullptr)
    , rigidbody(nullptr)
{
}

ColliderDebugRenderer::~ColliderDebugRenderer() noexcept
{
    this->Dispose();
}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::Initialize()
{
    //======================================================================
    // 必要なコンポーネントの取得
    //======================================================================
    this->transform = this->Owner()->GetComponent<Transform>();
    this->collider = this->Owner()->GetComponent<Framework::Physics::Collider3DComponent>();
    this->rigidbody = this->Owner()->GetComponent<Framework::Physics::Rigidbody3D>();
    if (!this->transform)
    {
        std::cout << "[ColliderDebugRenderer] Transform が見つかりません\n";
        return;
    }

    if (!this->collider)
    {
        std::cout << "[ColliderDebugRenderer] Collider3DComponent が存在しません\n";
        return;
    }

    auto& gameObjectManager = SystemLocator::Get<GameObjectManager>();
    GameObject* cameraObject = gameObjectManager.GetFindObjectByName("Camera3D");
    if (!cameraObject)
    {
        std::cout << "[ColliderDebugRenderer] Camera3D オブジェクトが見つかりません\n";
        return;
    }

    this->camera = cameraObject->GetComponent<Camera3D>();
    if (!this->camera)
    {
        std::cout << "[ColliderDebugRenderer] Camera3D コンポーネントが見つかりません\n";
        return;
    }

    //======================================================================
    // 形状に応じてライン頂点列を構築
    //======================================================================
    using ShapeType_t = Framework::Physics::ColliderShapeType;

    this->linePoints.clear();
    // デバッグ表示はワールドスケールを基準にする
    auto worldScale = this->Owner()->GetTransform()->GetWorldScale();

    // 球用の統一スケール（Collider と同じルール）
    const float uniformScale = std::max({ std::fabs(worldScale.x), std::fabs(worldScale.y), std::fabs(worldScale.z) });

    switch (this->collider->GetShapeType())
    {
    case ShapeType_t::Box:
        this->BuildBoxWire(this->collider->GetBoxHalfExtent() * worldScale);
        break;

    case ShapeType_t::Sphere:
        this->BuildSphereWire(this->collider->GetSphereRadius() * uniformScale);
        break;

    case ShapeType_t::Capsule:
        this->BuildCapsuleWire(
            this->collider->GetCapsuleRadius() * std::fabs(worldScale.x),
            this->collider->GetCapsuleHalfHeight() * std::fabs(worldScale.y)
        );
        break;

    default:
        // 未対応の形状は描画しない
        return;
    }

    if (this->linePoints.empty()) { return; }

    //======================================================================
    // 頂点バッファ生成
    //======================================================================
    auto& d3d = SystemLocator::Get<D3D11System>();
    auto device = d3d.GetDevice();

    this->vertexBuffer = std::make_unique<VertexBuffer>();

    this->vertexBuffer->Create(
        device,
        this->linePoints.data(),
        static_cast<UINT>(sizeof(DX::Vector3)),
        static_cast<UINT>(this->linePoints.size()),
        true   // 動的かどうか（ここではとりあえず true にしている）
    );

    //======================================================================
    // デバッグ用シェーダ取得
    //======================================================================
    auto& shaderMgr = ResourceHub::Get<ShaderManager>();
    this->shaders = shaderMgr.GetShaderProgram("DebugWireframe"); // まだライン描画は未実装

    if (!this->shaders)
    {
        std::cout << "[ColliderDebugRenderer] DebugWireframe シェーダが見つかりません\n";
    }
}

//-----------------------------------------------------------------------------
// Dispose
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::Dispose()
{
    this->vertexBuffer.reset();
    this->linePoints.clear();

    this->transform = nullptr;
    this->camera = nullptr;
    this->collider = nullptr;
    this->shaders = nullptr;
}

//-----------------------------------------------------------------------------
// Draw
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::Draw()
{
    // 必要なコンポーネントや頂点バッファが揃っていなければ描画しない
    if (!this->collider || !this->transform || !this->camera) { return; }
    if (!this->vertexBuffer || this->linePoints.empty()) { return; }

    auto& d3d = SystemLocator::Get<D3D11System>();
    auto context = d3d.GetContext();
    auto& render = SystemLocator::Get<RenderSystem>();

    // ラスタライザステートをワイヤーフレームに切り替え
    render.SetRasterizerState(RasterizerType::WireframeCullBack);

    // 実描画
    this->DrawInternal(context);

    // 元のラスタライザステートに戻す
    render.SetRasterizerState(RasterizerType::SolidCullNone);
}

//-----------------------------------------------------------------------------
// Build Box Wire
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::BuildBoxWire(const DX::Vector3& _halfExtent)
{
    this->linePoints.clear();

    DX::Vector3 v[8] =
    {
        { -_halfExtent.x, -_halfExtent.y, -_halfExtent.z },
        {  _halfExtent.x, -_halfExtent.y, -_halfExtent.z },
        {  _halfExtent.x,  _halfExtent.y, -_halfExtent.z },
        { -_halfExtent.x,  _halfExtent.y, -_halfExtent.z },

        { -_halfExtent.x, -_halfExtent.y,  _halfExtent.z },
        {  _halfExtent.x, -_halfExtent.y,  _halfExtent.z },
        {  _halfExtent.x,  _halfExtent.y,  _halfExtent.z },
        { -_halfExtent.x,  _halfExtent.y,  _halfExtent.z },
    };

    int edges[][2] =
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };

    constexpr int edgeCount = static_cast<int>(std::size(edges));

    for (int i = 0; i < edgeCount; ++i)
    {
        this->linePoints.push_back(v[edges[i][0]]);
        this->linePoints.push_back(v[edges[i][1]]);
    }
}

//-----------------------------------------------------------------------------
// Build Sphere Wire (XY / XZ / YZ の3リング)
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::BuildSphereWire(float _radius)
{
    this->linePoints.clear();

    const int segments = 24;

    for (int i = 0; i < segments; ++i)
    {
        float t0 = DX::TWO_PI * static_cast<float>(i) / static_cast<float>(segments);
        float t1 = DX::TWO_PI * static_cast<float>(i + 1) / static_cast<float>(segments);

        // XY 平面リング
        this->linePoints.emplace_back(_radius * cosf(t0), _radius * sinf(t0), 0.0f);
        this->linePoints.emplace_back(_radius * cosf(t1), _radius * sinf(t1), 0.0f);

        // XZ 平面リング
        this->linePoints.emplace_back(_radius * cosf(t0), 0.0f, _radius * sinf(t0));
        this->linePoints.emplace_back(_radius * cosf(t1), 0.0f, _radius * sinf(t1));

        // YZ 平面リング
        this->linePoints.emplace_back(0.0f, _radius * cosf(t0), _radius * sinf(t0));
        this->linePoints.emplace_back(0.0f, _radius * cosf(t1), _radius * sinf(t1));
    }
}

//-----------------------------------------------------------------------------
// Build Capsule Wire（完全版：上下が正しい向きに膨らむ）
//-----------------------------------------------------------------------------
void ColliderDebugRenderer::BuildCapsuleWire(float _radius, float _halfHeight)
{
    this->linePoints.clear();

    const int segments = 24;      // 円周方向の分割
    const int rings = 12;      // 半球の高さ方向の分割（端点除く）

    const float yTop = _halfHeight;   // 円柱の上端（上リングのY）
    const float yBot = -_halfHeight;   // 円柱の下端（下リングのY）

    //======================================================================
    // 円柱の上下リング（確認済み：そのまま）
    //======================================================================
    for (int i = 0; i < segments; ++i)
    {
        float t0 = DX::TWO_PI * (float)i / segments;
        float t1 = DX::TWO_PI * (float)(i + 1) / segments;

        // 下リング
        this->linePoints.emplace_back(_radius * cosf(t0), yBot, _radius * sinf(t0));
        this->linePoints.emplace_back(_radius * cosf(t1), yBot, _radius * sinf(t1));

        // 上リング
        this->linePoints.emplace_back(_radius * cosf(t0), yTop, _radius * sinf(t0));
        this->linePoints.emplace_back(_radius * cosf(t1), yTop, _radius * sinf(t1));
    }

    //======================================================================
    // 下半球（下方向にふくらむ）: yBot から極へ
    //======================================================================
    for (int r = 1; r < rings; ++r)
    {
        float a = (DX::PI * 0.5f) * (float)r / (float)rings; // 0→π/2（端点除く）
        float ringRadius = _radius * cosf(a);
        float ringY = yBot - _radius * sinf(a);

        for (int i = 0; i < segments; ++i)
        {
            float t0 = DX::TWO_PI * (float)i / segments;
            float t1 = DX::TWO_PI * (float)(i + 1) / segments;

            this->linePoints.emplace_back(ringRadius * cosf(t0), ringY, ringRadius * sinf(t0));
            this->linePoints.emplace_back(ringRadius * cosf(t1), ringY, ringRadius * sinf(t1));
        }
    }

    //======================================================================
    // 上半球（上方向にふくらむ）: yTop から極へ
    //======================================================================
    for (int r = 1; r < rings; ++r)
    {
        float a = (DX::PI * 0.5f) * (float)r / (float)rings; // 0→π/2（端点除く）
        float ringRadius = _radius * cosf(a);
        float ringY = yTop + _radius * sinf(a);

        for (int i = 0; i < segments; ++i)
        {
            float t0 = DX::TWO_PI * (float)i / segments;
            float t1 = DX::TWO_PI * (float)(i + 1) / segments;

            this->linePoints.emplace_back(ringRadius * cosf(t0), ringY, ringRadius * sinf(t0));
            this->linePoints.emplace_back(ringRadius * cosf(t1), ringY, ringRadius * sinf(t1));
        }
    }

    //======================================================================
    // 縦ライン（確認用）
    //======================================================================
    for (int i = 0; i < 4; ++i)
    {
        float t = DX::TWO_PI * (float)i / 4.0f;
        float x = _radius * cosf(t);
        float z = _radius * sinf(t);

        this->linePoints.emplace_back(x, yBot, z);
        this->linePoints.emplace_back(x, yTop, z);
    }
}
void ColliderDebugRenderer::DrawInternal(ID3D11DeviceContext* _context)
{
    // シェーダが未取得の場合でもクラッシュは避ける
    if (!this->shaders) { return; }

    if (!this->vertexBuffer) { return; }
    if (!this->collider) { return; }

    // シェーダのバインド
    this->shaders->Bind(*_context);

    // 頂点バッファのバインド
    this->vertexBuffer->Bind(_context, 0);

    // プリミティブトポロジー設定（ラインリスト）
    _context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // Body から直接位置・回転を取得する
    DX::Vector3 bodyPos;
    DX::Quaternion bodyRot;
    this->rigidbody->GetBodyTransform(bodyPos, bodyRot);

    // Transform の行列は使わない（ただし頂点生成時にワールドスケールを適用済み）
    Matrix world = Matrix::CreateFromQuaternion(bodyRot) * Matrix::CreateTranslation(bodyPos);

    // ビュー/プロジェクションはカメラから
    Matrix view = this->camera->GetViewMatrix();
    Matrix proj = this->camera->GetProjectionMatrix();

    auto& render = SystemLocator::Get<RenderSystem>();
    render.SetWorldMatrix(&world);
    render.SetViewMatrix(&view);
    render.SetProjectionMatrix(&proj);

    _context->Draw(static_cast<UINT>(this->linePoints.size()), 0);
}