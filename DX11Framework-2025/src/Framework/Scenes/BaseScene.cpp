/**	@file	BaseScene.cpp
*	@date	2025/07/03
*/

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include"Framework/Scenes/BaseScene.h"

#include"Framework/Core/SystemLocator.h"
#include"Framework/Core/InputSystem.h"
#include"Scenes/SceneManager.h"

#include"Framework/Entities/TestComponent.h"

#include<iostream>

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
	{ {-0.5f, -0.5f, 0}, {0,0,1,1} },
	{ { 0.5f, -0.5f, 0}, {0,1,0,1} },
	{ { 0.0f,  0.5f, 0}, {1,0,0,1} },
};

//-----------------------------------------------------------------------------
// BaseScene Class
//-----------------------------------------------------------------------------

/** @brief コンストラクタ
 *  @param GameObjectManager&	_gameObjectManager	ゲームオブジェクトの管理
 */
BaseScene::BaseScene(GameObjectManager& _gameObjectManager) :gameObjectManager(_gameObjectManager) {}

/// @brief	デストラクタ
BaseScene::~BaseScene() {}

/**	@brief		ゲームオブジェクトの初期化処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Initialize()
{
    // 初期化
    this->gameObjectManager.FlushInitialize();

    // テストなのでメンバに持たずに直接取得する
    auto& d3d11 = SystemLocator::Get<D3D11System>();
    auto& render = SystemLocator::Get<RenderSystem>();
    auto device = d3d11.GetDevice();

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    device->CreateBuffer(&vbDesc, &initData, vertexBuffer.GetAddressOf());

    // 入力レイアウト定義
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // シェーダー読み込み・生成
    {
        ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

        // DEBUGビルドでは .hlsl をコンパイル、Releaseビルドでは .cso を読み込む
#ifdef _DEBUG
    // .hlslファイルからコンパイル（開発用）
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
    // .csoファイルから読み込み（本番用）
        HRESULT hrVS = D3DReadFileToBlob(L"Assets/Shaders/VS_Test.cso", vsBlob.GetAddressOf());
        HRESULT hrPS = D3DReadFileToBlob(L"Assets/Shaders/PS_Test.cso", psBlob.GetAddressOf());

        if (FAILED(hrVS) || FAILED(hrPS)) {
            OutputDebugString(L"[CSO Load Error] シェーダーバイナリの読み込みに失敗しました。\n");
            return;
        }
#endif

        // シェーダー生成
        auto device = SystemLocator::Get<D3D11System>().GetDevice();
        device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
        device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());

        // 入力レイアウト定義
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                          D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3,          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        UINT numElements = ARRAYSIZE(layout);

        // 入力レイアウト生成
        device->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), inputLayout.GetAddressOf());

        if (vertexShader == nullptr || pixelShader == nullptr) {
            OutputDebugString(L"シェーダーが壊れている可能性あり\n");
        }
    }


    // 行列の初期設定
    using namespace DirectX::SimpleMath;

    Matrix world = Matrix::Identity;
    render.SetWorldMatrix(&world);

    Matrix view = Matrix::CreateLookAt(
        { 0, 0, -2 },
        { 0, 0,  0 },
        { 0, 1,  0 }
    );
    render.SetViewMatrix(&view);

    Matrix proj = Matrix::CreatePerspectiveFieldOfView(
        DirectX::XMConvertToRadians(45.0f),
        640.0f / 480.0f,
        0.1f, 100.0f
    );
    render.SetProjectionMatrix(&proj);

    // 時間計測スタート
    this->startTime = std::chrono::steady_clock::now();
}

/**	@brief		オブジェクトの更新を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Update(float _deltaTime) 
{
    // 未初期化オブジェクトの初期化
    this->gameObjectManager.FlushInitialize();

    // オブジェクトの一括更新
    this->gameObjectManager.UpdateAll(_deltaTime);

	// テスト的にメンバに持たずに直接取得する
	auto& input = SystemLocator::Get<InputSystem>();
	auto& scenemanager = SystemLocator::Get<SceneManager>();

	if (input.IsActionPressed("Space")) { std::cout << "Space：Press" << std::endl; }
	if (input.IsActionTriggered("Space")) { std::cout << "Space：Trigger" << std::endl; }

    if (input.IsActionTriggered("SceneChangeTest")) { scenemanager.RequestSceneChange(SceneType::Test); }
    if (input.IsActionTriggered("SceneChangeTitle")) { scenemanager.RequestSceneChange(SceneType::Title); }

}

/**	@brief		ゲームオブジェクトの描画処理を行う
 *	@param		float _deltaTime	デルタタイム
 *	@details	継承を禁止する
 */
void BaseScene::Draw()
{
    // オブジェクトの一括描画
    this->gameObjectManager.DrawAll();

	// テストなのでメンバに持たずに直接取得する
	auto& d3d11 = SystemLocator::Get< D3D11System>();
	auto& render = SystemLocator::Get< RenderSystem>();

	// RenderSystem実行確認用に三角形を動かす処理
	// 経過時間（秒）を取得
	auto now = std::chrono::steady_clock::now();
	float timeS = std::chrono::duration<float>(now - startTime).count();

	// World 行列：Z 軸回転 + 少し上下移動
	using namespace DirectX::SimpleMath;
	Matrix world = Matrix::CreateRotationZ(timeS);              // 回転
	world *= Matrix::CreateTranslation(0, std::sin(timeS) * 0.5f, 0);  // 縦揺れ
	// 更新を GPU に通知
	render.SetWorldMatrix(&world);

	// 頂点バッファ/シェーダ/Draw（例）
	auto ctx = d3d11.GetContext();
	UINT stride = sizeof(Vertex), offset = 0;
	ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetInputLayout(inputLayout.Get());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(vertexShader.Get(), nullptr, 0);
	ctx->PSSetShader(pixelShader.Get(), nullptr, 0);
	ctx->Draw(3, 0);
}

/**	@brief		終了処理を行う
 *	@details	継承を禁止する
 */
void BaseScene::Finalize()
{
    // オブジェクトを削除
    this->gameObjectManager.Dispose();
}