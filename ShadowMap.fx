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

    float4 WorldFrustumPlanes[6];

    float3 EyePosW;

    float MinDist;
    float MaxDist;

    float MinTess;
    float MaxTess;
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
		if (AABBBehindPlaneTest(center, extents, frustumPlanes[i]))
		{
			return true;
		}
	}

	return false;
}

struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[4] : SV_TessFactor;
	float Inside[2] : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PatchHS(InputPatch<VS_TERRAIN_OUTPUT, 4> ip, uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

	//
	//Frustum Cull
	//

	//We store the patch BounsY in th first control point
	float minY = ip[0].BoundsY.x;
	float maxY = ip[0].BoundsY.y;

	float3 vMin = float3(ip[2].PosW.x, minY, ip[2].PosW.z);
	float3 vMax = float3(ip[1].PosW.x, maxY, ip[1].PosW.z);

	float3 boxCenter = 0.5f * (vMin + vMax);
	float3 boxExtents = 10.0f * (vMax - vMin);
	if (AABBOutsideFrustumTest(boxCenter, boxExtents, WorldFrustumPlanes))
	{
		output.Edges[0] = 0.0f;
		output.Edges[1] = 0.0f;
		output.Edges[2] = 0.0f;
		output.Edges[3] = 0.0f;

		output.Inside[0] = 0.0f;
		output.Inside[1] = 0.0f;

		return output;
	}
	//
	// Do normal tessellation based on distance.
	//
	else
	{
		// It is important to do the tess factor calculation based on the
		// edge properties so that edges shared by more than one patch will
		// have the same tessellation factor.  Otherwise, gaps can appear.

		// Compute midpoint on edges, and patch center
		float3 e0 = 0.5f * (ip[0].PosW + ip[2].PosW);
		float3 e1 = 0.5f * (ip[0].PosW + ip[1].PosW);
		float3 e2 = 0.5f * (ip[1].PosW + ip[3].PosW);
		float3 e3 = 0.5f * (ip[2].PosW + ip[3].PosW);
		float3 c = 0.25f * (ip[0].PosW + ip[1].PosW + ip[2].PosW + ip[3].PosW);

		output.Edges[0] = CalcTessFactor(e0);
		output.Edges[1] = CalcTessFactor(e1);
		output.Edges[2] = CalcTessFactor(e2);
		output.Edges[3] = CalcTessFactor(e3);

		output.Inside[0] = CalcTessFactor(c);
		output.Inside[1] = output.Inside[0];

		return output;
	}
}

struct HS_OUTPUT
{
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchHS")]
[maxtessfactor(64.0f)]

HS_OUTPUT TerrainHS(InputPatch<VS_TERRAIN_OUTPUT, 4>ip, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
	HS_OUTPUT output;

	output.PosW = ip[i].PosW;
	output.Tex = ip[i].Tex;

	return output;
}

struct DS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

[domain("quad")]
DS_OUTPUT TerrainDS(HS_CONSTANT_DATA_OUTPUT patchTess, float2 uv : SV_DomainLocation, const OutputPatch <HS_OUTPUT, 4>quad)
{
	DS_OUTPUT output;

	// Bilinear interpolation.
	float3 PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y);

	output.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y);

	// Displacement mapping
	PosW.y = txHeightMap.SampleLevel(samLinear, output.Tex, 0).r;

	// Project to homogeneous clip space.
	output.PosH = float4(PosW, 1.0f);
	output.PosH = mul(output.PosH, View);
	output.PosH = mul(output.PosH, Projection);

	return output;
}