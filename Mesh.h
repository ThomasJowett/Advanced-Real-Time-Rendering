#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <vector>

#include "Commons.h"
#include "AnimatedModelData.h"

using namespace DirectX;

class Mesh
{
public:
	ID3D11Buffer * _vertexBuffer;
	ID3D11Buffer * _indexBuffer;
	int _numberOfIndices;

	UINT _vertexBufferStride;
	UINT _vertexBufferOffset;

	Mesh() {};
	Mesh(IndexedModel model, ID3D11Device* d3dDevice);
	Mesh(IndexedSkeletalModel model, ID3D11Device* d3dDevice);
	~Mesh();
};