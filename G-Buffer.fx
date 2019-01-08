//--------------------------------------------------------------------------------------
// File: G-Buffer.fx
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;

    float3 EyePosW;
    float HeightMapScale;


}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 PosL : POSITION;
    float3 NormL : NORMAL;
    float3 TangentL : TANGENT;
    float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float4 PosV : POSITION0;
    float3 NormW : NORMAL0;
    float3 NormV : NORMAL1;
    float3 PosW : POSITION1;
    float3 LightVecT : POSITION2;
    float3 EyeVecT : POSITION3;
    float2 Tex : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 LightAccumulation : SV_Target0;
    float4 Diffuse : SV_Target1;
    float4 Specular : SV_Target2;
    float4 NormalDepth : SV_Target3;
};

//--------------------------------------------------------------------------------------
// Normal Map Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT NormalVS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;


}
