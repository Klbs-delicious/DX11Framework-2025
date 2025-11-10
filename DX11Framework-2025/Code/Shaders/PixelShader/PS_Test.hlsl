#include "../Common.hlsli"

float4 main(PS_IN pin) : SV_TARGET
{
    //float4 color = tex.Sample(samp, pin.uv);
    //color *= pin.color;
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}