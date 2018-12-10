#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <vector>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 PosL;
	XMFLOAT3 NormL;
	XMFLOAT3 Tangent;
	XMFLOAT2 Tex;

	SimpleVertex() {};
	SimpleVertex(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT3 tangent, XMFLOAT2 uv)
		:PosL(position), NormL(normal), Tangent(tangent), Tex(uv) {}
	SimpleVertex(XMVECTOR position, XMVECTOR normal, XMVECTOR tangent, XMVECTOR uv)
	{
		XMStoreFloat3(&PosL, position);
		XMStoreFloat3(&NormL, normal);
		XMStoreFloat3(&Tangent, tangent);
		XMStoreFloat2(&Tex, uv);
	}
	SimpleVertex(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v)
		: PosL(px, py, pz), NormL(nx, ny, nz),
		Tangent(tx, ty, tz), Tex(u, v) {}

	bool operator<(const SimpleVertex other)const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};
};

struct SurfaceInfo
{
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 SpecularMtrl;
};

struct Light
{
	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;

	float SpecularPower;
	XMFLOAT3 Direction;
};

__declspec(align(16)) struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX ShadowTransform;

	SurfaceInfo surface;

	Light light;

	XMFLOAT3 EyePosW;
	float HasTexture;

	float HeightMapScale;
	int MaxSamples;
	int MinSamples;
};

const int c_MaxSamples = 16;

__declspec(align(16)) struct PostProcessConstantBuffer
{
	XMVECTOR sampleOffsets[c_MaxSamples];
	XMVECTOR sampleWeights[c_MaxSamples];
};

__declspec(align(16)) struct ShadowMapConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};

__declspec(align(16)) struct SSAOConstantBuffer
{
	XMMATRIX ViewToTexSpace;
	XMFLOAT4 OffsetVectors[14];
	XMFLOAT4 FrustumCorners[4];

	FLOAT OcclusionRadius;
	FLOAT OcculsionFadeStart;
	FLOAT OcclusionFadeEnd;
	FLOAT SurfaceEpsilon;
};

__declspec(align(16)) struct SSAONormalDepthConstantBuffer
{
	XMMATRIX WorldView;
	XMMATRIX WorldInvTransposeView;
	XMMATRIX WorldViewProjection;
	XMMATRIX TexTransform;
};

struct IndexedModel
{
	std::vector<SimpleVertex> Vertices;
	std::vector<WORD> Indices;
};