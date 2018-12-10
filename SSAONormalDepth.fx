//--------------------------------------------------------------------------------------
// File: SSAONormalDepth.fx
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register(b0)
{
    float4x4 WorldView;
    float4x4 WorldInvTransposeView;
    float4x4 WorldViewProjection;
    float4x4 TexTransform;
}

struct VS_INPUT
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 PosV : POSITION;
    float3 NormalV : NORMAL;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT SSAONormalDepthVS(VS_INPUT input)
{
    VS_OUTPUT output;

    output.PosV = mul(float4(input.PosL, 1.0f), WorldView).xyz;
    output.NormalV = mul(input.NormalL, (float3x3) WorldInvTransposeView);

    output.PosH = mul(float4(input.PosL, 1.0f), WorldViewProjection);

    output.Tex = mul(float4(input.Tex, 0.0f, 1.0f), TexTransform).xy;

    return output;
}

float4 SSAONormalDepthPS(VS_OUTPUT input) :SV_Target
{
    input.NormalV = normalize(input.NormalV);

    return float4(input.NormalV, input.PosV.z);
}