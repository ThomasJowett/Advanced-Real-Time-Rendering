#include "Mesh.h"
#include "Commons.h"

Mesh::Mesh(IndexedModel model, ID3D11Device* d3dDevice)
{
	//Vertex Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * model.Vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &model.Vertices[0];

	d3dDevice->CreateBuffer(&bd, &InitData, &_vertexBuffer);

	//Index Buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * model.Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &model.Indices[0];
	d3dDevice->CreateBuffer(&bd, &InitData, &_indexBuffer);

	_numberOfIndices = model.Indices.size();

	_vertexBufferOffset = 0;
	_vertexBufferStride = sizeof(SimpleVertex);
}

Mesh::Mesh(IndexedSkeletalModel model, ID3D11Device * d3dDevice)
{
	//VertexBuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SkeletalVertex) * model.Vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &model.Vertices[0];

	d3dDevice->CreateBuffer(&bd, &InitData, &_vertexBuffer);

	//Index Buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * model.Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &model.Indices[0];
	d3dDevice->CreateBuffer(&bd, &InitData, &_indexBuffer);

	_numberOfIndices = model.Indices.size();

	_vertexBufferOffset = 0;
	_vertexBufferStride = sizeof(SkeletalVertex);
}

Mesh::~Mesh()
{
	//if(_vertexBuffer) _vertexBuffer->Release();
	//if(_indexBuffer) _indexBuffer->Release();
}