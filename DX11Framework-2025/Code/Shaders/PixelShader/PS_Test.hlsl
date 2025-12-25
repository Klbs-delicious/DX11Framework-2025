#include "../Common.hlsli"

float4 main(VS_OUT_SPRITE pin) : SV_TARGET
{
    float4 color = tex.Sample(samp, pin.uv);
    // 乗算カラー（頂点色）とマテリアルの Diffuse を適用
    return color * pin.color * Diffuse; 
}