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

    // この法線変換は「等倍スケール前提」
    // 非一様スケールを使う場合は逆転置行列に切り替えること
    output.normal = mul(input.normal, (float3x3) world);

    // テクスチャ座標
    output.tex = input.tex;

    return output;
}