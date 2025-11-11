#include "../Common.hlsli"

VS_OUT_SPRITE main(VS_IN_SPRITE input)
{
    VS_OUT_SPRITE output;

    // モデル空間 → ワールド空間
    float4 worldPos = mul(float4(input.pos, 1.0f), world);

    // ワールド空間 → ビュー空間
    float4 viewPos = mul(worldPos, view);

    // ビュー空間 → スクリーン空間（プロジェクション）
    output.pos = mul(viewPos, projection);

    // 色はそのまま渡す
    output.color = input.color;
    
    // テクスチャ
    output.uv = input.uv;

    return output;
}