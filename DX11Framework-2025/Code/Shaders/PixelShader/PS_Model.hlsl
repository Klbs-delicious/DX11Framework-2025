//-----------------------------------------------------------------------------
// PS_Model.hlsl
// ピクセルシェーダー：ハーフランバート＋Phong＋テクスチャ対応
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
    float3 lightDir; // ワールド空間でのライト方向（ライト→モデル方向）
    float pad1;
    float4 baseColor;
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

// ---------------------------------------------
// 彩度補正（Saturation調整）
// ---------------------------------------------
float3 AdjustSaturation(float3 color, float saturation)
{
    // NTSC係数（輝度成分）
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    return lerp(gray.xxx, color, saturation);
}

//------------------------------------------------------
// メイン
//------------------------------------------------------
float4 main(PS_IN input) : SV_TARGET
{
    // 法線・ライト方向を正規化
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDir); // 負方向にして「光が当たる方向」
    float3 V = normalize(-input.worldPos);
    float3 R = reflect(-L, N);

    // ハーフランバートによる柔らかい拡散反射
    float diff = saturate(dot(N, L) * 0.5f + 0.5f);

    // 鏡面反射（Phong）
    float spec = pow(max(dot(R, V), 0.0f), Shiness);

    // テクスチャカラー（無ければピンク）
    float4 texColor = Diffuse;
    if (TextureEnable)
    {
        texColor *= albedoMap.Sample(samLinear, input.tex);
        if (dot(texColor.rgb, texColor.rgb) < 1e-5)
        {
            texColor = float4(1, 0, 1, 1);
        }
    }

    // 環境光・拡散・鏡面を合成
    float4 finalColor =
        (Ambient * 0.4f) +
        (texColor * diff * baseColor) +
        (Specular * spec) +
        (Emission);

    finalColor.a = 1.0f;

    // 彩度を少し強めに（例：1.2倍）
    finalColor.rgb = AdjustSaturation(finalColor.rgb, 1.2f);

    return saturate(finalColor);
}
