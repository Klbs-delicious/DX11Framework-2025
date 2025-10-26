cbuffer LightBuffer : register(b3)
{
    float3 lightDir; // ���[���h��Ԃł̃��C�g�����i��: {0.5, -1.0, 0.3}�j
    float4 baseColor; // �I�u�W�F�N�g�̊�{�F
}

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

//float4 main(PS_IN input) : SV_TARGET
//{
//    float3 N = normalize(input.normal);
//    float3 L = normalize(-lightDir);

//    // �g�U�� (Lambert)
//    float diffuse = saturate(dot(N, L));

//    // �ȈՃA���r�G���g�{�g�U
//    float3 color = baseColor.rgb * (0.2 + 0.8 * diffuse);

//    return float4(color, 1.0f);
//}

float4 main(PS_IN input) : SV_TARGET
{
    return float4(1, 0, 0, 1); // �Ԉ�F
}