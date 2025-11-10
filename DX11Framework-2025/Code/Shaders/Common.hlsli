//-----------------------------------------------------------------------------
// ModelCommon.hlsl
// 共通定数バッファ・構造体定義
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 定数バッファ
//-----------------------------------------------------------------------------

cbuffer WorldBuffer : register(b0)
{
    matrix world;       // ワールド変換行列
}

cbuffer ViewBuffer : register(b1)
{
    matrix view;        // ビュー行列
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix projection;  // プロジェクション行列
}

cbuffer MaterialBuffer : register(b3)
{
    float4 Ambient;     // 環境光反射成分
    float4 Diffuse;     // 拡散反射成分
    float4 Specular;    // 鏡面反射成分
    float4 Emission;    // 自己発光成分
    float Shiness;      // 鋭さ（鏡面反射強度）
    bool TextureEnable; // テクスチャ使用フラグ
    float2 Dummy;       // アライメント調整
}

cbuffer LightBuffer : register(b4)
{
    float3 lightDir;    // ワールド空間でのライト方向（ライト→モデル方向）
    float pad1;
    float4 baseColor;   // ライトの基本色
}

//-----------------------------------------------------------------------------
// 入出力構造体
//-----------------------------------------------------------------------------

struct VS_IN
{
    float3 pos : POSITION;  // 頂点位置
    float3 normal : NORMAL; // 法線
    float2 tex : TEXCOORD0; // テクスチャ座標
};

struct VS_OUT
{
    float4 pos : SV_POSITION;       // クリップ空間座標
    float3 worldPos : POSITION1;    // ワールド座標
    float3 normal : NORMAL;         // 法線
    float2 tex : TEXCOORD0;         // テクスチャ座標
};

struct PS_IN
{
    float4 pos : SV_POSITION;       // クリップ空間座標
    float3 worldPos : POSITION1;    // ワールド座標
    float3 normal : NORMAL;         // 法線
    float2 tex : TEXCOORD0;         // テクスチャ座標
};

//-----------------------------------------------------------------------------
// リソース
//-----------------------------------------------------------------------------

Texture2D tex : register(t0);       // テクスチャリソース
SamplerState samp : register(s0);   // サンプラーステート

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
//    bool enable; // 使用するか否か
//    bool3 dummy; // PADDING
//    float4 direction; // 方向
//    float4 diffuse; // 拡散反射用の光の強さ
//    float4 ambient; // 環境光用の光の強さ
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
