#pragma once

#include <d3d11_1.h>
#include <directxmath.h>
#include "Mesh.h"
#include "Camera.h"

using namespace DirectX;

class SSAO
{
public:
	SSAO(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height, float fovy, float farZ);
	~SSAO();

	ID3D11ShaderResourceView* NormalDepthSRV();
	ID3D11ShaderResourceView* AmbientSRV();

	void SetNormalDepthRenderTarget(ID3D11DepthStencilView* depthStencilView);

	void ComputeSSAO(const Camera* camera);

	void BuildSamplers();

	void BlurAmbientMap(int blurCount);
private:
	SSAO(const SSAO& rhs);
	SSAO& operator=(const SSAO& rhs);

	void BlurAmbientMap(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);

	void BuildFrustumFarCorners(float fovY, float farZ);

	void BuildTextureViews();
	void ReleaseTextureViews();

	void BuildRandomVectorTexture();

	void BuildOffsetVectors();

	void DrawFullScreenQuad();

private:
	ID3D11Device * _pd3dDevice;
	ID3D11DeviceContext* _pImmediateContext;

	ID3D11ShaderResourceView* _randomVectorSRV;

	ID3D11RenderTargetView* _normalDepthRTV;
	ID3D11ShaderResourceView* _normalDepthSRV;

	ID3D11RenderTargetView* _ambientRTV0;
	ID3D11ShaderResourceView* _ambientSRV0;

	ID3D11RenderTargetView* _ambientRTV1;
	ID3D11ShaderResourceView* _ambientSRV1;

	UINT _renderTargetWidth;
	UINT _renderTargetHeight;

	D3D11_VIEWPORT _ambientViewport;

	XMFLOAT4 _frustumFarCorner[4];

	XMFLOAT4 _offsets[14];

	Mesh* _mesh;

	ID3D11SamplerState * _pSamplerNormalDepth;
	ID3D11SamplerState * _pSamplerRandomVector;
	ID3D11Buffer * _pConstantBuffer;
};