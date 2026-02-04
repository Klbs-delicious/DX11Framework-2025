//-----------------------------------------------------------------------------
// SkinnedModelVS.hlsl (Debug Version)
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
VS_OUT_MODEL main(VS_IN_SKINNED_MODEL In)
{
    VS_OUT_MODEL Out;

    // --- 1) インデックスの取得 ---
    // アライメント修正後の boneCount を使用
    uint i0 = (In.boneIndex.x < boneCount) ? In.boneIndex.x : 0;
    uint i1 = (In.boneIndex.y < boneCount) ? In.boneIndex.y : 0;
    uint i2 = (In.boneIndex.z < boneCount) ? In.boneIndex.z : 0;
    uint i3 = (In.boneIndex.w < boneCount) ? In.boneIndex.w : 0;

    float4x4 skin = 0.0f;
    skin += mul(In.boneWeight.x, boneMatrices[i0]);
    skin += mul(In.boneWeight.y, boneMatrices[i1]);
    skin += mul(In.boneWeight.z, boneMatrices[i2]);
    skin += mul(In.boneWeight.w, boneMatrices[i3]);

    // lerp は消す
    float4 localPos = float4(In.pos, 1.0f);
    float4 skinnedPos = mul(localPos, skin); // ベクトルが左

    // ワールド・ビュー・プロジェクション変換
    float4 worldPos4 = mul(skinnedPos, world);
    Out.pos = mul(mul(worldPos4, view), projection);
    Out.worldPos = worldPos4.xyz;
    
    // 法線もスキニング行列で変形させる
    float3 skinnedNrm = mul(In.normal, (float3x3) skin);
    Out.normal = normalize(mul((float3x3) world, skinnedNrm));
    
    // テクスチャ座標を渡す
    Out.tex = In.tex;

    return Out;
}