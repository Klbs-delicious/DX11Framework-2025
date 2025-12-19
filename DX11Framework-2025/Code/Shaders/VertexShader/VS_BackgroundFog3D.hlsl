//-----------------------------------------------------------------------------
// 3Dモデル用 頂点シェーダー
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

VS_OUT_MODEL main(VS_IN_MODEL input)
{
    VS_OUT_MODEL output;

    // モデル空間 → ワールド空間
    float4 worldPos = mul(float4(input.pos, 1.0f), world);

    // ワールド空間 → ビュー空間
    float4 viewPos = mul(worldPos, view);

    // ビュー空間 → クリップ空間
    output.pos = mul(viewPos, projection);

    // 疑似フォグ用：ワールド座標を渡す
    output.worldPos = worldPos.xyz;

    // 法線変換：CPU送信の逆転置3x3（行ベクトル3本）を使用
    float3x3 normalMatrix = float3x3(
        row0.x, row0.y, row0.z,
        row1.x, row1.y, row1.z,
        row2.x, row2.y, row2.z
    );
    output.normal = mul(input.normal, normalMatrix);

    // テクスチャ座標
    output.tex = input.tex;

    return output;
}