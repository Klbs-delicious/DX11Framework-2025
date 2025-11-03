//-----------------------------------------------------------------------------
// 定数バッファ
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
    matrix projection;
}

//-----------------------------------------------------------------------------
// 入出力構造体
//-----------------------------------------------------------------------------
struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1; // ライティング用
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ワールド変換
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;

    // ビュー・プロジェクション変換
    float4 viewPos = mul(worldPos, view);
    output.pos = mul(viewPos, projection);

    // 法線変換（回転成分のみ使用）
    output.normal = normalize(mul((float3x3) world, input.normal));

    return output;
}