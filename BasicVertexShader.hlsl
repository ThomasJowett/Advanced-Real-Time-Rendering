struct VS_INPUT
{
	float4 PosL : POSITION;
	float3 NormL : NORMAL;
	float3 TangentL : TANGENT;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 PosL : SV_POSITION;
	float3 NormL : NORMAL;
	float3 TangentL : TANGENT;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT main( VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.PosL = input.PosL;
	output.NormL = input.NormL;
	output.TangentL = input.TangentL;
	output.Tex = input.Tex;

	return output;
}