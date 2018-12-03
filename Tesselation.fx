//--------------------------------------------------------------------------------------
// File: Tesselation.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txDisplacement : register(t2);
Texture2D txShadow : register(t3);

SamplerState samLinear : register(s0);

SamplerComparisonState samShadow :register(s1);

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
	float3 LightDir;
};

cbuffer ConstantBuffer : register (b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	matrix ShadowTransform;

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

struct PS_DISPLACEMENT_INPUT
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float3 PosW : POSITION;
	float3 LightVecT : POSITION2;
	float3 EyeVecT : POSITION3;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

struct VS_INPUT
{
	float4 PosL : POSITION0;
	float3 NormL : NORMAL;
	float3 Tangent : TANGENT;
	float2 Tex : TEXCOORD0;
};

struct HS_IO
{
	float4 Pos:POSITION0;
	float4 worldPos:POSITION1;
	float3 Norm:NORMAL;
	float3 Tangent : TANGENT;
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
	output.Tangent = ip[i].Tangent;
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
	float tesselationFactor = 5.0f;
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
PS_DISPLACEMENT_INPUT DisplacementDS(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_IO, 3> TrianglePatch)
{
	PS_DISPLACEMENT_INPUT output;

	//Interpolate world space position with barycentric coordinates
	float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].Pos
		+ BarycentricCoordinates.y * TrianglePatch[1].Pos
		+ BarycentricCoordinates.z * TrianglePatch[2].Pos;

	output.PosH = float4(vWorldPos.xyz, 1.0);

	float2 vTex = BarycentricCoordinates.x * TrianglePatch[0].Tex
		+ BarycentricCoordinates.y * TrianglePatch[1].Tex
		+ BarycentricCoordinates.z * TrianglePatch[2].Tex;

	output.Tex = vTex;

	output.NormW = BarycentricCoordinates.x * TrianglePatch[0].Norm
		+ BarycentricCoordinates.y * TrianglePatch[1].Norm
		+ BarycentricCoordinates.z * TrianglePatch[2].Norm;

	vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].worldPos
		+ BarycentricCoordinates.y * TrianglePatch[1].worldPos
		+ BarycentricCoordinates.z * TrianglePatch[2].worldPos;

	float3 Tangent = BarycentricCoordinates.x * TrianglePatch[0].Tangent
		+ BarycentricCoordinates.y * TrianglePatch[1].Tangent
		+ BarycentricCoordinates.z * TrianglePatch[2].Tangent;

	float displacement = txDisplacement.SampleLevel(samLinear, output.Tex.xy, 0).r;

	displacement *= HeightMapScale;

	vWorldPos += -output.NormW * displacement;

	output.PosW = float4(vWorldPos.xyz, 1.0);

	output.PosH = mul(output.PosH, World);
	output.PosH = mul(output.PosH, View);
	output.PosH = mul(output.PosH, Projection);

	float3x3 tbnMatrix;
	tbnMatrix[0] = normalize(mul(float4(Tangent, 0.0f), World).xyz);
	tbnMatrix[1] = normalize(mul(float4(cross(output.NormW, Tangent), 0.0f), World).xyz);
	tbnMatrix[2] = normalize(output.NormW);

	float3 EyeVecW = (EyePosW - output.PosW).xyz;
	
	float3 LightVecW = (light.LightDir).xyz;

	output.LightVecT = normalize(mul(tbnMatrix, LightVecW));
	output.EyeVecT = normalize(mul(tbnMatrix, EyeVecW));

	//output.ShadowPosH = mul(output.PosW, ShadowTransform);

	return output;
}

float4 TesselationPS(PS_INPUT input) : SV_Target
{
	return surface.DiffuseMtrl;
}

float4 DisplacementPS(PS_DISPLACEMENT_INPUT input) : SV_Target
{
	float3 normalW = normalize(input.NormW);
}