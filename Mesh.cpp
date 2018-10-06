#include "Mesh.h"

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

	d3dDevice->CreateBuffer(&bd, &InitData, &vertexBuffer);

	//Index Buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * model.Indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &model.Indices[0];
	d3dDevice->CreateBuffer(&bd, &InitData, &indexBuffer);

	numberOfIndices = model.Indices.size();

	vertexBufferOffset = 0;
	vertexBufferStride = sizeof(SimpleVertex);
}
