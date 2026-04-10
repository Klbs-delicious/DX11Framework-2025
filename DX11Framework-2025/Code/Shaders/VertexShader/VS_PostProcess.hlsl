struct VS_OUT
{
    float4 pos : SV_POSITION;   // 画面上の位置
    float2 uv : TEXCOORD0;      // テクスチャを参照するための座標
};

VS_OUT main(uint vID : SV_VertexID)
{
    VS_OUT output;

    // 頂点ID（0, 1, 2）を利用して、画面を覆う巨大な三角形を生成
    output.uv = float2((vID << 1) & 2, vID & 2);
    output.pos = float4(output.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);

    return output;
}