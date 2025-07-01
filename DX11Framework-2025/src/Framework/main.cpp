/**	@file	main.cpp
*	@brief 	エントリポイント
*	@date	2025/06/12
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Core/WindowSystem.h"
#include"Framework/Core/RenderSystem.h"
#include "Framework/Utils/DebugHooks.h"

#include <d3dcompiler.h>				
#pragma comment(lib, "d3dcompiler.lib") 

//-----------------------------------------------------------------------------
// Test
//-----------------------------------------------------------------------------

// 三角形の頂点構造体
struct Vertex {
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector4 color;
};

// 三角形の頂点データ
Vertex vertices[] = {
	{ { 0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ {-0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
};

//-----------------------------------------------------------------------------
// EntryPoint
//-----------------------------------------------------------------------------
int main()
{    
	// Debug、Releseで適切なハンドラをセット
	DebugHooks::Install();  

    WindowSystem::Initialize(640, 480);
	RenderSystem::Initialize();

	auto device = D3D11System::GetDevice();

	// RenderSystem実行確認用
	// 頂点バッファ作成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	ComPtr<ID3D11Buffer> vertexBuffer;
	device->CreateBuffer(&vbDesc, &initData, vertexBuffer.GetAddressOf());

	D3D11_INPUT_ELEMENT_DESC layout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,       D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

	// 頂点シェーダのコンパイル
	D3DCompileFromFile(L"src/Framework/Graphics/Shaders/VertexShader/VS_Test.hlsl", nullptr, nullptr,
		"main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());

	// ピクセルシェーダのコンパイル
	D3DCompileFromFile(L"src/Framework/Graphics/Shaders/PixelShader/PS_Test.hlsl", nullptr, nullptr,
		"main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());

	// シェーダ生成
	ComPtr<ID3D11VertexShader> vertexShader;
	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());

	ComPtr<ID3D11PixelShader> pixelShader;
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());

	// 入力レイアウト生成
	ComPtr<ID3D11InputLayout> inputLayout;
	device->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());

	if (vertexShader == nullptr || pixelShader == nullptr) {
		OutputDebugString(L"シェーダが壊れている可能性あり\n");
	}

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// ゲームループなど
		RenderSystem::BeginRender();

		// RenderSystem実行確認用に三角形を描画
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		auto context = D3D11System::GetContext();
		context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// シェーダをバインド
		context->IASetInputLayout(inputLayout.Get());
		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader.Get(), nullptr, 0);

		// 実際に描画
		context->Draw(3, 0);

		RenderSystem::EndRender();
	}

	RenderSystem::Finalize();
    WindowSystem::Finalize();
    return 0;
}