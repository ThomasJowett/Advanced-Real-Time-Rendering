//--------------------------------------------------------------------------------------
// File: SkinnedMesh.fx
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

static int MAX_BONES = 50;

cbuffer ConstantBuffer : register(b1)
{
    float4x4 WorldMatrixArray[50] : WORLDMATRIXARRAY;
    float4x4 ViewProjection;

    matrix ShadowTransform;
}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 BlendWeights : BLENDWEIGHT;
    float4 BlendIndices : BLENDINDICES;
    float3 Normal : NORMAL;
    float3 Tex0 : TEXCOORD0;
    float3 Tangent : TANGENT;
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
    VS_OUTPUT output = (VS_OUTPUT) 0;

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

    float4 posW = input.Pos;
    output.PosW = posW.xyz;

    //Pos = input.Pos;
    //Normal = input.Normal;

    // transform position from world space into view and then projection space
    output.PosH = mul(float4(Pos.xyz, 1.0f), ViewProjection);

    output.NormW = float4(Normal.xyz, 1.0f);

    output.TangentW = float4(input.Tangent, 0.0f);

    output.ShadowPosH = mul(posW, ShadowTransform);
    output.Tex = input.Tex0;

	return output;
}
