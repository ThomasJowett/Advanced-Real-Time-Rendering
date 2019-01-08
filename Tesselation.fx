//--------------------------------------------------------------------------------------
// File: Tesselation.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txDisplacement : register(t2);
Texture2D txShadow : register(t3);

SamplerState samLinear : register(s0);

SamplerComparisonState samShadow : register(s1);

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

cbuffer ConstantBuffer : register(b0)
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
cbuffer TessConstantBuffer : register(b1)
{
    float MaxTessDistance = 100.0f;
    float MinTessDistance = 1.0f;
    float MinTessFactor = 1.0f;
    float MaxTessFactor = 64.0f;
}

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float4 PosW : POSITION;
    float3 NormW : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex : TEXCOORD0;
};

struct PS_DISPLACEMENT_INPUT
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION0;
    float3 NormW : NORMAL;
    float3 LightVecT : POSITION1;
    float3 EyeVecT : POSITION2;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float3 Tangent : TANGENT;
    float2 Tex : TEXCOORD0;
};

struct HS_IO
{
    float4 Pos : POSITION0;
    float3 worldPos : POSITION1;
    float3 Norm : NORMAL;
    float3 Tangent : TANGENT;
    float2 Tex : TEXCOORD0;
    float TessFactor : TESS;
};

//Helper function that calculate if the pixel is a shadow
static const float SMAP_SIZE = 4096.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(float4 shadowPosH)
{
	// Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
    float depth = shadowPosH.z - 0.001f;

	// Texel size.
    const float dx = SMAP_DX;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

	[unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += txShadow.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit /= 9.0f;
}

HS_IO TesselationVS(VS_INPUT input)
{
    HS_IO output;

    output.Pos = input.PosL;

    output.worldPos = mul(input.PosL, World).xyz;

    float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
    output.Norm = normalize(normalW);


    output.Tangent = mul(input.Tangent, (float3x3) World);

    output.Tex = input.Tex;

    float dist = distance( EyePosW, output.worldPos);

    //float tess = saturate((MinTessDistance - dist) / (MinTessDistance - MaxTessDistance));
    float tess = saturate(dist / 25.0f);

    tess = 1.0f - tess;

    //output.TessFactor = MinTessFactor + tess * (MaxTessFactor - MinTessFactor);
    output.TessFactor = 0.1f + (tess * 64.0f);

   

    return output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
[maxtessfactor(64.0)]

HS_IO MainHS(InputPatch<HS_IO, 3> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
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
    float Inside : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PatchHS(InputPatch<HS_IO, 3> ip, uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;
    output.Edges[0] = 0.5f * (ip[1].TessFactor + ip[2].TessFactor);
    output.Edges[1] = 0.5f * (ip[2].TessFactor + ip[0].TessFactor);
    output.Edges[2] = 0.5f * (ip[0].TessFactor + ip[1].TessFactor);
    output.Inside = output.Edges[0];
    return output;
}

//Domain Shader
[domain("tri")]
PS_INPUT DSMAIN(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_IO, 3> TrianglePatch)
{
    PS_INPUT output;

	//Interpolate world space position with barycentric coordinates
    float4 Pos = BarycentricCoordinates.x * TrianglePatch[0].Pos
		+ BarycentricCoordinates.y * TrianglePatch[1].Pos
		+ BarycentricCoordinates.z * TrianglePatch[2].Pos;

    output.PosH = float4(Pos.xyz, 1.0);

    output.Tex = BarycentricCoordinates.x * TrianglePatch[0].Tex
		+ BarycentricCoordinates.y * TrianglePatch[1].Tex
		+ BarycentricCoordinates.z * TrianglePatch[2].Tex;

    output.NormW = BarycentricCoordinates.x * TrianglePatch[0].Norm
		+ BarycentricCoordinates.y * TrianglePatch[1].Norm
		+ BarycentricCoordinates.z * TrianglePatch[2].Norm;

    float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].worldPos
		+ BarycentricCoordinates.y * TrianglePatch[1].worldPos
		+ BarycentricCoordinates.z * TrianglePatch[2].worldPos;

    output.PosW = float4(vWorldPos.xyz, 1.0);

    output.PosH = mul(output.PosH, World);
    output.PosH = mul(output.PosH, View);
    output.PosH = mul(output.PosH, Projection);

    return output;
}

[domain("tri")]
PS_DISPLACEMENT_INPUT DisplacementDS(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation, const OutputPatch<HS_IO, 3> TrianglePatch)
{
    PS_DISPLACEMENT_INPUT output;

	//Interpolate world space position with barycentric coordinates
     output.PosW = BarycentricCoordinates.x * TrianglePatch[0].worldPos
		+ BarycentricCoordinates.y * TrianglePatch[1].worldPos
		+ BarycentricCoordinates.z * TrianglePatch[2].worldPos;

    output.Tex = BarycentricCoordinates.x * TrianglePatch[0].Tex
		+ BarycentricCoordinates.y * TrianglePatch[1].Tex
		+ BarycentricCoordinates.z * TrianglePatch[2].Tex;

    output.NormW = BarycentricCoordinates.x * TrianglePatch[0].Norm
		+ BarycentricCoordinates.y * TrianglePatch[1].Norm
		+ BarycentricCoordinates.z * TrianglePatch[2].Norm;

    float3 Tangent = BarycentricCoordinates.x * TrianglePatch[0].Tangent
		+ BarycentricCoordinates.y * TrianglePatch[1].Tangent
		+ BarycentricCoordinates.z * TrianglePatch[2].Tangent;

    float displacement = txDisplacement.SampleLevel(samLinear, output.Tex, 0.0f).r;

    output.PosW += (HeightMapScale * (displacement - 1.0)) * output.NormW;

    output.PosH = float4(output.PosW, 1.0);

    //output.PosH = mul(output.PosH, World);
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

    output.ShadowPosH = mul(float4(output.PosW.xyz, 1.0), ShadowTransform);

    return output;
}

float4 TesselationPS(PS_INPUT input) : SV_Target
{
    return surface.DiffuseMtrl;
}

float4 DisplacementPS(PS_DISPLACEMENT_INPUT input) : SV_Target
{
    float3 toEye = normalize(input.EyeVecT);
    float3 normalW = normalize(input.NormW);

    float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
    float4 bumpMap = txNormal.Sample(samLinear, input.Tex);

    //Expand the range of the normal value from (0, +1) to (-1, +1)
    bumpMap = (bumpMap * 2.0f) - 1.0f;

    float3 ambient = float3(0.0f, 0.0f, 0.0f);
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
	
    float shadow = CalcShadowFactor(input.ShadowPosH);

    float3 lightLecNorm = normalize(input.LightVecT);
	// Compute Colour
	
	// Compute the reflection vector.
    float3 r = reflect(-lightLecNorm, bumpMap.xyz);
	
	// Determine how much specular light makes it into the eye.
    float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);
	
	// Determine the diffuse light intensity that strikes the vertex.
    float diffuseAmount = max(dot(lightLecNorm, bumpMap.xyz), 0.0f);
	
	// Only display specular when there is diffuse
    if (diffuseAmount <= 0.0f)
    {
        specularAmount = 0.0f;
    }
	
	// Compute the ambient, diffuse, and specular terms separately.
    specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
    diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
    ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

	// Sum all the terms together and copy over the diffuse alpha.
    float4 finalColour;

    finalColour.rgb = (textureColour.rgb * (ambient + (diffuse * shadow)) + (specular * shadow));
	
    finalColour.a = surface.DiffuseMtrl.a;
	
    return finalColour;
}