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

// 疑似フォグ用定数バッファ（時間でレンジ揺らぎ対応）
cbuffer FogBuffer : register(b5)
{
    float3 cameraPos;   // カメラのワールド座標
    float fogStart;     // フォグ開始距離
    float fogEnd;       // フォグ終了距離
    float3 fogColor;    // フォグ色（RGB）

    // レンジ揺らぎ用
    float timeSec;      // 経過時間（秒）
    float waveSpeed;    // 揺らぎ速度
    float waveAmp;      // 揺らぎ強度
    float fogPad;       // アラインメント
}

// 法線行列（逆転置 3x3）: 行ベクトル3本
cbuffer NormalMatrixBuffer : register(b6)
{
    float3 row0;
    float3 row1;
    float3 row2;
    float  normalPad; // 16バイトアラインメント用
}

// スキニング用定数バッファ
cbuffer BoneBuffer : register(b7)
{
    uint boneCount;
    float3 pad; // 16バイト境界に合わせる
    float4x4 boneMatrices[128];
}

//-----------------------------------------------------------------------------
// 通常モデル描画用構造体
//-----------------------------------------------------------------------------
struct VS_IN_MODEL
{
    float3 pos : POSITION;  // 頂点位置
    float3 normal : NORMAL; // 法線
    float2 tex : TEXCOORD0; // テクスチャ座標
};

struct VS_OUT_MODEL
{
    float4 pos : SV_POSITION;       // クリップ空間座標
    float3 worldPos : POSITION1;    // ワールド座標
    float3 normal : NORMAL;         // 法線
    float2 tex : TEXCOORD0;         // テクスチャ座標
};

struct PS_IN_MODEL
{
    float4 pos : SV_POSITION;       // クリップ空間座標
    float3 worldPos : POSITION1;    // ワールド座標
    float3 normal : NORMAL;         // 法線
    float2 tex : TEXCOORD0;         // テクスチャ座標
};

// ----------------------------------------------------------------------------
// スキニングモデル描画用構造体
// ----------------------------------------------------------------------------
struct VS_IN_SKINNED_MODEL
{
    float3 pos : POSITION;          // 頂点位置
    float3 normal : NORMAL;         // 法線
    float2 tex : TEXCOORD0;         // テクスチャ座標
    uint4 boneIndex : BONEINDEX;    // ボーンインデックス（未使用の場合は 0 が入る）
    float4 boneWeight : BONEWEIGHT; // ボーンウェイト
};

//-----------------------------------------------------------------------------
// スプライト描画用構造体
//-----------------------------------------------------------------------------
struct VS_IN_SPRITE
{
    float3 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct VS_OUT_SPRITE
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// デバッグライン描画用構造体
//----------------------------------------------------------------------------- 

struct VS_IN_DEBUGLINE
{
    float3 pos : POSITION;
};

struct VS_OUT_DEBUGLINE
{
    float4 pos : SV_POSITION;
};

//-----------------------------------------------------------------------------
// リソース
//-----------------------------------------------------------------------------
Texture2D tex : register(t0);       // テクスチャ（モデル・スプライト共通）
SamplerState samp : register(s0);   // サンプラー（共通）