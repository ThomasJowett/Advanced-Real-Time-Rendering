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
#include "Terrain.h"
#include "AnimatedModel.h"

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
#include "SSAO.h"

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
	ID3D11VertexShader*     _pSSAOVertexShader;
	ID3D11PixelShader*      _pSSAOPixelShader;
	ID3D11VertexShader*     _pSSAONormalDepthVertexShader;
	ID3D11PixelShader*      _pSSAONormalDepthPixelShader;
	ID3D11VertexShader*     _pSSAOBlurVertexShader;
	ID3D11PixelShader*      _pSSAOBlurPixelShader;
	ID3D11VertexShader*		_pSkinnedVertexShader;

	ID3D11HullShader*		_pHullShader = nullptr;
	ID3D11DomainShader*		_pDomainShader = nullptr;
	ID3D11DomainShader*		_pDisplacementDomainShader = nullptr;
	ID3D11VertexShader*     _pTesselationVertexShader;
	ID3D11PixelShader*      _pTesselationPixelShader;
	ID3D11PixelShader*      _pDisplacmentPixelShader;

	ID3D11VertexShader*     _pTerrainVertexShader = nullptr;
	ID3D11HullShader*		_pTerrainHullShader = nullptr;
	ID3D11DomainShader*		_pTerrainDomainShader = nullptr;
	ID3D11PixelShader*      _pTerrainPixelShader = nullptr;

	ID3D11VertexShader*     _pTerrainShadowVertexShader = nullptr;
	ID3D11HullShader*		_pTerrainShadowHullShader = nullptr;
	ID3D11DomainShader*		_pTerrainShadowDomainShader = nullptr;

	ID3D11InputLayout*      _pVertexLayout;
	ID3D11InputLayout*      _pPostProcessLayout;
	ID3D11InputLayout*		_pSSOALayout;
	ID3D11InputLayout*		_pTerrainLayout;
	ID3D11InputLayout*		_pSkinnedLayout;

	Mesh*					_fullscreenQuad;

	ID3D11Buffer*           _pConstantBuffer;
	ID3D11Buffer*			_pTessConstantBuffer;
	ID3D11Buffer*			_pSkinnedConstantBuffer;

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
	SSAO* _pSSAO;

	Terrain _terrain;

	AnimatedModel* _character;

	vector<GameObject *> _gameObjects;

	Camera * _camera;
	float _cameraOrbitRadius = 7.0f;
	float _cameraOrbitRadiusMin = 2.0f;
	float _cameraOrbitRadiusMax = 50.0f;
	float _cameraOrbitAngleXZ = -90.0f;
	float _cameraSpeed = 10.0f;
	float _cameraRotaion = 2.0f;
	float _mouseSensitivity = 0.01f;
	bool _walkCamera = true;
	bool _rightMouseButtonHeld = false;
	int _mouseRawX;
	int _mouseRawY;

	int _lastCursorPosX;
	int _lastCursorPosY;

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
	void SetShader(Shader shaderToUse);

	ID3D11RasterizerState* ViewMode();

	void DrawSceneToShadowMap();
	void DrawSceneToSSAODepthMap();

	void DrawImGui();

	void GetWindowPosition(int&X, int&Y);

	float counter = 0.01f;
	float heightMapScale = 0.04f;

	int textureToShow = 0;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg, float deltaTime);
	bool HandleMouse(MSG msg, float deltaTime);

	void Update(float deltaTime);
	void Draw();
};

