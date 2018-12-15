#include "SSAO.h"
#include "GeometryGenerator.h"
#include <DirectXPackedVector.h>

constexpr UINT randomTextureSize = 128;

SSAO::SSAO(ID3D11Device * device, ID3D11DeviceContext * deviceContext, int width, int height, float fovy, float farZ)
	:_pd3dDevice(device), _pImmediateContext(deviceContext), _randomVectorSRV(0), _normalDepthRTV(0), _normalDepthSRV(0),
	_ambientRTV0(0), _ambientRTV1(0), _ambientSRV0(0), _ambientSRV1(0)
{
	_renderTargetWidth = width;
	_renderTargetHeight = height;

	_ambientViewport.TopLeftX = 0.0f;
	_ambientViewport.TopLeftY = 0.0f;
	_ambientViewport.Width = width / 2.0f;
	_ambientViewport.Height = height / 2.0f;
	_ambientViewport.MinDepth = 0.0f;
	_ambientViewport.MaxDepth = 1.0f;

	BuildFrustumFarCorners(fovy, farZ);
	BuildTextureViews();

	BuildFullScreenQuad();
	BuildSamplers();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	_pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	BuildOffsetVectors();
	BuildRandomVectorTexture();
}

SSAO::~SSAO()
{
	if (_fullScreenQuadIndexBuffer) _fullScreenQuadIndexBuffer->Release();
	if (_fullScreenQuadVertexBuffer) _fullScreenQuadVertexBuffer->Release();
	if (_randomVectorSRV) _randomVectorSRV->Release();

	ReleaseTextureViews();
}

ID3D11ShaderResourceView * SSAO::NormalDepthSRV() const
{
	return _normalDepthSRV;
}

ID3D11ShaderResourceView * SSAO::AmbientSRV()
{
	return _ambientSRV0;
}

ID3D11ShaderResourceView * SSAO::RandomVectorSRV()
{
	return _randomVectorSRV;
}

void SSAO::SetNormalDepthRenderTarget(ID3D11DepthStencilView * depthStencilView)
{
	_pImmediateContext->OMSetRenderTargets(1, &_normalDepthRTV, depthStencilView);

	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	_pImmediateContext->ClearRenderTargetView(_normalDepthRTV, clearColor);

	_pImmediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void SSAO::ComputeSSAO(const Camera * camera)
{
	//Assumes that the vertex and pixel shaders are already set
	//Set the render target
	_pImmediateContext->OMSetRenderTargets(1, &_ambientRTV0, 0);
	float ClearColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f };//cyan
	_pImmediateContext->ClearRenderTargetView(_ambientRTV0, ClearColor);
	_pImmediateContext->RSSetViewports(1, &_ambientViewport);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	
	XMMATRIX P = XMLoadFloat4x4(&camera->GetProjection());
	XMMATRIX PT = XMMatrixMultiply(P, T);
	
	
	//Set variables in the constant buffer
	SSAOConstantBuffer cb;
	
	cb.ViewToTexSpace = PT;
	auto offsets = reinterpret_cast<XMFLOAT4*>(cb.OffsetVectors);
	offsets = _offsets;
	auto frustumCorners = reinterpret_cast<XMFLOAT4*>(cb.FrustumCorners);
	frustumCorners = _frustumFarCorner;
	
	//Bind shader resource views
	_pImmediateContext->PSSetShaderResources(0, 1, &_normalDepthSRV);
	_pImmediateContext->PSSetShaderResources(1, 1, &_randomVectorSRV);
	
	//set samplers
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerNormalDepth);
	_pImmediateContext->PSSetSamplers(1, 1, &_pSamplerRandomVector);
	_pImmediateContext->PSSetSamplers(2, 1, &_pSamplerLinear);
	
	//Bind the constant buffers
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	_pImmediateContext->IASetVertexBuffers(0, 1, &_fullScreenQuadVertexBuffer, &stride, &offset);
	_pImmediateContext->IASetIndexBuffer(_fullScreenQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	
	_pImmediateContext->DrawIndexed(6, 0, 0);
}

void SSAO::BuildSamplers()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerNormalDepth);
	
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerRandomVector);

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);
}

void SSAO::BlurAmbientMap(int blurCount)
{
	for (int i = 0; i < blurCount; ++i)
	{
		BlurAmbientMap(_ambientSRV0, _ambientRTV1, true);
		BlurAmbientMap(_ambientSRV1, _ambientRTV0, false);
	}
}

void SSAO::BlurAmbientMap(ID3D11ShaderResourceView * inputSRV, ID3D11RenderTargetView * outputRTV, bool horzBlur)
{
	_pImmediateContext->OMSetRenderTargets(1, &outputRTV, 0);
	float ClearColor[4] = { 1.0f, 0.7f, 0.2f, 1.0f };//Orange
	_pImmediateContext->ClearRenderTargetView(outputRTV, ClearColor);
	_pImmediateContext->RSSetViewports(1, &_ambientViewport);


}

void SSAO::BuildFrustumFarCorners(float fovY, float farZ)
{
	float aspect = (float)_renderTargetWidth / (float)_renderTargetHeight;

	float halfHeight = farZ * tanf(0.5*fovY);
	float halfWidth = aspect * halfHeight;

	_frustumFarCorner[0] = XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	_frustumFarCorner[1] = XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	_frustumFarCorner[2] = XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	_frustumFarCorner[3] = XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}

