#include "../Common.hlsli"

struct VS_IN
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;

    // World → View → Projection の順で変換
    float4 pos = mul(float4(vin.position, 1.0f), world);
    pos = mul(pos, view);
    vout.position = mul(pos, projection);

    // カラーはそのままパススルー
    vout.color = vin.color;
    return vout;
}