#pragma once
#include <vector>
#include "Commons.h"
#include "GameObject.h"
#include "Camera.h"

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;
		float HeightScale;
		UINT HeightMapWidth;
		UINT HeightMapHeight;
		float CellSpacing;
	};

	Terrain();
	~Terrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const InitInfo& initInfo);

	void Draw(ID3D11DeviceContext* deviceContext, Light light, Camera* camera);
private:

	void LoadHeightMap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightMapSRV(ID3D11Device* device);

private:
	static const int CellsPerPatch = 64;

	ID3D11Buffer* _quadPatchVertexBuffer;
	ID3D11Buffer* _quadPatchIndexBuffer;

	ID3D11ShaderResourceView* _layerMapArraySRV;
	ID3D11ShaderResourceView* _blendMapSRV;
	ID3D11ShaderResourceView* _heightMapSRV;

	InitInfo _info;

	UINT _numPatchVertices;
	UINT _numPatchQuadFaces;

	UINT _numPatchVertRows;
	UINT _numPatchVertCols;
	
	XMFLOAT4X4 _world;

	Material _material;

	std::vector<XMFLOAT2> _patchBoundsY;
	std::vector<float> _heightMap;

	ID3D11ShaderResourceView * _layer0SRV;
	ID3D11ShaderResourceView * _layer1SRV;
	ID3D11ShaderResourceView * _layer2SRV;
	ID3D11ShaderResourceView * _layer3SRV;
	ID3D11ShaderResourceView * _layer4SRV;
};