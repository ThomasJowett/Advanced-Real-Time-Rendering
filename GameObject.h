#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>

#include "Transform.h"

using namespace DirectX;
using namespace std;

enum TextureID
{
	TX_DIFFUSE = 0,
	TX_NORMAL,
};

struct Geometry
{
	ID3D11Buffer * vertexBuffer;
	ID3D11Buffer * indexBuffer;
	int numberOfIndices;

	UINT vertexBufferStride;
	UINT vertexBufferOffset;
};

struct Material
{
	XMFLOAT4 diffuse;
	XMFLOAT4 ambient;
	XMFLOAT4 specular;
	float specularPower;
};

class GameObject
{
public:
	GameObject(string type, Transform* transform, Geometry geometry, Material material);
	~GameObject();

	Transform* GetTransform() const { return _transform; }

	string GetType() const { return _type; }

	Geometry GetGeometryData() const { return _geometry; }

	Material GetMaterial() const { return _material; }

	void SetTextureRV(ID3D11ShaderResourceView * textureRV, TextureID ID) { _textureRV[ID] = textureRV; }
	ID3D11ShaderResourceView * GetTextureRV(TextureID ID) const { return _textureRV[ID]; }
	bool HasTexture(TextureID ID) const { return _textureRV[ID] ? true : false; }

	void SetParent(GameObject * parent) { _parent = parent; }

	void Update(float deltaTime);
	void Draw(ID3D11DeviceContext * pImmediateContext);

private:

	string _type;

	Transform* _transform;

	Geometry _geometry;
	Material _material;

	ID3D11ShaderResourceView * _textureRV[2];

	GameObject * _parent;
};

