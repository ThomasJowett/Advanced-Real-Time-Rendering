#include "ShadowMapping.h"

ShadowMap::ShadowMap(ID3D11Device * d3dDevice, int width, int height)
	:_width(width), _height(height), _shadowMapShaderResourceView(0), _shadowMapDepthStencil(0)
{
	_viewport.TopLeftX = 0.0f;
	_viewport.TopLeftY = 0.0f;
	_viewport.Width = static_cast<float>(width);
	_viewport.Height = static_cast<float>(height);
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = _width;
	texDesc.Height = _height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* depthMap = 0;
	HRESULT hr = d3dDevice->CreateTexture2D(&texDesc, 0, &depthMap);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = d3dDevice->CreateDepthStencilView(depthMap, &dsvDesc, &_shadowMapDepthStencil);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = d3dDevice->CreateShaderResourceView(depthMap, &srvDesc, &_shadowMapShaderResourceView);

	if (depthMap) depthMap->Release();
}

ShadowMap::~ShadowMap()
{
	if (_shadowMapShaderResourceView) _shadowMapShaderResourceView->Release();
	if (_shadowMapDepthStencil) _shadowMapDepthStencil->Release();
}

void ShadowMap::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext * deviceContext)
{
	deviceContext->RSSetViewports(1, &_viewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	deviceContext->OMSetRenderTargets(1, renderTargets, _shadowMapDepthStencil);

	deviceContext->ClearDepthStencilView(_shadowMapDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowMap::BuildShadowTransforms(Light light)
{
	XMVECTOR lightDirection = XMLoadFloat3(&light.Direction);
	XMVECTOR lightPosition = lightDirection;
	XMVECTOR targetPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(lightPosition, targetPosition, up);

	XMMATRIX projection = XMMatrixOrthographicLH(50, 50, 0, 50);

	XMMATRIX transform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = view * projection * transform;

	XMStoreFloat4x4(&_view, view);
	XMStoreFloat4x4(&_projection, projection);
	XMStoreFloat4x4(&_transform, S);
}
