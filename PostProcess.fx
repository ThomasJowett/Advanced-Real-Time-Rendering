//--------------------------------------------------------------------------------------
// File: PostProcess.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D vingette : register(t1);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;
};

struct Light
{
	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightPosW;
};

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;

	float HeightMapScale;
	int MaxSamples;
	int MinSamples;
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
	output.PosH.z = 0.0f;
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// No Post Process Pixel Shader
//--------------------------------------------------------------------------------------

float4 NoPostProcessPS(VS_OUTPUT input) : SV_Target
{
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	float4 vingetteColour = vingette.Sample(samLinear, input.Tex);

	float4 finalColour = lerp(0.0f, textureColour, vingetteColour + 0.5f);
	//finalColour += vingetteColour/2;
	return finalColour;
}

//------------------------------------------------------------------------------------
// Gaussian Blur Pixel Shader
//--------------------------------------------------------------------------------------
float4 GaussianBlurPS(VS_OUTPUT input) : SV_Target
{
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

	float2 position = { 0.0f,0.01f };
	textureColour += txDiffuse.Sample(samLinear, input.Tex + position);
	float2 position2 = { 0.0f, -0.01f };
	textureColour += txDiffuse.Sample(samLinear, input.Tex + position2);
	float2 position3 = { 0.001f, 0.0f };
	textureColour += txDiffuse.Sample(samLinear, input.Tex + position3);
	float2 position4 = { -0.001f, 0.0f };
	textureColour += txDiffuse.Sample(samLinear, input.Tex + position4);

	textureColour /= 5;

	return textureColour;
}