//-----------------------------------------------------------------------------
// Debug Line Pixel Shader
//-----------------------------------------------------------------------------

cbuffer DebugLineBuffer : register(b0)
{
    float4 lineColor; // ラインの色
}

float4 main() : SV_TARGET
{
    // デバッグ用ワイヤーフレーム色
    return lineColor;
}