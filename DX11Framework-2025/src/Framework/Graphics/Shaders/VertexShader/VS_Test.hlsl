#include "../Common.hlsli"

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ���f����� �� ���[���h���
    float4 worldPos = mul(float4(input.position, 1.0f), world);

    // ���[���h��� �� �r���[���
    float4 viewPos = mul(worldPos, view);

    // �r���[��� �� �X�N���[����ԁi�v���W�F�N�V�����j
    output.position = mul(viewPos, projection);

    // �F�͂��̂܂ܓn��
    output.color = input.color;

    return output;
}