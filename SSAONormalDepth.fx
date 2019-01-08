//--------------------------------------------------------------------------------------
// File: SSAONormalDepth.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D txNormal : register(t1);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormalL : NORMAL;
    float3 TangentL : TANGENT;
    float2 Tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 PosV : POSITION;
    float3 NormalV : NORMAL;
    float2 Tex : TEXCOORD0;
};

VS_OUTPUT SSAONormalDepthVS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4x4 WorldView = mul(World, View);

    output.PosV = mul(input.PosL, WorldView);
    output.PosH = mul(output.PosV, Projection);

    //output.PosV.b = output.PosV.b / 100;

    float3x3 tbnMatrix;
    tbnMatrix[0] = normalize(mul(float4(input.TangentL, 0.0f), World).xyz);
    tbnMatrix[1] = normalize(mul(float4(cross(input.NormalL, input.TangentL), 0.0f), World).xyz);
    tbnMatrix[2] = normalize(mul(float4(input.NormalL, 0.0f), World).xyz);

    output.NormalV = mul((float3x3) transpose(View), tbnMatrix[2]);

    //output.NormalV.b = -output.NormalV.b;

    return output;
}

float4 SSAONormalDepthPS(VS_OUTPUT input) :SV_Target
{
    input.NormalV = normalize(input.NormalV);
    return float4(input.NormalV, input.PosV.z);
}