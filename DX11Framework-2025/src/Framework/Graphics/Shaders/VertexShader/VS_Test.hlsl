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

    // World �� View �� Projection �̏��ŕϊ�
    float4 pos = mul(float4(vin.position, 1.0f), world);
    pos = mul(pos, view);
    vout.position = mul(pos, projection);

    // �J���[�͂��̂܂܃p�X�X���[
    vout.color = vin.color;
    return vout;
}