/** @file   MeshManager.cpp
 *  @date   2025/11/05
 */

 //-----------------------------------------------------------------------------
 // Includes
 //-----------------------------------------------------------------------------
#include "Include/Framework/Graphics/MeshManager.h"
#include "Include/Framework/Graphics/PrimitiveMeshData.h"

#include "Include/Framework/Core/SystemLocator.h"
#include "Include/Framework/Core/ResourceHub.h"

#include <iostream>

//-----------------------------------------------------------------------------
// MeshManager class
//-----------------------------------------------------------------------------

MeshManager::MeshManager() : d3d11System(SystemLocator::Get<D3D11System>()), shaderManager(ResourceHub::Get<ShaderManager>()), defaultMesh(nullptr)
{
    // デフォルトメッシュを生成する（Box）
    this->defaultMesh = this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Box);
    std::cout << "[MeshManager] Created default mesh (Box).\n";

	// プリミティブなメッシュの事前登録
    this->Register("Box", this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Box).release());
    this->Register("Plane", this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Plane).release());
    this->Register("Sphere", this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Sphere).release());
	this->Register("Capsule", this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Capsule).release());
}

Graphics::Mesh* MeshManager::Register(const std::string& _key)
{
    // 既に存在する場合はそのまま返す
    auto it =this-> meshTable.find(_key);
    if (it != this->meshTable.end()) { return it->second.get(); }    

    // 新規メッシュを生成して登録
    auto mesh = std::make_unique<Graphics::Mesh>();
    Graphics::Mesh* ptr = mesh.get();
    this->meshTable[_key] = std::move(mesh);

    std::cout << "[MeshManager] Registered mesh: " << _key << std::endl;
    return ptr;
}

void MeshManager:: Register(const std::string& _key, Graphics::Mesh* _mesh)
{
    if (!_mesh) { return; }

    // 所有を移す or 参照保持に切替
    this->meshTable[_key] = std::unique_ptr<Graphics::Mesh>(_mesh);
}

void MeshManager::Unregister(const std::string& _key)
{
    auto it = this->meshTable.find(_key);
    if (it != this->meshTable.end())
    {
        this->meshTable.erase(it);
        std::cout << "[MeshManager] Unregistered mesh: " << _key << std::endl;
    }
}

Graphics::Mesh* MeshManager::Get(const std::string& _key)
{
    auto it = this->meshTable.find(_key);
    if (it != this->meshTable.end()) { return it->second.get(); }
        
    return nullptr;
}

Graphics::Mesh* MeshManager::Default() const
{
    return this->defaultMesh.get();
}

void MeshManager::Clear()
{
    this->meshTable.clear();
    this->defaultMesh.reset();
    this->defaultMesh = this->CreatePrimitiveMesh(Graphics::Primitives::PrimitiveType::Box);
    std::cout << "[MeshManager] Cleared and recreated default mesh.\n";
}

/**@brief プリミティブなメッシュを生成する
 * @param _type
 * @return
 */
