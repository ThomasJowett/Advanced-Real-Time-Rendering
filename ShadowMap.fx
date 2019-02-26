//--------------------------------------------------------------------------------------
// File: ShadowMap.fx
//--------------------------------------------------------------------------------------

Texture2D txHeightMap : register(t1);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float3 TangentL : TANGENT;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosH :SV_POSITION;
	float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Pass ShadowMap Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT ShadowMapVS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 posW = mul(input.PosL, World);
	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);

	output.Tex = input.Tex;

	return output;
}

struct VS_TERRAIN_INPUT
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
    float2 BoundsY : TEXCOORD;
};

struct VS_TERRAIN_OUTPUT
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

VS_TERRAIN_OUTPUT ShadowMapTerrainVS(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;

    output.PosW = input.PosL;

    output.PosW.y = txHeightMap.SampleLevel(samLinear, input.Tex, 0).r;

    output.Tex = input.Tex;
    output.BoundsY = input.BoundsY;

    return output;
};

