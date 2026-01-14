//-----------------------------------------------------------------------------
// SkinnedModelVS.hlsl
// スキニング対応頂点シェーダ（row vector 前提：mul(v, M)）
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

/** @brief 行列をスカラー倍する
 *  @param _matrix スカラー倍する行列
 *  @param _scalar スカラー値
 */
static float4x4 ScaleMatrix(float4x4 _matrix, float _scalar)
{
    return float4x4(
        _matrix[0] * _scalar,
        _matrix[1] * _scalar,
        _matrix[2] * _scalar,
        _matrix[3] * _scalar
    );
}

/** @brief 単位行列を返す
 *  @return 単位行列
 */
static float4x4 Identity4x4()
{
    return float4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
VS_OUT_MODEL main(VS_IN_SKINNED_MODEL _input)
{
    VS_OUT_MODEL output;

    uint4 idxU = _input.boneIndex;
    float4 weight = _input.boneWeight;

    // weight 合計が 0 ならスキニング無効（頂点を潰さない）
    float wsum = weight.x + weight.y + weight.z + weight.w;

    // boneCount が 0 ならスキニング無効
    bool disableSkin = (boneCount == 0u) || (wsum <= 0.0f);

    float4x4 skin = Identity4x4();

    // スキニング計算
    if (!disableSkin)
    {
        uint maxIdx = boneCount - 1u;
        idxU = min(idxU, uint4(maxIdx, maxIdx, maxIdx, maxIdx));

        int i0 = (int) idxU.x;
        int i1 = (int) idxU.y;
        int i2 = (int) idxU.z;
        int i3 = (int) idxU.w;

        skin =
              ScaleMatrix(boneMatrices[i0], weight.x)
            + ScaleMatrix(boneMatrices[i1], weight.y)
            + ScaleMatrix(boneMatrices[i2], weight.z)
            + ScaleMatrix(boneMatrices[i3], weight.w);
    }

    // スキニング後の頂点位置・法線
    float4 localPos = mul(float4(_input.pos, 1.0f), skin);
    float3 localNrm = mul(_input.normal, (float3x3) skin);

    float4 worldPos4 = mul(localPos, world);
    float4 viewPos4 = mul(worldPos4, view);
    float4 clipPos4 = mul(viewPos4, projection);

    output.pos = clipPos4;
    output.worldPos = worldPos4.xyz;
    output.tex = _input.tex;

    output.normal = normalize(mul(localNrm, (float3x3) world));

    return output;
}