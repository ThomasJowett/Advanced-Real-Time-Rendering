#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <vector>

#include "Commons.h"

using namespace DirectX;

class Mesh
{
public:
	ID3D11Buffer * vertexBuffer;
	ID3D11Buffer * indexBuffer;
	int numberOfIndices;

	UINT vertexBufferStride;
	UINT vertexBufferOffset;

	Mesh() {};
	Mesh(IndexedModel model, ID3D11Device* d3dDevice);
};
