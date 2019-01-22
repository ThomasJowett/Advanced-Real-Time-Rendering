struct GS_IO
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float4 TangentW : TANGENT;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

struct PS_INPUT
{
	float4 PosH : SV_POSITION;
	float3 NormW : NORMAL;
	float4 TangentW : TANGENT;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};



[maxvertexcount(6)]
void main(triangle GS_IO input[3] : SV_POSITION, inout TriangleStream< PS_INPUT > output)
{
	for (uint i = 0; i < 3; i++)
	{
		GS_IO element;
		element = input[i];
		output.Append(element);
	}

	output.RestartStrip();

	for (uint i = 0; i < 3; i++)
	{
		GS_IO element;

		element = input[i];
		element.PosH.x += 5.0f;
	}
}