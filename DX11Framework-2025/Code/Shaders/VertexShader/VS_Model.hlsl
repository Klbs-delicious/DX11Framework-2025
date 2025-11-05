//-----------------------------------------------------------------------------
// ModelTestVS.hlsl
// 頂点シェーダー：基本的なモデル変換＋ライト計算用データ準備
//-----------------------------------------------------------------------------

cbuffer WorldBuffer : register(b0)
{
    matrix world;
}
cbuffer ViewBuffer : register(b1)
{
    matrix view;
}
cbuffer ProjectionBuffer : register(b2)
{
    matrix proj;
}


cbuffer MaterialBuffer : register(b3)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shiness;
    bool TextureEnable;
    float2 Dummy;
};

// 入力頂点構造（C++の ModelVertexGPU に対応）
struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

// ピクセルシェーダーへ送るデータ
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION1;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ワールド座標変換
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(mul(worldPos, view), proj);
    output.worldPos = worldPos.xyz;

    // 法線をワールド空間に変換（正規化）
    output.normal = normalize(mul(float4(input.normal, 0.0f), world).xyz);

    output.tex = input.tex;
    return output;
}
