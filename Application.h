#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "Camera.h"
#include "Commons.h"

#include <vector>
/*
//#include <SpriteFont.h>
#include "CommonStates.h"
//#include "DDSTextureLoader.h"
#include "Effects.h"
#include "GeometricPrimitive.h"
#include "Model.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
*/
#include "GameObject.h"
#include "ShadowMapping.h"

using namespace DirectX;



class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pNormalVertexShader;
	ID3D11PixelShader*      _pNormalPixelShader;
	ID3D11VertexShader*     _pParralaxVertexShader;
	ID3D11PixelShader*      _pParralaxPixelShader;
	ID3D11VertexShader*     _pParralaxOcclusionVertexShader;
	ID3D11PixelShader*      _pParralaxOcclusionPixelShader;
	ID3D11PixelShader*      _pBlockColourPixelShader;
	ID3D11VertexShader*     _pPassThroughVertexShader;
	ID3D11PixelShader*      _pNoPostProcessPixelShader;
	ID3D11PixelShader*      _pGaussianBlurPixelShader;
	ID3D11PixelShader*      _pBloomPixelShader;
	ID3D11VertexShader*     _pShadowMapVertexShader;

	ID3D11HullShader*		_pHullShader = nullptr;
	ID3D11DomainShader*		_pDomainShader = nullptr;
	ID3D11DomainShader*		_pDisplacementDomainShader = nullptr;
	ID3D11VertexShader*     _pTesselationVertexShader;
	ID3D11PixelShader*      _pTesselationPixelShader;

	ID3D11InputLayout*      _pVertexLayout;
	ID3D11InputLayout*      _pPostProcessLayout;

	Mesh*					_fullscreenQuad;

	ID3D11Buffer*           _pConstantBuffer;

	ID3D11DepthStencilView*		_depthStencilView = nullptr;
	ID3D11Texture2D*			_depthStencilBuffer = nullptr;
	ID3D11RenderTargetView*		_renderTargetView;

	//Render To Texture
	ID3D11Texture2D*			_RTTRenderTargetTexture = nullptr;
	ID3D11ShaderResourceView*	_RTTshaderResourceView = nullptr;
	ID3D11RenderTargetView*		_RTTRenderTargetView;

	ID3D11ShaderResourceView * _pDiffuseStoneTextureRV = nullptr;
	ID3D11ShaderResourceView * _pNormalStoneTextureRV = nullptr;

	ID3D11ShaderResourceView * _pDiffuseGroundTextureRV = nullptr;
	ID3D11ShaderResourceView * _pNormalGroundTextureRV = nullptr;

	ID3D11ShaderResourceView * _pVingetteTextureRV = nullptr;

	ID3D11SamplerState * _pSamplerLinear = nullptr;
	ID3D11SamplerState * _pSamplerShadow = nullptr;

	Light basicLight;

	ShadowMap* _pShadowMap;

	vector<GameObject *> _gameObjects;

	Camera * _camera;
	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = -90.0f;
	float _cameraSpeed = 2.0f;

	UINT _WindowHeight;
	UINT _WindowWidth;

	// Render dimensions - Change here to alter screen resolution
	UINT _renderHeight = 1080;
	UINT _renderWidth = 1920;

	UINT _shadowMapHeight = 4096;
	UINT _shadowMapWidth = 4096;

	D3D11_VIEWPORT _vp;

	ID3D11DepthStencilState* DSLessEqual;
	ID3D11RasterizerState* RSCull;
	ID3D11RasterizerState* RSCullNone;
	ID3D11RasterizerState* RSWireFrame;
	ID3D11RasterizerState* _pCurrentState;

	ID3D11RasterizerState* CCWcullMode;
	ID3D11RasterizerState* CWcullMode;

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();

	void moveForward(int objectNumber);
	void Rotate(int objectNumber);

	ID3D11RasterizerState* ViewMode();

	void DrawSceneToShadowMap();

	float counter = 0.01f;
	float heightMapScale = 0.01f;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg);

	void Update(float deltaTime);
	void Draw();
};

