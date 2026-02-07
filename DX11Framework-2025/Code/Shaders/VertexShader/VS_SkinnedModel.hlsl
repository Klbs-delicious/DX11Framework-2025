//-----------------------------------------------------------------------------
// SkinnedModelVS.hlsl (Debug Version)
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
VS_OUT_MODEL main(VS_IN_SKINNED_MODEL _input)
{
    VS_OUT_MODEL output;

    // インデックスの取得
    // アライメント修正後の boneCount を使用する
    uint i0 = (_input.boneIndex.x < boneCount) ? _input.boneIndex.x : 0;
    uint i1 = (_input.boneIndex.y < boneCount) ? _input.boneIndex.y : 0;
    uint i2 = (_input.boneIndex.z < boneCount) ? _input.boneIndex.z : 0;
    uint i3 = (_input.boneIndex.w < boneCount) ? _input.boneIndex.w : 0;

    float4x4 skin = 0.0f;
    skin += mul(_input.boneWeight.x, boneMatrices[i0]);
    skin += mul(_input.boneWeight.y, boneMatrices[i1]);
    skin += mul(_input.boneWeight.z, boneMatrices[i2]);
    skin += mul(_input.boneWeight.w, boneMatrices[i3]);

    // スキニング変換
    float4 localPos = float4(_input.pos, 1.0f);
    float4 skinnedPos = mul(localPos, skin);

    // ワールド・ビュー・プロジェクション変換
    float4 worldPos4 = mul(skinnedPos, world);
    output.pos = mul(mul(worldPos4, view), projection);
    output.worldPos = worldPos4.xyz;

    // 法線もスキニング行列で変形させる
    float3 skinnedNrm = mul(_input.normal, (float3x3) skin);
    output.normal = normalize(mul((float3x3) world, skinnedNrm));

    // テクスチャ座標を渡す
    output.tex = _input.tex;

    return output;
}