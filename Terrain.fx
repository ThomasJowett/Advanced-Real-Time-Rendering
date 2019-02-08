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

    matrix View;
    matrix Projection;

    SurfaceInfo surface;
    Light light;
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

    output.PosW.y = txHeightMap.SampleLevel(samHeightMap, input.Tex, 0).r;

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

HS_CONSTANT_DATA_OUTPUT PatchHS(InputPatch<VS_OUTPUT, 4> ip, uint patchID : SV_PrimitiveID)
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
    float3 boxExtents = 0.5f * (vMax - vMin);
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

HS_OUTPUT MainHS(InputPatch<VS_OUTPUT, 4>ip, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID)
{
    HS_OUTPUT output;

    output.PosW = ip[i].PosW;
    output.Tex = ip[i].Tex;

    return output;
}

struct DS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TiledTex : TEXCOORD1;
};

[domain("quad")]
DS_OUTPUT DS(HS_CONSTANT_DATA_OUTPUT patchTess, float2 uv : SV_DomainLocation, const OutputPatch <HS_OUTPUT, 4>quad)
{
    DS_OUTPUT output;

    // Bilinear interpolation.
    output.PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y);
	
    output.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y);
		
	// Tile layer textures over terrain.
    output.TiledTex = output.Tex * TexScale;
	
	// Displacement mapping
    output.PosW.y = txHeightMap.SampleLevel(samHeightMap, output.Tex, 0).r;
	
	// Project to homogeneous clip space.
    output.PosH = mul(float4(output.PosW, 1.0f), View * Projection);
	
    return output;
}

float4 TerrainPS(DS_OUTPUT input) :SV_Target
{
    float2 leftTex = input.Tex + float2(TexelCellSpaceU, 0.0f);
    float2 rightTex = input.Tex + float2(TexelCellSpaceU, 0.0f);
    float2 bottomTex = input.Tex + float2(0.0f, TexelCellSpaceV);
    float2 topTex = input.Tex + float2(0.0f, -TexelCellSpaceV);

    float leftY = txHeightMap.SampleLevel(samHeightMap, leftTex, 0).r;
    float rightY = txHeightMap.SampleLevel(samHeightMap, rightTex, 0).r;
    float bottomY = txHeightMap.SampleLevel(samHeightMap, bottomTex, 0).r;
    float topY = txHeightMap.SampleLevel(samHeightMap, topTex, 0).r;

    float3 tangent = normalize(float3(2.0f * WorldCellSpace, rightY - leftY, 0.0f));
    float3 bitangent = normalize(float3(0.0f, bottomY -topY, 2.0f * WorldCellSpace));
    float3 normalW = cross(tangent, bitangent);

    float3 toEye = EyePosW - input.PosW;

    float distToEye = length(toEye);

    toEye /= distToEye;

    //
    //Texturing
    //

    //Sample Layers textures
    float c0 = txLayer0.Sample(samLinear, input.TiledTex);
    float c1 = txLayer1.Sample(samLinear, input.TiledTex);
    float c2 = txLayer2.Sample(samLinear, input.TiledTex);
    float c3 = txLayer3.Sample(samLinear, input.TiledTex);
    float c4 = txLayer4.Sample(samLinear, input.TiledTex);

    //sample the blend map
    float4 t = txBlendMap.Sample(samLinear, input.Tex);

    //blend the layers ontop of each other
    float4 texColour = c0;
    texColour = lerp(texColour, c1, t.r);
    texColour = lerp(texColour, c2, t.g);
    texColour = lerp(texColour, c3, t.b);
    texColour = lerp(texColour, c4, t.a);

    //
    //Lighting
    //
    
    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);

    float3 lightVecNorm = normalize(light.LightDir);

    float3 r = reflect(-lightVecNorm, normalW);

    // Determine how much specular light makes it into the eye.
    float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
    float diffuseAmount = max(dot(lightVecNorm, normalW), 0.0f);

	// Only display specular when there is diffuse
    if (diffuseAmount <= 0.0f)
    {
        specularAmount = 0.0f;
    }

	// Compute the ambient, diffuse, and specular terms separately.
    specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
    diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
    ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

    float4 finalColour;

    finalColour.rgb = (texColour.rgb * (ambient + diffuse)) + specular;

    finalColour.a = surface.DiffuseMtrl.a;

    return finalColour;
}