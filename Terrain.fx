//--------------------------------------------------------------------------------------
// File: Terrain.fx
//--------------------------------------------------------------------------------------

Texture2D txBlendMap : register(t0);
Texture2D txHeightMap : register(t1);
Texture2D txLayer0 : register(t2);
Texture2D txLayer1 : register(t3);
Texture2D txLayer2 : register(t4);
Texture2D txLayer3 : register(t5);
Texture2D txLayer4 : register(t6);

SamplerState samLinear : register(s0);

SamplerState samHeightMap : register(s1);

struct VertexIn
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

struct VertexOut
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

