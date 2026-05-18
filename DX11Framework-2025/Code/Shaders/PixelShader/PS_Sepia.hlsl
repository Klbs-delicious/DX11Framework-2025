//-----------------------------------------------------------------------------
// SepiaPS.hlsl
//-----------------------------------------------------------------------------
Texture2D g_SceneTexture : register(t0);
SamplerState g_Sampler : register(s0);

// セピア調のパラメータを格納する定数バッファ
cbuffer SepiaParams : register(b0)
{
    float g_Intensity; // セピアの適用度 (0.0 ～ 1.0)
    float3 g_Padding; // パディング
};

// ポストプロセス用の入力構造体
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Pixel Shader
//---------------------------------------------------   --------------------------
float4 main(PS_INPUT input) : SV_TARGET
{
    // 元のカラーをサンプリング
    float4 sceneColor = g_SceneTexture.Sample(g_Sampler, input.uv);

    // 輝度計算 (NTSC係数)
    float gray = dot(sceneColor.rgb, float3(0.299, 0.587, 0.114));

    // セピア調の色味を計算
    float3 sepiaColor;
    sepiaColor.r = gray * 1.0;
    sepiaColor.g = gray * 0.9;
    sepiaColor.b = gray * 0.7;

    // g_Intensityを使い、元の色とセピア色を線形補間する。
    // 0.0で元の色、1.0でセピア色。
    float3 finalColor = lerp(sceneColor.rgb, sepiaColor, g_Intensity);

    return float4(finalColor, sceneColor.a);
}