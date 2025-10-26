//-----------------------------------------------------------------------------
// �萔�o�b�t�@
//-----------------------------------------------------------------------------
cbuffer WorldBuffer : register(b0)
{
    matrix world;
}
cbuffer ViewBuffer : register(b1)
{
    matrix view;
}
cbuffer ProjectionBuffer : register(b2)
{
    matrix projection;
}

//-----------------------------------------------------------------------------
// ���o�͍\����
//-----------------------------------------------------------------------------
struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1; // ���C�e�B���O�p
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ���[���h�ϊ�
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;

    // �r���[�E�v���W�F�N�V�����ϊ�
    float4 viewPos = mul(worldPos, view);
    output.pos = mul(viewPos, projection);

    // �@���ϊ��i��]�����̂ݎg�p�j
    output.normal = normalize(mul((float3x3) world, input.normal));

    return output;
}