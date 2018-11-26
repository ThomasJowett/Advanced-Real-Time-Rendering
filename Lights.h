#pragma once
//#include <windows.h>
#include <d3d11_1.h>
//#include <d3dcompiler.h>
#include <directxmath.h>

using namespace DirectX;

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;
	float SpecularPower;

	XMFLOAT3 Position;
	XMFLOAT3 Direction;
};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	XMFLOAT3 Attenuation;
	float Pad;
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (Direction, Spot)
	XMFLOAT3 Direction;
	float Spot;

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};