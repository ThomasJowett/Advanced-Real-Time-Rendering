#include "Terrain.h"
#include <fstream>
#include <algorithm>
#include "DDSTextureLoader.h"
#include "DirectXPackedVector.h"
#include "Utilities.h"

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

float Terrain::GetWidth() const
{
	return (_info.HeightMapWidth-1)*_info.CellSpacing;
}

float Terrain::GetDepth() const
{
	return (_info.HeightMapHeight - 1)*_info.CellSpacing;
}

float Terrain::GetHeight(float x, float z) const
{
	if (x < -0.5f * GetWidth() || z < -0.5f * GetDepth() || x > 0.5f * GetWidth() || z > 0.5f * GetDepth())
		return 0.0f;
	float c = (x + 0.5f*GetWidth()) / _info.CellSpacing;
	float d = (z - 0.5f*GetDepth()) / -_info.CellSpacing;

	int row = (int)floorf(d);
	int col = (int)floorf(c);

	float A = _heightMap[row*_info.HeightMapWidth + col];
	float B = _heightMap[row*_info.HeightMapWidth + col + 1];
	float C = _heightMap[(row + 1)*_info.HeightMapWidth + col];
	float D = _heightMap[(row + 1)*_info.HeightMapWidth + col + 1];

	float s = c - (float)col;
	float t = d - (float)row;

	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s)*uy + (1.0f - t)*vy;
	}
}

XMMATRIX Terrain::GetWorld() const
{
	return XMLoadFloat4x4(&_world);
}

void Terrain::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&_world, M);
}

void Terrain::Init(ID3D11Device * device, ID3D11DeviceContext * deviceContext, const InitInfo & initInfo, std::vector<float> heightmapData)
{
	_heightMap = heightmapData;
	_info = initInfo;

	_numPatchVertRows = ((_info.HeightMapHeight - 1) / CellsPerPatch) + 1;
	_numPatchVertCols = ((_info.HeightMapWidth - 1) / CellsPerPatch) + 1;

	_numPatchVertices = _numPatchVertRows * _numPatchVertCols;
	_numPatchQuadFaces = (_numPatchVertRows - 1)*(_numPatchVertCols - 1);

	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(device);
	BuildQuadPatchIB(device);
	BuildHeightMapSRV(device, _heightMap);

	CreateDDSTextureFromFile(device, _info.LayerMapFilename0.c_str(), nullptr, &_layer0SRV);
	CreateDDSTextureFromFile(device, _info.LayerMapFilename1.c_str(), nullptr, &_layer1SRV);
	CreateDDSTextureFromFile(device, _info.LayerMapFilename2.c_str(), nullptr, &_layer2SRV);
	CreateDDSTextureFromFile(device, _info.LayerMapFilename3.c_str(), nullptr, &_layer3SRV);
	CreateDDSTextureFromFile(device, _info.LayerMapFilename4.c_str(), nullptr, &_layer4SRV);

	CreateDDSTextureFromFile(device, _info.BlendMapFilename.c_str(), nullptr, &_blendMapSRV);

	_material.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	_material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	_material.specularPower = 1.0f;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TerrainConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HR(device->CreateBuffer(&bd, nullptr, &_pConstantBuffer));
}

void Terrain::Draw(ID3D11DeviceContext * pImmediateContext, Light light, Camera* camera, ShadowMap* pShadowMap)
{
	pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	pImmediateContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
	pImmediateContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);

	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(TerrainVertex);
	UINT offset = 0;

	TerrainConstantBuffer cb;

	XMFLOAT4* worldPlanes = reinterpret_cast<XMFLOAT4*>(cb.WorldFrustumPlanes);

	cb.EyePosW = camera->GetPosition();
	cb.light = light;
	cb.MaxDist = 500.0f;
	cb.MinDist = 20.0f;
	cb.MaxTess = 6.0f;
	cb.MinTess = 0.0f;
	cb.TexelCellSpaceU = 1.0f / _info.HeightMapWidth;
	cb.TexelCellSpaceV = 1.0f / _info.HeightMapHeight;
	cb.WorldCellSpace = _info.CellSpacing;

	Util::ExtractFrustumPlanes(worldPlanes, camera->GetViewProjection());

	XMMATRIX view = XMLoadFloat4x4(&camera->GetView());
	XMMATRIX projection = XMLoadFloat4x4(&camera->GetProjection());

	cb.World = XMMATRIX();
	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	XMMATRIX shadowTransform = XMLoadFloat4x4(&pShadowMap->GetTransform());
	cb.ShadowTransform = XMMatrixTranspose(shadowTransform);

	cb.surface.AmbientMtrl = _material.ambient;
	cb.surface.DiffuseMtrl = _material.diffuse;
	cb.surface.SpecularMtrl = _material.specular;

	cb.TexScale = { 50.0f, 50.0f };

	pImmediateContext->PSSetShaderResources(0, 1, &_blendMapSRV);
	pImmediateContext->PSSetShaderResources(1, 1, &_heightMapSRV);
	pImmediateContext->VSSetShaderResources(1, 1, &_heightMapSRV);
	pImmediateContext->DSSetShaderResources(1, 1, &_heightMapSRV);

	pImmediateContext->PSSetShaderResources(2, 1, &_layer0SRV);
	pImmediateContext->PSSetShaderResources(3, 1, &_layer1SRV);
	pImmediateContext->PSSetShaderResources(4, 1, &_layer2SRV);
	pImmediateContext->PSSetShaderResources(5, 1, &_layer3SRV);
	pImmediateContext->PSSetShaderResources(6, 1, &_layer4SRV);

	pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	pImmediateContext->IASetVertexBuffers(0, 1, &_quadPatchVertexBuffer, &stride, &offset);
	pImmediateContext->IASetIndexBuffer(_quadPatchIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_numPatchQuadFaces * 4, 0, 0);
}

