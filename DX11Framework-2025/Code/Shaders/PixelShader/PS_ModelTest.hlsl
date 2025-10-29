cbuffer LightBuffer : register(b3)
{
    float3 lightDir; // ワールド空間でのライト方向（例: {0.5, -1.0, 0.3}）
    float4 baseColor; // オブジェクトの基本色
}

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

float4 main(PS_IN input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDir);

    // 拡散光 (Lambert)
    float diffuse = saturate(dot(N, L));

    // 簡易アンビエント＋拡散
    float3 color = baseColor.rgb * (0.2 + 0.8 * diffuse);

    return float4(color, 1.0f);
}