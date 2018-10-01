//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

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
	float3 LightVecW;
};

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light light;

	float3 EyePosW;
	float HasTexture;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float3 tangent : TANGENT;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float4 TangentW : TANGENT;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);
	output.Tex = input.Tex;

	float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	output.NormW = normalize(normalW);

	float4 tangentW = mul(float4(input.tangent, 0.0f), World);
	output.TangentW = normalize(tangentW);

    return output;
}

//Helper function to convert tex tangent normal to world space
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	//Build orthonormal basis
	//ensure T is orthonormal to w as PS interpolation may have affected this
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);

	//create binormal from N and T
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);

	//Transform from tangent space to world space
	float3 bumpedNormalW = mul(normalMapSample, TBN);

	return bumpedNormalW;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 normalW = normalize(input.NormW);

	float3 toEye = normalize(EyePosW - input.PosW);

	// Get texture data from file
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	float4 bumpMap = txNormal.Sample(samLinear, input.Tex);

	//Expand the range of the normal value from (0, +1) to (-1, +1)
	bumpMap = (bumpMap * 2.0f) -1.0f;
	float3 bumpedNormalW = NormalSampleToWorldSpace(bumpMap.xyz, input.NormW, input.TangentW);

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	float3 lightLecNorm = normalize(light.LightVecW);
	// Compute Colour

	// Compute the reflection vector.
	float3 r = reflect(-lightLecNorm, bumpedNormalW);

	// Determine how much specular light makes it into the eye.
	float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

	// Determine the diffuse light intensity that strikes the vertex.
	float diffuseAmount = max(dot(lightLecNorm, bumpedNormalW), 0.0f);

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

	if (HasTexture == 1.0f)
	{
		finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
	}
	else
	{
		finalColour.rgb = ambient + diffuse + specular;
	}

	finalColour.a = surface.DiffuseMtrl.a;

	return finalColour;
}

