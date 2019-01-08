//--------------------------------------------------------------------------------------
// File: SSAOBlur.fx
//--------------------------------------------------------------------------------------

Texture2D txNormalDepthMap : register(t0);
Texture2D txImage : register(t1);

SamplerState samLinear : register(s0);

cbuffer ConstantBuffer
{
    float TexelWidth;
    float TexelHeight;

    float Weights[11] =
    {
        0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
    };

    bool HorizontalBlur;
}

cbuffer Fixed
{
    static const int BlurRadius = 5;
};

struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float3 TangentL : TANGENT;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

VS_OUTPUT BlurVS(VS_INPUT input)
{
    VS_OUTPUT output;

    output.PosH = float4(input.PosL.xyz, 1.0f);

    output.Tex = input.Tex;

    return output;
}

float4 BlurPS(VS_OUTPUT input) : SV_Target
{
    float2 texOffset;
    if(HorizontalBlur)
    {
        texOffset = float2(TexelWidth, 0.0f);
    }
    else
    {
        texOffset = float2(0.0f, TexelHeight);
    }

    float4 colour = Weights[5] * txImage.SampleLevel(samLinear, input.Tex, 0.0);
    float totalWeight = Weights[5];

    float4 centerNormalDepth = txNormalDepthMap.SampleLevel(samLinear, input.Tex, 0.0f);

    for (float i = -BlurRadius; i <= BlurRadius; i++)
    {
        if (i == 0)
            continue;
        float2 tex = input.Tex + i * texOffset;

        float4 neighborNormalDepth = txNormalDepthMap.SampleLevel(samLinear, tex, 0.0f);

        if(dot(neighborNormalDepth.xyz, centerNormalDepth.xyz) >= 0.8f && abs(neighborNormalDepth.a - centerNormalDepth.a) <= 0.2f)
        {
            float weight = Weights[i + BlurRadius];

            //add neighbor pixel to blur
            colour += weight * txImage.SampleLevel(samLinear, tex, 0.0f);

            totalWeight += weight;
        }
    }

    return colour / totalWeight;
}