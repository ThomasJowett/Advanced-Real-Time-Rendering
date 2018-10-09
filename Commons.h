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
	XMFLOAT3 LightVecW;
};

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	SurfaceInfo surface;

	Light light;

	XMFLOAT3 EyePosW;
	float HasTexture;

	float HeightMapScale;
	int MaxSamples;
	int MinSamples;
};

struct IndexedModel
{
	std::vector<SimpleVertex> Vertices;
	std::vector<WORD> Indices;
};