//--------------------------------------------------------------------------------------
// File: Tesselation.fx
//--------------------------------------------------------------------------------------

Texture2D txDisplacement : register(t0);
SamplerState samLinear : register(s0);

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

cbuffer ConstantBuffer : register (b0)
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
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float3 worldPos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct VS_INPUT
{
	float4 PosL : POSITION0;
	float3 NormL : NORMAL;
	float3 TangentL : TANGENT;
	float2 Tex : TEXCOORD0;
};

struct HS_IO
{
	float4 Pos:POSITION0;
	float4 worldPos:POSITION1;
	float3 Norm:NORMAL;
	float2 Tex:TEXCOORD0;
};

HS_IO TesselationVS(VS_INPUT input)
{
	HS_IO output;

	output.Pos = input.PosL;

	output.worldPos = mul(input.PosL,World);

	float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	output.Norm = normalize(normalW);

	output.Tex = input.Tex;

	return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PassThroughConstantHS")]
[maxtessfactor(7.0)]

HS_IO MainHS(InputPatch<HS_IO, 3> ip, uint i : SV_OutputControlPointID,	uint PatchID : SV_PrimitiveID)
{
	HS_IO output;

	output.Pos = ip[i].Pos;
	output.worldPos = ip[i].worldPos;
	output.Norm = ip[i].Norm;
	output.Tex = ip[i].Tex;

	return output;
}

struct HS_CONSTANT_DATA_OUTPUT
{
	float Edges[3] : SV_TessFactor;
	float Inside   : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PassThroughConstantHS(InputPatch<HS_IO, 3> ip, uint PatchID : SV_PrimitiveID)
{
	float tesselationFactor = 2.0f;
	HS_CONSTANT_DATA_OUTPUT output;
	output.Edges[0] = tesselationFactor;
	output.Edges[1] = tesselationFactor;
	output.Edges[2] = tesselationFactor;
	output.Inside = tesselationFactor;
	return output;
}

//Domain Shader
[domain("tri")]
PS_INPUT DSMAIN(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_IO, 3> TrianglePatch)
{
	PS_INPUT output;

	//Interpolate world space position with barycentric coordinates
	float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].Pos
		+ BarycentricCoordinates.y * TrianglePatch[1].Pos
		+ BarycentricCoordinates.z * TrianglePatch[2].Pos;

	output.Pos = float4(vWorldPos.xyz, 1.0);

	float2 vTex = BarycentricCoordinates.x * TrianglePatch[0].Tex
		+ BarycentricCoordinates.y * TrianglePatch[1].Tex
		+ BarycentricCoordinates.z * TrianglePatch[2].Tex;

	output.Tex = vTex;

	output.Norm = BarycentricCoordinates.x * TrianglePatch[0].Norm
		+ BarycentricCoordinates.y * TrianglePatch[1].Norm
		+ BarycentricCoordinates.z * TrianglePatch[2].Norm;

	vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].worldPos
		+ BarycentricCoordinates.y * TrianglePatch[1].worldPos
		+ BarycentricCoordinates.z * TrianglePatch[2].worldPos;

	output.worldPos = float4(vWorldPos.xyz, 1.0);

	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	return output;
}

[domain("tri")]
PS_INPUT DisplacementDS(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_IO, 3> TrianglePatch)
{
	PS_INPUT output;

	//Interpolate world space position with barycentric coordinates
	float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].Pos
		+ BarycentricCoordinates.y * TrianglePatch[1].Pos
		+ BarycentricCoordinates.z * TrianglePatch[2].Pos;

	output.Pos = float4(vWorldPos.xyz, 1.0);

	float2 vTex = BarycentricCoordinates.x * TrianglePatch[0].Tex
		+ BarycentricCoordinates.y * TrianglePatch[1].Tex
		+ BarycentricCoordinates.z * TrianglePatch[2].Tex;

	output.Tex = vTex;

	output.Norm = BarycentricCoordinates.x * TrianglePatch[0].Norm
		+ BarycentricCoordinates.y * TrianglePatch[1].Norm
		+ BarycentricCoordinates.z * TrianglePatch[2].Norm;

	vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].worldPos
		+ BarycentricCoordinates.y * TrianglePatch[1].worldPos
		+ BarycentricCoordinates.z * TrianglePatch[2].worldPos;

	float displacement = txDisplacement.SampleLevel(samLinear, output.Tex.xy, 0).r;

	displacement *= HeightMapScale;

	vWorldPos += -output.Norm * displacement;

	output.worldPos = float4(vWorldPos.xyz, 1.0);

	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	return output;
}

float4 TesselationPS(PS_INPUT input) : SV_Target
{
	return surface.DiffuseMtrl;
}