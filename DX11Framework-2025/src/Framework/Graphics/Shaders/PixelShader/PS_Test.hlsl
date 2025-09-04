#include "../Common.hlsli"

float4 main(PS_IN pin) : SV_TARGET
{
    return pin.color;
}