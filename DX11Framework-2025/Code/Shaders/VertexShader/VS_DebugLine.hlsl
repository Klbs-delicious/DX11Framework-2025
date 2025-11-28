//-----------------------------------------------------------------------------
// Debug Line Vertex Shader
//-----------------------------------------------------------------------------
#include "../Common.hlsli"

VS_OUT_DEBUGLINE main(VS_IN_DEBUGLINE input)
{
    VS_OUT_DEBUGLINE output;

    float4 pos = float4(input.pos, 1.0f);

    pos = mul(pos, world);
    pos = mul(pos, view);
    pos = mul(pos, projection);

    output.pos = pos;
    return output;
}