std::unique_ptr<Graphics::Mesh> MeshManager::CreatePrimitiveMesh(const Graphics::Primitives::PrimitiveType _type)
{
    const std::vector<Graphics::ModelVertexGPU>* vertices = nullptr;
    const std::vector<uint32_t>* indices = nullptr;

    std::vector<Graphics::ModelVertexGPU> dynamicVertices;
    std::vector<uint32_t> dynamicIndices;

    // --- プリミティブなメッシュの頂点・インデックスデータを取得する ---
    switch (_type)
    {
    case Graphics::Primitives::PrimitiveType::Box:
        vertices = &Graphics::Primitives::Box::Vertices;
        indices = &Graphics::Primitives::Box::Indices;
        break;

    case Graphics::Primitives::PrimitiveType::Plane:
        vertices = &Graphics::Primitives::Plane::Vertices;
        indices = &Graphics::Primitives::Plane::Indices;
        break;

    case Graphics::Primitives::PrimitiveType::Sphere:
        dynamicVertices = Graphics::Primitives::Sphere::CreateVertices();
        dynamicIndices = Graphics::Primitives::Sphere::CreateIndices();
        vertices = &dynamicVertices;
        indices = &dynamicIndices;
        break;

    case Graphics::Primitives::PrimitiveType::Capsule:
        dynamicVertices = Graphics::Primitives::Capsule::CreateVertices();
        dynamicIndices = Graphics::Primitives::Capsule::CreateIndices();
        vertices = &dynamicVertices;
        indices = &dynamicIndices;
        break;

    default:
        return nullptr;
    }

    // --- GPUリソースを生成する ---
    auto mesh = std::make_unique<Graphics::Mesh>();
    auto vb = std::make_unique<VertexBuffer>();
    auto ib = std::make_unique<IndexBuffer>();
    auto device = this->d3d11System.GetDevice();

    vb->Create(
        device, 
        vertices->data(), 
        sizeof(Graphics::ModelVertexGPU),
        static_cast<UINT>(vertices->size()),
        false
    );
    ib->Create(
        device, 
        indices->data(), 
        sizeof(uint32_t),
        static_cast<UINT>(indices->size())
    );

    // --- Subsetの作成 ---
    std::vector<Graphics::MeshSubset> subsets(1);
    subsets[0].indexStart = 0;
    subsets[0].indexCount = static_cast<UINT>(indices->size());
    subsets[0].vertexBase = 0;
    subsets[0].vertexCount = static_cast<UINT>(vertices->size());
    subsets[0].materialIndex = -1;

    mesh->SetVertexBuffer(std::move(vb));
    mesh->SetIndexBuffer(std::move(ib));
    mesh->SetSubsets(std::move(subsets));

    return mesh;
}

/** @brief モデルデータからメッシュを生成する
 *  @param _modelData 読み込んだモデルデータ
 *  @return 生成されたメッシュ
 */
std::unique_ptr<Graphics::Mesh> MeshManager::CreateFromModelData(
    const Graphics::Import::ModelData& _modelData)
{
    auto mesh = std::make_unique<Graphics::Mesh>();

    // --- Subset 構築 ---
    std::vector<Graphics::MeshSubset> subsets;
    subsets.reserve(_modelData.subsets.size());

    for (const auto& subset : _modelData.subsets)
    {
        Graphics::MeshSubset s{};
        s.indexStart = subset.indexBase;
        s.indexCount = subset.indexNum;
        s.vertexBase = subset.vertexBase;
        s.vertexCount = subset.vertexNum;
        s.materialIndex = subset.materialIndex;
        subsets.push_back(s);
    }
    mesh->SetSubsets(std::move(subsets));

    // --- 頂点／インデックス統合 ---
    std::vector<Graphics::ModelVertexGPU> vertexData;
    std::vector<uint32_t> indexData;

    size_t totalVerts = 0, totalIdx = 0;
    for (size_t i = 0; i < _modelData.vertices.size(); ++i)
    {
        totalVerts += _modelData.vertices[i].size();
        totalIdx += _modelData.indices[i].size();
    }
    vertexData.reserve(totalVerts);
    indexData.reserve(totalIdx);

    uint32_t vertexOffset = 0;
    for (size_t meshIndex = 0; meshIndex < _modelData.vertices.size(); ++meshIndex)
    {
        const auto& verts = _modelData.vertices[meshIndex];
        const auto& idx = _modelData.indices[meshIndex];

        for (const auto& v : verts)
        {
            Graphics::ModelVertexGPU gpu{};
            gpu.position = { v.pos.x, v.pos.y, v.pos.z };
            gpu.normal = { v.normal.x, v.normal.y, v.normal.z };
            gpu.texcoord = { v.texCoord.x, v.texCoord.y };
            vertexData.push_back(gpu);
        }

        for (const auto& i : idx)
            indexData.push_back(i + vertexOffset);

        vertexOffset += static_cast<uint32_t>(verts.size());
    }

    // --- GPUバッファ生成 ---
    auto device = this->d3d11System.GetDevice();

    auto vb = std::make_unique<VertexBuffer>();
    vb->Create(device, vertexData.data(),
        sizeof(Graphics::ModelVertexGPU),
        static_cast<UINT>(vertexData.size()),
        false);
    mesh->SetVertexBuffer(std::move(vb));

    auto ib = std::make_unique<IndexBuffer>();
    ib->Create(device, indexData.data(),
        sizeof(uint32_t),
        static_cast<UINT>(indexData.size()));
    mesh->SetIndexBuffer(std::move(ib));

    return mesh;
}