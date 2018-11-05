//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);
Texture2D txHeight : register(t2);

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
	float3 LightPosW;
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

	float HeightMapScale;
	int MaxSamples;
	int MinSamples;
}

struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float3 TangentL : TANGENT;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT_NORMAL
{
    float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float4 TangentW : TANGENT;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT_SIMPLE_PARRALAX
{
	float4 PosH : SV_POSITION;
	float3 PosW : POSITION;
	float LightVecT : POSITION2;
	float EyeVecT : POSITION3;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT_PARRALAX
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float3 PosW : POSITION;
	float3 LightVecT : POSITION2;
	float3 EyeVecT : POSITION3;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Normal Map Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_NORMAL NormalVS(VS_INPUT input)
{
    VS_OUTPUT_NORMAL output = (VS_OUTPUT_NORMAL)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);

	output.Tex = input.Tex;

	float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
	output.NormW = normalize(normalW);

	float4 tangentW = mul(float4(input.TangentL, 0.0f), World);
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
// Normal Pixel Shader
//--------------------------------------------------------------------------------------
float4 NormalPS(VS_OUTPUT_NORMAL input) : SV_Target
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


	float3 lightLecNorm = normalize(light.LightPosW - input.PosW);
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
//--------------------------------------------------------------------------------------
// Parralax Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_SIMPLE_PARRALAX SimpleParralaxVS(VS_INPUT input)
{
	VS_OUTPUT_SIMPLE_PARRALAX output = (VS_OUTPUT_SIMPLE_PARRALAX)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;

	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);

	float3x3 tbnMatrix;
	tbnMatrix[0] = normalize(mul(float4(input.TangentL, 0.0f), World).xyz);
	tbnMatrix[1] = normalize(mul(float4(cross(input.NormL, input.TangentL), 0.0f), World).xyz);
	tbnMatrix[2] = normalize(mul(float4(input.NormL, 0.0f), World).xyz);

	output.Tex = input.Tex;

	float3 EyeVecW = (EyePosW - output.PosW).xyz;

	float3 LightVecW = (light.LightPosW - posW).xyz;

	output.LightVecT = normalize(mul(tbnMatrix, LightVecW));
	output.EyeVecT = normalize(mul(tbnMatrix, EyeVecW));

	return output;
}
//--------------------------------------------------------------------------------------
// Parralax Pixel Shader
//--------------------------------------------------------------------------------------
float4 SimpleParralaxPS(VS_OUTPUT_SIMPLE_PARRALAX input) : SV_Target
{
	//Expand the range of the normal value from (0, +1) to (-1, +1)
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	//float3 lightLecNorm = normalize(light.LightPosW - input.PosW);
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

	finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;

	finalColour.a = surface.DiffuseMtrl.a;
	
	return finalColour;
}
//--------------------------------------------------------------------------------------
// Parralax Occlusion Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_PARRALAX ParralaxVS(VS_INPUT input)
{
	VS_OUTPUT_PARRALAX output = (VS_OUTPUT_PARRALAX)0;

	float4 posW = mul(input.PosL, World);
	output.PosW = posW.xyz;
	
	output.PosH = mul(posW, View);
	output.PosH = mul(output.PosH, Projection);
	
	float3x3 tbnMatrix;
	tbnMatrix[0] = normalize(mul(float4(input.TangentL, 0.0f), World).xyz);
	tbnMatrix[1] = normalize(mul(float4(cross(input.NormL, input.TangentL), 0.0f), World).xyz);
	tbnMatrix[2] = normalize(mul(float4(input.NormL, 0.0f), World).xyz);

	output.Tex = input.Tex;

	output.NormW = tbnMatrix[2];
	
	float3 EyeVecW = (EyePosW - output.PosW).xyz;

	float3 LightVecW = (light.LightPosW - posW).xyz;

	output.LightVecT = normalize(mul(tbnMatrix, LightVecW));
	output.EyeVecT = normalize(mul(tbnMatrix, EyeVecW));

	return output;
}

//--------------------------------------------------------------------------------------
// Parralax Occlusion Pixel Shader
//--------------------------------------------------------------------------------------
float4 ParralaxPS(VS_OUTPUT_PARRALAX input) : SV_Target
{
	float3 toEye = normalize(input.EyeVecT);
	float parralaxLimit = -length(toEye.xy) / toEye.z;
	parralaxLimit *= HeightMapScale;
	
	float2 offsetDir = normalize(input.EyeVecT.xy);
	float2 maxOffset = offsetDir * parralaxLimit;
	
	float3 normalW = normalize(input.NormW);
	float3 toEyeW = normalize(EyePosW - input.PosW);
	

	int NumSamples = (int)lerp(MaxSamples, MinSamples, dot(toEyeW, normalW));

	float StepSize = 1.0f / (float)NumSamples;
	
	float2 dx = ddx(input.Tex);
	float2 dy = ddy(input.Tex);
	
	float CurrRayHeight = 1.0f;
	float2 CurrOffset = float2(0.0f, 0.0f);
	float2 LastOffset = float2(0.0f, 0.0f);
	
	float LastSampledHeight = 1.0f;
	float CurrSampledHeight = 1.0f;
	int CurrSample = 0;
	
	while (CurrSample < NumSamples)
	{
		CurrSampledHeight = txHeight.SampleGrad(samLinear, input.Tex + CurrOffset, dx, dy).r;
		if (CurrSampledHeight > CurrRayHeight)
		{
			float delta1 = CurrSampledHeight - CurrRayHeight;
			float delta2 = (CurrRayHeight + StepSize) - LastSampledHeight;
			
			float ratio = delta1 / (delta1 + delta2);
			
			CurrOffset = (ratio)* LastOffset + (1.0f - ratio) * CurrOffset;
			CurrSample = NumSamples + 1;
		}
		else
		{
			CurrSample++;
			
			CurrRayHeight -= StepSize;
			
			LastOffset = CurrOffset;
			CurrOffset += StepSize * maxOffset;
			
			LastSampledHeight = CurrSampledHeight;
		}
	}
	
	float2 FinalCoords = input.Tex + CurrOffset;

	// Get texture data from file
	float4 textureColour = txDiffuse.Sample(samLinear, FinalCoords);
	float4 bumpMap = txNormal.Sample(samLinear, FinalCoords);
	
	
	//float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
	//float4 bumpMap = txNormal.Sample(samLinear, input.Tex);
	
	//Expand the range of the normal value from (0, +1) to (-1, +1)
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	
	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);
	
	//float3 lightLecNorm = normalize(light.LightPosW - input.PosW);
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
	
	finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
	
	finalColour.a = surface.DiffuseMtrl.a;

	return finalColour;
}