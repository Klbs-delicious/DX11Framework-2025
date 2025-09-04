#include "../Common.hlsli"

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