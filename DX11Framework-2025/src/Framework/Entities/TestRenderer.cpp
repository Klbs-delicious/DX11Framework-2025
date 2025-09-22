#include "Framework/Entities/TestRenderer.h"
#include "Framework/Core/SystemLocator.h"
#include "Framework/Core/D3D11System.h"
#include "Framework/Core/RenderSystem.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

TestRenderer::TestRenderer(GameObject* owner, bool isActive)
    : Component(owner, isActive)
{
}

TestRenderer::~TestRenderer() {}

void TestRenderer::Initialize()
{
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto device = d3d11.GetDevice();

    Vertex vertices[] = {
        { {-0.5f, -0.5f, 0}, {0,0,1,1} },
        { { 0.5f, -0.5f, 0}, {0,1,0,1} },
        { { 0.0f,  0.5f, 0}, {1,0,0,1} },
    };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &initData, vertexBuffer.GetAddressOf());

    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

#ifdef _DEBUG
    HRESULT hrVS = D3DCompileFromFile(
        L"src/Framework/Graphics/Shaders/VertexShader/VS_Test.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "vs_5_0", 0, 0,
        vsBlob.GetAddressOf(), errorBlob.GetAddressOf());

    HRESULT hrPS = D3DCompileFromFile(
        L"src/Framework/Graphics/Shaders/PixelShader/PS_Test.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main", "ps_5_0", 0, 0,
        psBlob.GetAddressOf(), errorBlob.GetAddressOf());

    if (FAILED(hrVS) || FAILED(hrPS)) {
        if (errorBlob) {
            const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
            std::string errorString(errorMsg, errorBlob->GetBufferSize());
            std::wstring wErrorString(errorString.begin(), errorString.end());
            OutputDebugString(L"[Shader Compilation Error]\n");
            OutputDebugString(wErrorString.c_str());
        }
        else {
            OutputDebugString(L"シェーダーのコンパイルに失敗しましたが、errorBlob に情報がありません。\n");
        }
        return;
    }
#else
    HRESULT hrVS = D3DReadFileToBlob(L"Assets/Shaders/VS_Test.cso", vsBlob.GetAddressOf());
    HRESULT hrPS = D3DReadFileToBlob(L"Assets/Shaders/PS_Test.cso", psBlob.GetAddressOf());

    if (FAILED(hrVS) || FAILED(hrPS)) {
        OutputDebugString(L"[CSO Load Error] シェーダーバイナリの読み込みに失敗しました。\n");
        return;
    }
#endif

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());

    using namespace DirectX::SimpleMath;
    Matrix world = Matrix::Identity;
    render.SetWorldMatrix(&world);

    Matrix view = Matrix::CreateLookAt({ 0, 0, -2 }, { 0, 0, 0 }, { 0, 1, 0 });
    render.SetViewMatrix(&view);

    Matrix proj = Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(45.0f), 640.0f / 480.0f, 0.1f, 100.0f);
    render.SetProjectionMatrix(&proj);

    startTime = std::chrono::steady_clock::now();
}

void TestRenderer::Draw()
{
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();

    auto now = std::chrono::steady_clock::now();
    float timeS = std::chrono::duration<float>(now - startTime).count();

    using namespace DirectX::SimpleMath;
    Matrix world = Matrix::CreateRotationZ(timeS);
    world *= Matrix::CreateTranslation(0, std::sin(timeS) * 0.5f, 0);
    render.SetWorldMatrix(&world);

    auto ctx = d3d11.GetContext();
    UINT stride = sizeof(Vertex), offset = 0;
    ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    ctx->IASetInputLayout(inputLayout.Get());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
    ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
    ctx->Draw(3, 0);
}

void TestRenderer::Dispose()
{
    // ComPtr に任せる
}

