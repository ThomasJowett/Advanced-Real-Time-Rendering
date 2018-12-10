#include "SSAO.h"
#include "GeometryGenerator.h"

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

	_mesh = new Mesh(GeometryGenerator::CreateFullScreenQuad(), device);
	BuildSamplers();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	_pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);
}

SSAO::~SSAO()
{
	delete _mesh;

	if (_randomVectorSRV) _randomVectorSRV->Release();

	ReleaseTextureViews();
}

ID3D11ShaderResourceView * SSAO::NormalDepthSRV()
{
	return _normalDepthSRV;
}

ID3D11ShaderResourceView * SSAO::AmbientSRV()
{
	return _ambientSRV0;
}

void SSAO::SetNormalDepthRenderTarget(ID3D11DepthStencilView * depthStencilView)
{
	ID3D11RenderTargetView* renderTargets[1] = { _normalDepthRTV };
	_pImmediateContext->OMSetRenderTargets(1, renderTargets, depthStencilView);

	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	_pImmediateContext->ClearRenderTargetView(_normalDepthRTV, clearColor);
}

void SSAO::ComputeSSAO(const Camera * camera)
{
	//Assumes that the vertex and pixel shaders are already set

	ID3D11RenderTargetView* renderTargets[1] = { _ambientRTV0 };
	_pImmediateContext->OMSetRenderTargets(1, renderTargets, 0);
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
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

	//set samplers
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerNormalDepth);
	_pImmediateContext->PSSetSamplers(1, 1, &_pSamplerRandomVector);

	//Bind the constant buffers
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_pImmediateContext->IASetVertexBuffers(0, 1, &_mesh->_vertexBuffer, &_mesh->_vertexBufferStride, &_mesh->_vertexBufferOffset);
	_pImmediateContext->IASetIndexBuffer(_mesh->_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_pImmediateContext->DrawIndexed(_mesh->_numberOfIndices, 0, 0);
}

void SSAO::BuildSamplers()
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerNormalDepth);
	
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

	_pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerRandomVector);
}

void SSAO::BlurAmbientMap(int blurCount)
{
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
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
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
	texDesc.Width = 256;
	texDesc.Height = 256;
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
	initData.SysMemPitch = 256 * sizeof(XMFLOAT4);

	XMFLOAT4 color[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			XMFLOAT3 v((float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX, (float)(rand()) / (float)RAND_MAX);

			color[i * 256 + j] = XMFLOAT4(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

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
