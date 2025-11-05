//-----------------------------------------------------------------------------
// ModelTestPS.hlsl
// ピクセルシェーダー：簡易Phongライティング＋テクスチャ対応
//-----------------------------------------------------------------------------

cbuffer MaterialBuffer : register(b1)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shiness;
    bool TextureEnable;
    float2 Dummy;
};

cbuffer LightBuffer : register(b3)
{
    float3 lightDir; // ワールド空間でのライト方向
    float pad1;
    float4 baseColor; // ライトの色
};

Texture2D albedoMap : register(t0);
SamplerState samLinear : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION1;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET
{
    // 正規化された法線・ライト
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDir);
    float3 V = normalize(-input.worldPos);
    float3 R = reflect(-L, N);

    // 拡散反射
    float diff = max(dot(N, L), 0.0f);

    // 鏡面反射
    float spec = pow(max(dot(R, V), 0.0f), Shiness);

    // 基本色（テクスチャ or Diffuse）
    float4 texColor = Diffuse;
    if (TextureEnable)
    {
        texColor *= albedoMap.Sample(samLinear, input.tex);
    }

    // 最終カラー計算（環境光＋拡散＋鏡面＋自己発光）
    float4 finalColor =
        Ambient * 0.2f +
        texColor * diff * baseColor +
        Specular * spec +
        Emission;

    finalColor.a = 1.0f;
    return saturate(finalColor);
}
