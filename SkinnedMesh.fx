//--------------------------------------------------------------------------------------
// File: SkinnedMesh.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

SamplerState samlinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

static int MAX_BONES = 50;

cbuffer ConstantBuffer : register(b0)
{
    float4x4 WorldMatrixArray[50] : WORLDMATRIXARRAY;
    float4x4 ViewProjection;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 BlendWeights : BLENDWEIGHT;
    float4 BlendIndices : BLENDINDICES;
    float3 Normal : NORMAL;
    float3 Tex0 : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float4 TangentW : TANGENT;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

VS_OUTPUT SkinnedVS(VS_INPUT input)
{
    VS_OUTPUT output;

    // cast the vectors to arrays for use in the for loop below
    int4 IndexVector = D3DCOLORtoUBYTE4(input.BlendIndices); // convert from float4 to int4
    int IndexArray[4] = (int[4]) IndexVector; // then to an array
    float BlendWeightsArray[4] = (float[4]) input.BlendWeights; // convert to array

    float3 Pos = float3(0.0f, 0.0f, 0.0f);
    float3 Normal = float3(0.0f, 0.0f, 0.0f);

    for (int iBone = 0; iBone < 4; iBone++)
    {	
        Pos += mul(input.Pos, WorldMatrixArray[IndexArray[iBone]]) * BlendWeightsArray[iBone];
        Normal += mul(input.Normal, WorldMatrixArray[IndexArray[iBone]]) * BlendWeightsArray[iBone];
    }

    // transform position from world space into view and then projection space
    output.PosH = mul(float4(Pos.xyz, 1.0f), ViewProjection);

    output.Tex = input.Tex0;

	return output;
}
