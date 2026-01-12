//-----------------------------------------------------------------------------
// ModelPS_Fog.hlsl
// 疑似フォグ（レンジ揺らぎ：fogEndのみ変化）
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

float4 main(PS_IN_MODEL input) : SV_TARGET
{
    float4 texColor = tex.Sample(samp, input.tex);

    // 基本距離
    float dist = length(input.worldPos - cameraPos);

    // fogEnd を時間で揺らす（色は固定）
    float wave = 0.5 + 0.5 * sin(timeSec * waveSpeed);
    float mod = lerp(1.0 - waveAmp, 1.0 + waveAmp, wave);

    float dynamicEnd = fogStart + (fogEnd - fogStart) * mod;
    float denom = max(dynamicEnd - fogStart, 1e-3);
    float fogFactor = saturate((dist - fogStart) / denom);

    float3 finalRGB = lerp(texColor.rgb, fogColor, fogFactor);
    return float4(finalRGB, texColor.a);
}