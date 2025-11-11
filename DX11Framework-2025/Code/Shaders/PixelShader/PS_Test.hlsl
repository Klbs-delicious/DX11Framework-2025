#include "../Common.hlsli"

float4 main(VS_OUT_SPRITE pin) : SV_TARGET
{
    float4 color = tex.Sample(samp, pin.uv);
    return color * pin.color; 
}