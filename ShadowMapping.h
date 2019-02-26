#pragma once
#include <d3d11_1.h>
#include "Commons.h"

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* d3dDevice, int width, int height);
	~ShadowMap();

	ID3D11ShaderResourceView* GetShaderResourceView() const { return _shadowMapShaderResourceView; }

	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* deviceContext);

	void BuildShadowTransforms(Light light, XMFLOAT3 cameraPosition);

	XMFLOAT4X4 GetView() const { return _view; }
	XMFLOAT4X4 GetProjection() const { return _projection; }
	XMFLOAT4X4 GetTransform() const { return _transform; }

private:
	ID3D11ShaderResourceView * _shadowMapShaderResourceView;
	ID3D11DepthStencilView * _shadowMapDepthStencil;

	UINT _width;
	UINT _height;

	D3D11_VIEWPORT _viewport;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
	XMFLOAT4X4 _transform;
};