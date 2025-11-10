#include "../Common.hlsli"

VS_OUT main(VS_IN input) 
{
    VS_OUT output = (VS_OUT) 0;

    //// モデル空間 → ワールド空間
    //float4 worldPos = mul(float4(input.position, 1.0f), world);

    //// ワールド空間 → ビュー空間
    //float4 viewPos = mul(worldPos, view);

    //// ビュー空間 → スクリーン空間（プロジェクション）
    //output.position = mul(viewPos, projection);

    //// 色はそのまま渡す
    //output.color = input.color;
    
    //// テクスチャ
    //output.uv = input.uv;

    return output;
}