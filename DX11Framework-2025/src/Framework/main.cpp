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
#include <chrono>

// 三角形の頂点構造体
struct Vertex {
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector4 color;
};

// 三角形の頂点データ
Vertex vertices[] = {
	{ {-0.5f, -0.5f, 0}, {0,0,1,1} },
	{ { 0.5f, -0.5f, 0}, {0,1,0,1} },
	{ { 0.0f,  0.5f, 0}, {1,0,0,1} },
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
	HRESULT hr = D3DCompileFromFile(L"src/Framework/Graphics/Shaders/VertexShader/VS_Test.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), errorBlob.GetAddressOf());

	// ピクセルシェーダのコンパイル
	D3DCompileFromFile(L"src/Framework/Graphics/Shaders/PixelShader/PS_Test.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), errorBlob.GetAddressOf());

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			// エラーメッセージを文字列として取り出す
			const char* errorMsg = static_cast<const char*>(errorBlob->GetBufferPointer());
			std::string errorString(errorMsg, errorBlob->GetBufferSize());

			// WideChar に変換してデバッグ出力
			std::wstring wErrorString(errorString.begin(), errorString.end());
			OutputDebugString(L"[Shader Compilation Error]\n");
			OutputDebugString(wErrorString.c_str());
		}
		else
		{
			OutputDebugString(L"シェーダのコンパイルに失敗しましたが、errorBlob に情報がありません。\n");
		}
	}

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

	// ループ前に一度だけ行列をCPUで作成して、各CBに転送
	{
		using namespace DirectX::SimpleMath;

		// ワールド行列
		Matrix world = Matrix::Identity;
		RenderSystem::SetWorldMatrix(&world);

		// ビュー行列
		Matrix view = Matrix::CreateLookAt(
			{ 0, 0, -2 },   // eye
			{ 0, 0,  0 },   // at
			{ 0, 1,  0 }    // up
		);
		RenderSystem::SetViewMatrix(&view);

		// プロジェクション行列
		Matrix proj = Matrix::CreatePerspectiveFieldOfView(
			DirectX::XMConvertToRadians(45.0f),
			640.0f / 480.0f,
			0.1f, 100.0f
		);
		RenderSystem::SetProjectionMatrix(&proj);
	}
	// 時間計測スタート
	auto startTime = std::chrono::steady_clock::now();


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


		// RenderSystem実行確認用に三角形を動かす処理
		// 経過時間（秒）を取得
		auto now = std::chrono::steady_clock::now();
		float timeS = std::chrono::duration<float>(now - startTime).count();

		// World 行列：Z 軸回転 + 少し上下移動
		using namespace DirectX::SimpleMath;
		Matrix world = Matrix::CreateRotationZ(timeS);              // 回転
		world *= Matrix::CreateTranslation(0, std::sin(timeS) * 0.5f, 0);  // 縦揺れ
		// 更新を GPU に通知
		RenderSystem::SetWorldMatrix(&world);

		// 頂点バッファ/シェーダ/Draw（例）
		auto ctx = D3D11System::GetContext();
		UINT stride = sizeof(Vertex), offset = 0;
		ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		ctx->IASetInputLayout(inputLayout.Get());
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
		ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
		ctx->Draw(3, 0);


		RenderSystem::EndRender();
	}

	RenderSystem::Finalize();
    WindowSystem::Finalize();
    return 0;
}