#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>

#include "Transform.h"
#include "Mesh.h"

using namespace DirectX;
using namespace std;

enum TextureID
{
	TX_DIFFUSE = 0,
	TX_NORMAL,
	TX_HEIGHTMAP,

	TX_NUMBER_OF_TEXTURES
};

enum Shader
{
	FX_NORMAL = 0,
	FX_PARRALAXED,
	FX_PARRALAXED_OCCLUSION,
	FX_BLOCK_COLOUR,
	FX_WIREFRAME,
	FX_DISPLACEMENT,
	FX_GEOMETRY,
	FX_TERRAIN,
	FX_SKY,
	FX_SKELETAL
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
	GameObject(string type, Transform* transform, Mesh geometry, Material material);
	~GameObject();

	Transform* GetTransform() const { return _transform; }

	string GetType() const { return _type; }

	Mesh GetGeometryData() const { return _geometry; }

	Material GetMaterial() const { return _material; }

	void SetTextureRV(ID3D11ShaderResourceView * textureRV, TextureID ID) { _textureRV[ID] = textureRV; }
	ID3D11ShaderResourceView * GetTextureRV(TextureID ID) const { return _textureRV[ID]; }
	bool HasTexture(TextureID ID) const { return _textureRV[ID] != nullptr ? true : false; }

	Shader GetShaderToUse() const { return _shaderToUse; }
	void SetShaderToUse(Shader shader) { _shaderToUse = shader; }

	void SetParent(GameObject * parent) { _parent = parent; }

	void Update(float deltaTime);
	virtual void Draw(ID3D11DeviceContext * pImmediateContext);

private:

	string _type;

	Transform* _transform;

	Mesh _geometry;
	Material _material;

	Shader _shaderToUse = FX_NORMAL;

	ID3D11ShaderResourceView * _textureRV[TX_NUMBER_OF_TEXTURES];

	GameObject * _parent;
};

