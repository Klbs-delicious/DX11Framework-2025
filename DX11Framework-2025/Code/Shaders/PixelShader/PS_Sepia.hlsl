//-----------------------------------------------------------------------------
// SepiaPS.hlsl
//-----------------------------------------------------------------------------

Texture2D g_SceneTexture : register(t0);
SamplerState g_Sampler : register(s0);

// ポストプロセス用の入力構造体
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_TARGET
{
    // 共通定義のサンプラー(s0)を使用してサンプリング
    float4 color = g_SceneTexture.Sample(g_Sampler, input.uv);

    // 輝度計算 (NTSC係数)
    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));

    // セピア調の色調補正 (固定値)
    // パラメータ化を後回しにするため、ここで色味を決定
    float3 sepia;
    sepia.r = gray * 1.0;
    sepia.g = gray * 0.9;
    sepia.b = gray * 0.7;

    return float4(sepia, color.a);
}