void Terrain::DrawToShadowMap(ID3D11DeviceContext * deviceContext, ShadowMapConstantBuffer &cb, Camera * camera)
{
	deviceContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	deviceContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
	deviceContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	UINT stride = sizeof(TerrainVertex);
	UINT offset = 0;

	XMFLOAT4* worldPlanes = reinterpret_cast<XMFLOAT4*>(cb.WorldFrustumPlanes);

	Util::ExtractFrustumPlanes(worldPlanes, camera->GetViewProjection());

	cb.MaxDist = 500.0f;
	cb.MinDist = 20.0f;
	cb.MaxTess = 6.0f;
	cb.MinTess = 0.0f;

	deviceContext->VSSetShaderResources(0, 1, &_heightMapSRV);
	deviceContext->DSSetShaderResources(0, 1, &_heightMapSRV);

	deviceContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	deviceContext->IASetVertexBuffers(0, 1, &_quadPatchVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(_quadPatchIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed(_numPatchQuadFaces * 4, 0, 0);
}

void Terrain::Smooth()
{
	std::vector<float> dest(_heightMap.size());

	for (UINT i = 0; i < _info.HeightMapHeight; ++i)
	{
		for (UINT j = 0; j < _info.HeightMapWidth; ++j)
		{
			dest[i * _info.HeightMapWidth + j] = Average(i, j);
		}
	}

	_heightMap = dest;
}

bool Terrain::InBounds(int i, int j)
{
	return 
		i >= 0 && i < (int)_info.HeightMapHeight &&
		j >= 0 && j < (int)_info.HeightMapWidth;
}

float Terrain::Average(int i, int j)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for (int m = i - 1; m <= i + 1; ++m)
	{
		for (int n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += _heightMap[m*_info.HeightMapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Terrain::CalcAllPatchBoundsY()
{
	_patchBoundsY.resize(_numPatchQuadFaces);

	// For each patch
	for (UINT i = 0; i < _numPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _numPatchVertCols - 1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j * CellsPerPatch;
	UINT x1 = (j + 1)*CellsPerPatch;

	UINT y0 = i * CellsPerPatch;
	UINT y1 = (i + 1)*CellsPerPatch;

	float minY = +FLT_MAX;
	float maxY = -FLT_MAX;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y * _info.HeightMapWidth + x;
			minY = minY < _heightMap[k] ? minY : _heightMap[k];
			maxY = maxY > _heightMap[k] ? maxY : _heightMap[k];
		}
	}

	UINT patchID = i * (_numPatchVertCols - 1) + j;
	_patchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::BuildQuadPatchVB(ID3D11Device * device)
{
	std::vector<TerrainVertex> patchVertices(_numPatchVertRows*_numPatchVertCols);

	float halfWidth = 0.5f*GetWidth();
	float halfDepth = 0.5f*GetDepth();

	float patchWidth = GetWidth() / (_numPatchVertCols - 1);
	float patchDepth = GetDepth() / (_numPatchVertRows - 1);
	float du = 1.0f / (_numPatchVertCols - 1);
	float dv = 1.0f / (_numPatchVertRows - 1);

	for (UINT i = 0; i < _numPatchVertRows; ++i)
	{
		float z = halfDepth - i * patchDepth;
		for (UINT j = 0; j < _numPatchVertCols; ++j)
		{
			float x = -halfWidth + j * patchWidth;

			patchVertices[i*_numPatchVertCols + j].PosL = XMFLOAT3(x, 0.0f, z);

			// Stretch texture over grid.
			patchVertices[i*_numPatchVertCols + j].Tex.x = j * du;
			patchVertices[i*_numPatchVertCols + j].Tex.y = i * dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for (UINT i = 0; i < _numPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _numPatchVertCols - 1; ++j)
		{
			UINT patchID = i * (_numPatchVertCols - 1) + j;
			patchVertices[i*_numPatchVertCols + j].BoundsY = _patchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TerrainVertex) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	device->CreateBuffer(&vbd, &vinitData, &_quadPatchVertexBuffer);
}

void Terrain::BuildQuadPatchIB(ID3D11Device * device)
{
	std::vector<USHORT> indices(_numPatchQuadFaces * 4); // 4 indices per quad face

	// Iterate over each quad and compute indices.
	int k = 0;
	for (UINT i = 0; i < _numPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _numPatchVertCols - 1; ++j)
		{
			// Top row of 2x2 quad patch
			indices[k] = i * _numPatchVertCols + j;
			indices[k + 1] = i * _numPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1)*_numPatchVertCols + j;
			indices[k + 3] = (i + 1)*_numPatchVertCols + j + 1;

			k += 4; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	device->CreateBuffer(&ibd, &iinitData, &_quadPatchIndexBuffer);
}

void Terrain::BuildHeightMapSRV(ID3D11Device * device, std::vector<float> heightMap)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = _info.HeightMapWidth;
	texDesc.Height = _info.HeightMapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF is defined in DirectXPackedVector.h, for storing 16-bit float.
	std::vector<PackedVector::HALF> hmap(heightMap.size());
	std::transform(heightMap.begin(), heightMap.end(), hmap.begin(), PackedVector::XMConvertFloatToHalf);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
	data.SysMemPitch = _info.HeightMapWidth * sizeof(PackedVector::HALF);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	device->CreateTexture2D(&texDesc, &data, &hmapTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	device->CreateShaderResourceView(hmapTex, &srvDesc, &_heightMapSRV);

	// SRV saves reference.
	if(hmapTex)  hmapTex->Release();
}