void SSAO::BuildTextureViews()
{
	ReleaseTextureViews();

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = _renderTargetWidth;
	texDesc.Height = _renderTargetHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* normalDepthTex = 0;
	_pd3dDevice->CreateTexture2D(&texDesc, 0, &normalDepthTex);
	_pd3dDevice->CreateShaderResourceView(normalDepthTex, 0, &_normalDepthSRV);
	_pd3dDevice->CreateRenderTargetView(normalDepthTex, 0, &_normalDepthRTV);

	if (normalDepthTex) normalDepthTex->Release();

	texDesc.Width = _renderTargetWidth / 2;
	texDesc.Height = _renderTargetHeight / 2;
	//texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	ID3D11Texture2D* ambientTex0 = 0;
	_pd3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex0);
	_pd3dDevice->CreateShaderResourceView(ambientTex0, 0, &_ambientSRV0);
	_pd3dDevice->CreateRenderTargetView(ambientTex0, 0, &_ambientRTV0);

	ID3D11Texture2D* ambientTex1 = 0;
	_pd3dDevice->CreateTexture2D(&texDesc, 0, &ambientTex1);
	_pd3dDevice->CreateShaderResourceView(ambientTex1, 0, &_ambientSRV1);
	_pd3dDevice->CreateRenderTargetView(ambientTex1, 0, &_ambientRTV1);

	if (ambientTex0) ambientTex0->Release();
	if (ambientTex1) ambientTex1->Release();
}

void SSAO::ReleaseTextureViews()
{
	if (_normalDepthRTV) _normalDepthRTV->Release();
	if (_normalDepthSRV) _normalDepthSRV->Release();
	if (_ambientRTV0) _ambientRTV0->Release();
	if (_ambientSRV0) _ambientSRV0->Release();
	if (_ambientRTV1) _ambientRTV0->Release();
	if (_ambientSRV1) _ambientSRV0->Release();
}

void SSAO::BuildRandomVectorTexture()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = randomTextureSize;
	texDesc.Height = randomTextureSize;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = randomTextureSize * sizeof(XMFLOAT4);
	
	XMFLOAT4 colour[randomTextureSize * randomTextureSize];
	for (int i = 0; i < randomTextureSize; ++i)
	{
		for (int j = 0; j < 128; ++j)
		{
				XMFLOAT3 v((float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX);
				colour[i * randomTextureSize + j] = XMFLOAT4( v.x, v.y, v.z, 0.0f );
		}
	}
	
	initData.pSysMem = colour;
	
	ID3D11Texture2D* tex = 0;
	_pd3dDevice->CreateTexture2D(&texDesc, &initData, &tex);
	
	_pd3dDevice->CreateShaderResourceView(tex, 0, &_randomVectorSRV);
	
	// view saves a reference.
	if (tex) tex->Release();
}

void SSAO::BuildOffsetVectors()
{
	_offsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	_offsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
	
	_offsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	_offsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);
	
	_offsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	_offsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);
	
	_offsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	_offsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);
	
	// 6 centers of cube faces
	_offsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	_offsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);
	
	_offsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	_offsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);
	
	_offsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	_offsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; ++i)
	{
		// Create random lengths in [0.25, 1.0].
		float s = 0.25f + ((float)(rand()) / (float)RAND_MAX) * (0.25 - 1.0f);

		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&_offsets[i]));

		XMStoreFloat4(&_offsets[i], v);
	}
}

void SSAO::BuildFullScreenQuad()
{
	SimpleVertex vertices[4];

	vertices[0].PosL = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[1].PosL = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	vertices[2].PosL = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	vertices[3].PosL = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	//Store far plane frustum corner indices in normal.x slot
	//and the texture coordinates in the yz
	vertices[0].NormL = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertices[1].NormL = XMFLOAT3(1.0f, 0.0f, 0.0f);
	vertices[2].NormL = XMFLOAT3(2.0f, 1.0f, 0.0f);
	vertices[3].NormL = XMFLOAT3(3.0f, 1.0f, 1.0f);

	//vertices[0].Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertices[1].Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertices[2].Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//vertices[3].Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//
	//vertices[0].Tex = XMFLOAT2(0.0f, 1.0f);
	//vertices[1].Tex = XMFLOAT2(0.0f, 0.0f);
	//vertices[2].Tex = XMFLOAT2(1.0f, 0.0f);
	//vertices[3].Tex = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	//vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(SimpleVertex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	ZeroMemory(&vinitData, sizeof(vinitData));
	vinitData.pSysMem = vertices;

	_pd3dDevice->CreateBuffer(&vbd, &vinitData, &_fullScreenQuadVertexBuffer);

	WORD indices[6] =
	{
		0,1,2,
		0,2,3
	};

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	//ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(WORD) * 6;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	ZeroMemory(&iinitData, sizeof(iinitData));
	iinitData.pSysMem = indices;

	_pd3dDevice->CreateBuffer(&ibd, &iinitData, &_fullScreenQuadIndexBuffer);
}