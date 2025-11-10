//-----------------------------------------------------------------------------
// VS_Model.hlsl
// 頂点シェーダー：ワールド変換＋法線変換＋ライト計算用データ出力
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

//------------------------------------------------------
// メイン
//------------------------------------------------------
VS_OUT main(VS_IN input)
{
    VS_OUT output;

    // ワールド・ビュー・プロジェクション
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    float4 viewPos = mul(worldPos, view);
    output.pos = mul(viewPos, projection);

    // ワールド空間での法線（正規化して補間させる）
    output.normal = normalize(mul(input.normal, (float3x3) world));

    // ワールド座標
    output.worldPos = worldPos.xyz;

    // テクスチャ座標
    output.tex = input.tex;

    return output;
}