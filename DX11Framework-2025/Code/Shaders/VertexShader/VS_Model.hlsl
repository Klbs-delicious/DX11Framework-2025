//-----------------------------------------------------------------------------
// VS_Model.hlsl
// 頂点シェーダー：ワールド変換＋法線変換＋ライト計算用データ出力
//-----------------------------------------------------------------------------

cbuffer WorldBuffer : register(b0)
{
    matrix world;
};
cbuffer ViewBuffer : register(b1)
{
    matrix view;
};
cbuffer ProjectionBuffer : register(b2)
{
    matrix proj;
};

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION1;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

//------------------------------------------------------
// メイン
//------------------------------------------------------
VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ワールド・ビュー・プロジェクション
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    output.pos = mul(viewPos, proj);

    // ワールド空間での法線（正規化して補間させる）
    output.normal = normalize(mul(input.normal, (float3x3) world));

    // ワールド座標
    output.worldPos = worldPos.xyz;

    // テクスチャ座標
    output.tex = input.tex;

    return output;
}