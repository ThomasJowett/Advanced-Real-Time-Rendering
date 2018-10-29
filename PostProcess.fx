//--------------------------------------------------------------------------------------
// File: PostProcess.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register(b0)
{

}

struct VS_INPUT
{
	float4 PosH: POSITION;
	float2 Tex: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Pass Through Vertex Shader
//--------------------------------------------------------------------------------------

VS_OUTPUT PassThroughVS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.PosH = input.PosH;
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// No Post Process Pixel Shader
//--------------------------------------------------------------------------------------

float4 NoPostProcessPS(VS_OUTPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex);
}