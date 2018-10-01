#pragma once

#include <vector>

struct Vertex
{
	//XMFLOAT3 PosL;
	//XMFLOAT3 NormL;
	//XMFLOAT3 Tangent;
	//XMFLOAT2 Tex;
};

struct IndexedModel
{
	std::vector<Vertex> Vertices;
	//std::vector<UINT> Indices;
};

class Mesh
{

};
