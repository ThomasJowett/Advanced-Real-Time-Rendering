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

cbuffer ConstantBuffer : register (b0)
{
	float3 EyePosW;

	float MinDist;
	float MaxDist;

	float MinTess;
	float MaxTess;

	float TexelCellSpaceU;
	float TexelCellSpaceV;
	float WorldCellSpace;
	float2 TexScale = 50.0f;

	float4 WorldFrustumPlanes[6];
}

struct VS_INPUT
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

struct VS_OUTPUT
{
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 BoundsY : TEXCOORD1;
};

VS_OUTPUT TerrainVS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.PosW = input.PosL;

	output.PosW.y = txHeightMap.SampleLevel(samHeightmap, input.Tex, 0).r;

	output.Tex = input.Tex;
	output.BoundsY = input.BoundsY;

	return output;
}

float CalcTessFactor(float3 p)
{
	float d = distance(p, EyePosW);

	float s = saturate((d - MinDist) / (MaxDist - MinDist));

	return pow(2, (lerp(MaxTess, MinTess, s)));
}

bool AABBBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
	float3 n = abs(plane.xyz);

	float r = dot(extents, n);

	float s = dot(float4(center, 1.0f), plane);

	return (s + r) < 0.0f;
}

bool AABBOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6])
{
	for (int i = 0; i < 6; i++)
	{
		if (AABBBehindPlaneTest(center, extents, frustum[i]))
		{
			return true;
		}
	}

	return;
}

struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[3] : SV_TessFactor;
	float Inside : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PatchHS(InputPatch<VS_OUTPUT, 3> ip, uint patchID : SV_PrimitiveID)