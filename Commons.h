#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <vector>
#include <Windows.h>
#include <cstdarg>

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

__declspec(align(16)) struct TesselationConstantBuffer
{
	float MaxTessDistance;
	float MinTessDistance;
	float MinTessFactor;
	float MaxTessFactor;
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

	FLOAT OcclusionRadius = 0.3f;
	FLOAT OcculsionFadeStart = 0.2f;
	FLOAT OcclusionFadeEnd = 2.0f;
	FLOAT SurfaceEpsilon = 0.05f;

	INT SampleCount = 14;
};

__declspec(align(16)) struct SSAOBlurConstantBuffer
{
	FLOAT TexelWidth;
	FLOAT TexelHeight;

	FLOAT Weights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
	BOOL HorizontalBlur;
};

struct IndexedModel
{
	std::vector<SimpleVertex> Vertices;
	std::vector<WORD> Indices;
};

namespace Debug
{
#define DBG_OUTPUT(...) Debug::Output(__FILE__, __LINE__, __VA_ARGS__)
#define OUTPUT(...) Debug::Output(__VA_ARGS__)

	static void Output(WCHAR* pFormat, ...)
	{
		WCHAR buffer[1024] = { 0 };
		va_list args;
		va_start(args, pFormat);
		wvsprintf(buffer, pFormat, args);
		va_end(args);

		OutputDebugString(buffer);
	}

	static void Output(const char* file, const int line, WCHAR *pFormat, ...)
	{
		WCHAR buffer[1024] = { 0 };
		int stringLength = wsprintf(buffer, L"%hs(%d): ", file, line);

		va_list args;
		va_start(args, pFormat);
		wvsprintf(buffer + stringLength, pFormat, args);
		va_end(args);

		OutputDebugString(buffer);
	}
};