#pragma once
#include <d3d11_1.h>

class ShadowMap
{
	ShadowMap(int width, int height);

	ID3D11ShaderResourceView* GetShaderResourceView() { return _shadowMapShaderResourceView; }
private:
	ID3D11ShaderResourceView * _shadowMapShaderResourceView;
};