//--------------------------------------------------------------------------------------
// File: PostProcess.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D vingette : register(t1);

SamplerState samLinear : register(s0);

static const int MAX_SAMPLES = 16;

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer Parameters : register(b0)
{
	float4 sampleOffsets[MAX_SAMPLES];
	float4 sampleWeights[MAX_SAMPLES];
};

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
    //return float4(input.Tex, 1.0f, 1.0f);
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	float4 vingetteColour = vingette.Sample(samLinear, input.Tex);

	float4 finalColour = lerp(0.0f, textureColour, vingetteColour + 0.5f);
	//finalColour += vingetteColour/2;
    return textureColour;
}

//------------------------------------------------------------------------------------
// Gaussian Blur Pixel Shader
//--------------------------------------------------------------------------------------
float4 GaussianBlurPS(VS_OUTPUT input) : SV_Target
{
	float4 finalColour = 0.0f;

	for (int i = 0; i < 13; i++)
	{
		finalColour += sampleWeights[i] * txDiffuse.Sample(samLinear, input.Tex + sampleOffsets[i].xy);
	}

	return finalColour;
}

//--------------------------------------------------------------------------------------
// Bloom Pixel Shader
//--------------------------------------------------------------------------------------
float4 BloomPS(VS_OUTPUT input) : SV_Target0
{
	// Uses sampleWeights[0] as 'bloom threshold'
	float4 c = txDiffuse.Sample(samLinear, input.Tex);
	return saturate((c - sampleWeights[0]) / (1 - sampleWeights[0]));
}