cbuffer WorldBuffer : register(b0)
{
    matrix world;
}
cbuffer ViewBuffer : register(b1)
{
    matrix view;
}
cbuffer ProjectionBuffer : register(b2)
{
    matrix projection;
}

struct VS_IN
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
    
struct PS_IN
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

//struct MATERIAL
//{
//    float4 ambient;
//    float4 diffuse;
//    float4 specular;
//    float4 emission;
//    float shininess;
//    bool textureEnable;
//    float2 dummy;
//};

//cbuffer MaterialBuffer : register(b3)
//{
//    MATERIAL material;
//}

//struct LIGHT
//{
//    bool enable; // Žg—p‚·‚é‚©”Û‚©
//    bool3 dummy; // PADDING
//    float4 direction; // •ûŒü
//    float4 diffuse; // ŠgŽU”½ŽË—p‚ÌŒõ‚Ì‹­‚³
//    float4 ambient; // ŠÂ‹«Œõ—p‚ÌŒõ‚Ì‹­‚³
//};

//cbuffer LightBuffer : register(b4)
//{
//    LIGHT light;
//};

//#define MAX_BONE 400
//cbuffer BoneMatrixBuffer : register(b5)
//{
//    matrix boneMatrix[MAX_BONE];
//}

//struct VS_IN
//{
//    float4 Position : POSITION0;
//    float4 Normal : NORMAL0;
//    float4 Diffuse : COLOR0;
//    float2 TexCoord : TEXCOORD0;
//};

//struct VSONESKIN_IN
//{
//    float4 Position : POSITION0;
//    float4 Normal : NORMAL0;
//    float4 Diffuse : COLOR0;
//    float2 TexCoord : TEXCOORD0;
//    int4 BoneIndex : BONEINDEX;
//    float4 BoneWeight : BONEWEIGHT;
//};

//struct PS_IN
//{
//    float4 Position : SV_POSITION;
//    float4 Diffuse : COLOR0;
//    float2 TexCoord : TEXCOORD0;
//};
