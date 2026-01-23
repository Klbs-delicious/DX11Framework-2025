//-----------------------------------------------------------------------------
// SkinnedModelVS.hlsl
// スキニング対応頂点シェーダ（row vector 前提：mul(v, M)）
// - BoneBuffer(b7) に入っている boneMatrices は CPU 側で Transpose 済み想定
// - そのため VS 側は「mul(v, boneMatrix)」で良い
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
VS_OUT_MODEL main(VS_IN_SKINNED_MODEL In)
{
    VS_OUT_MODEL Out;

    //--------------------------------------------------------------------------
    // 1) スキニング行列を作る（最大4本、row-vector 運用）
    //--------------------------------------------------------------------------
    float4x4 skin = 0.0f;

    // boneCount を見て範囲外アクセスを避ける（未使用 0 はそのまま 0 番を参照する）
    uint i0 = (In.boneIndex.x < boneCount) ? In.boneIndex.x : 0;
    uint i1 = (In.boneIndex.y < boneCount) ? In.boneIndex.y : 0;
    uint i2 = (In.boneIndex.z < boneCount) ? In.boneIndex.z : 0;
    uint i3 = (In.boneIndex.w < boneCount) ? In.boneIndex.w : 0;

    skin += boneMatrices[i0] * In.boneWeight.x;
    skin += boneMatrices[i1] * In.boneWeight.y;
    skin += boneMatrices[i2] * In.boneWeight.z;
    skin += boneMatrices[i3] * In.boneWeight.w;

    //--------------------------------------------------------------------------
    // 2) 位置・法線をスキニング
    //--------------------------------------------------------------------------
    float4 localPos = float4(In.pos, 1.0f);
    float4 localNrm = float4(In.normal, 0.0f);

    float4 skinnedPos = mul(localPos, skin);
    float4 skinnedNrm = mul(localNrm, skin);

    //--------------------------------------------------------------------------
    // 3) ワールド・ビュー・射影（row-vector）
    //--------------------------------------------------------------------------
    float4 worldPos4 = mul(skinnedPos, world);
    float4 viewPos4 = mul(worldPos4, view);
    float4 clipPos4 = mul(viewPos4, projection);

    Out.pos = clipPos4;
    Out.worldPos = worldPos4.xyz;

    // ワールド法線は NormalMatrixBuffer(b6) を使用（row-vector 3本）
    // 正規化まで行う
    float3 n = skinnedNrm.xyz;
    float3 wn;
    wn.x = dot(n, row0);
    wn.y = dot(n, row1);
    wn.z = dot(n, row2);
    Out.normal = normalize(wn);

    Out.tex = In.tex;

    return Out;
}