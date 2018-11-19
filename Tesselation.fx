//--------------------------------------------------------------------------------------
// File: Tesselation.fx
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register (b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 worldPos : POSITION0;
};

struct HS_IO
{
	float4 Pos:POSITION0;
	float4 worldPos:POSITION1;
	float3 Norm:NORMAL;
	float2 Tex:TEXCOORD0;
};

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PassThroughConstantsHS")]
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

