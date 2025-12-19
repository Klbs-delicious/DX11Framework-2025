//-----------------------------------------------------------------------------
// ModelPS_Fog.hlsl
// 3Dモデル用 疑似フォグ対応ピクセルシェーダー
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

float4 main(VS_OUT_MODEL input) : SV_TARGET
{
    // ベースカラー取得
    float4 texColor = tex.Sample(samp, input.tex);

    // カメラからの距離（ワールド空間）
    float dist = length(input.worldPos - cameraPos);

    // 線形フォグ係数
    float fogFactor = saturate((dist - fogStart) / (fogEnd - fogStart));

    // フォグ適用
    float3 finalRGB = lerp(texColor.rgb, fogColor, fogFactor);

    return float4(finalRGB, texColor.a);
}