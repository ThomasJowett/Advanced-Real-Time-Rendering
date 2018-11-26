//--------------------------------------------------------------------------------------
// File: ShadowMap.fx
//--------------------------------------------------------------------------------------

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