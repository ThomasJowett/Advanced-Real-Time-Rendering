#include "Application.h"
#include "GeometryGenerator.h"

#include "ObJLoader.h"
#include "PostProcess.h"
#include <iostream>
#include "ColladaLoader.h"
#include "ProceduralLandscape.h"

#include "imGUI/imgui.h"
#include "imGUI/imgui_impl_dx11.h"
#include "imGUI/imgui_impl_win32.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


bool Application::HandleKeyboard(MSG msg, float deltaTime)
{
	switch (msg.wParam)
	{

	}

	

	return false;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Application::HandleMouse(MSG msg, float deltaTime)
{
	if (ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam))
		return true;
	int x, y;
	float delta;
	switch (msg.message)
	{
	case WM_MOUSEMOVE:
		x = LOWORD(msg.lParam);
		y = HIWORD(msg.lParam);
		return true;
	case WM_LBUTTONDOWN:
		return true;
	case WM_RBUTTONDOWN:
		_lastCursorPosX = LOWORD(msg.lParam);
		_lastCursorPosY = HIWORD(msg.lParam);
		ShowCursor(FALSE);
		_rightMouseButtonHeld = true;
		return true;
	case WM_RBUTTONUP:
		ShowCursor(TRUE);
		GetWindowPosition(x, y);
		SetCursorPos(_lastCursorPosX + x + 8, _lastCursorPosY + y + 31);
		_rightMouseButtonHeld = false;
		return true;
	case WM_MBUTTONDOWN:
		return true;
	case WM_MOUSEWHEEL:
		delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);

		if (delta > 0)
		{
			_cameraSpeed++;
		}
		else if (delta < 0)
		{
			_cameraSpeed--;
			if (_cameraSpeed < 1.0f)
				_cameraSpeed = 1.0f;
		}
		return true;
	case WM_INPUT:
		UINT dataSize;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

		if (dataSize > 0)
		{
			std::unique_ptr<BYTE[]> rawData = std::make_unique<BYTE[]>(dataSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam), RID_INPUT, rawData.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					_mouseRawX = raw->data.mouse.lLastX;
					_mouseRawY = raw->data.mouse.lLastY;
				}
			}
		}
		if (DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam) == FALSE)
			return false;
		
		return true;
	}

	

	return false;
}

Application::Application()
	:_pShadowMap(0)
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pNormalVertexShader = nullptr;
	_pNormalPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pTerrainLayout = nullptr;
	_pSkinnedLayout = nullptr;
	_pConstantBuffer = nullptr;
	_pTessConstantBuffer = nullptr;

	DSLessEqual = nullptr;
	RSCullNone = nullptr;
	RSCullNone = nullptr;
	RSWireFrame = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	//setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(_pd3dDevice, _pImmediateContext);
	ImGui::StyleColorsLight();

	//Load Textures ------------------------------------------------------------------------------------------------
	//TODO: make a texture 2d manager that automatically releases all the textures when program closes
	ID3D11ShaderResourceView * _pNormalEarthTextureRV;
	ID3D11ShaderResourceView *_pDiffuseEarthTextureRV;
	ID3D11ShaderResourceView * _pHeightEarthTextureRV;

	ID3D11ShaderResourceView * _pNormalSpaceManTextureRV;
	ID3D11ShaderResourceView *_pDiffuseSpaceManTextureRV;

	

	ID3D11ShaderResourceView * _pNormalCrateTextureRV;
	ID3D11ShaderResourceView *_pDiffuseCrateTextureRV;
	ID3D11ShaderResourceView *_pHeightCrateTextureRV;

	ID3D11ShaderResourceView *_pHeightGroundTextureRV;

	ID3D11ShaderResourceView * _pNormalGunTextureRV;
	ID3D11ShaderResourceView *_pDiffuseGunTextureRV;
	ID3D11ShaderResourceView *_pHeightGunTextureRV;
	ID3D11ShaderResourceView *_pMetalGunTextureRV;
	ID3D11ShaderResourceView *_pAOGunTextureRV;
	ID3D11ShaderResourceView *_pEmissiveGunTextureRV;

	AnimatedModelData modelData = ColladaLoader::LoadModel("Resources/model.dae", 4);

	AnimationData animationData = ColladaLoader::LoadAnimation("Resources/model.dae");

	Animation* animation = new Animation(animationData);

	Mesh SpaceManGeometry(modelData.ToIndexedModel(), _pd3dDevice);

	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Diffuse.dds", nullptr, &_pDiffuseGroundTextureRV);
	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Height.dds", nullptr, &_pHeightGroundTextureRV);
	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Normal.dds", nullptr, &_pNormalGroundTextureRV);

	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\conenormal.dds", nullptr, &_pNormalGroundTextureRV);
	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\coneHeight.dds", nullptr, &_pHeightGroundTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\SpaceMan_Diffuse.dds", nullptr, &_pDiffuseSpaceManTextureRV);
	
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\SpaceMan_Normal.dds", nullptr, &_pNormalSpaceManTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Model.dds", nullptr, &_pDiffuseManTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Man_normal.dds", nullptr, &_pNormalManTextureRV);
	

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Earth_Normal.dds", nullptr, &_pNormalEarthTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Earth_Diffuse.dds", nullptr, &_pDiffuseEarthTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Earth_Height.dds", nullptr, &_pHeightEarthTextureRV);
	
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Stone_Diffuse.dds", nullptr, &_pDiffuseStoneTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Stone_Normal.dds", nullptr, &_pNormalStoneTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_NRM.dds", nullptr, &_pNormalCrateTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_COLOR.dds", nullptr, &_pDiffuseCrateTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_HEIGHT.dds", nullptr, &_pHeightCrateTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Pebbles_height.dds", nullptr, &_pHeightGroundTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Pebbles_albedo.dds", nullptr, &_pDiffuseGroundTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Pebbles_normal.dds", nullptr, &_pNormalGroundTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Vingette.dds", nullptr, &_pVingetteTextureRV);
	
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_Albedo.dds", nullptr, &_pDiffuseGunTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_Normal.dds", nullptr, &_pNormalGunTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_Height.dds", nullptr, &_pHeightGunTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_AO.dds", nullptr, &_pAOGunTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_Metallic.dds", nullptr, &_pMetalGunTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\GunGS_Emissive.dds", nullptr, &_pEmissiveGunTextureRV);

    // Setup Camera
	XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -1.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 2.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 2000.0f);

	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.Direction = XMFLOAT3(10.0f, 10.0f, -10.0f);

	Mesh cubeGeometry(GeometryGenerator::CreateCube(1.0f, 1.0f,1.0f),_pd3dDevice);

	Mesh planeGeometry(GeometryGenerator::CreateGrid(250.0f, 250.0f, 100, 100, 15, 15), _pd3dDevice);

	Mesh sphereGeometry(GeometryGenerator::CreateSphere(0.5f, 20, 20), _pd3dDevice);

	Mesh cylinderGeometry(GeometryGenerator::CreateCylinder(0.5f,0.5f,1.0f, 20, 2), _pd3dDevice);

	Mesh torusGeometry(GeometryGenerator::CreateTorus(0.7f, 0.25f, 15), _pd3dDevice);

	//Mesh SpaceManGeometry(OBJLoader::Load("Resources\\SpaceMan.obj", true), _pd3dDevice);

	Mesh GunGeometry(OBJLoader::Load("Resources\\Gun.obj", true), _pd3dDevice);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Resources\\terrain.raw";
	tii.LayerMapFilename0 = L"Resources\\grass.dds";
	tii.LayerMapFilename1 = L"Resources\\darkdirt.dds";
	tii.LayerMapFilename2 = L"Resources\\stone.dds";
	tii.LayerMapFilename3 = L"Resources\\lightdirt.dds";
	tii.LayerMapFilename4 = L"Resources\\snow.dds";
	tii.BlendMapFilename = L"Resources\\blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightMapWidth = 2049;
	tii.HeightMapHeight = 2049;
	tii.CellSpacing = 0.5f;

	_terrain.Init(_pd3dDevice, _pImmediateContext, tii, ProceduralLandscape::LoadHeightMap(tii));


	_character = new AnimatedModel(modelData, _pDiffuseManTextureRV, _pd3dDevice);

	_character->DoAnimation(animation);

	_fullscreenQuad = new Mesh(GeometryGenerator::CreateFullScreenQuad(), _pd3dDevice);

	Material shinyMaterial;
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	Material metal;
	metal.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	metal.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	metal.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	metal.specularPower = 20.0f;

	Material noSpecMaterial;
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;

	Transform* transform = new Transform(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(0.0f, 0.0f, 0.0f), Vector3D(1.0f, 1.0f, 1.0f));
	
	GameObject * gameObject = new GameObject("Floor", transform, planeGeometry, noSpecMaterial);

	gameObject->SetTextureRV(_pDiffuseGroundTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalGroundTextureRV, TX_NORMAL);
	gameObject->SetTextureRV(_pHeightGroundTextureRV, TX_HEIGHTMAP);
	
	gameObject->SetShaderToUse(FX_PARRALAXED_OCCLUSION);
	//gameObject->SetShaderToUse(FX_PARRALAXED);
	//gameObject->SetShaderToUse(FX_DISPLACEMENT);
	//gameObject->SetShaderToUse(FX_WIREFRAME);
	
	//_gameObjects.push_back(gameObject);

	transform = new Transform(Vector3D(0.0f, _terrain.GetHeight(0.0f, 0.0f) + 1.0f, -10.0f), Vector3D(XM_PIDIV2, 0, 0), Vector3D(1.0f, 1.0f, 1.0f));
	
	gameObject = new GameObject("Crate", transform, cubeGeometry, shinyMaterial);
	gameObject->SetTextureRV(_pDiffuseCrateTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalCrateTextureRV, TX_NORMAL);
	gameObject->SetTextureRV(_pHeightCrateTextureRV, TX_HEIGHTMAP);
	//gameObject->SetShaderToUse(FX_PARRALAXED);
	//gameObject->SetShaderToUse(FX_DISPLACEMENT);
	
	_gameObjects.push_back(gameObject);
	
	transform = new Transform(Vector3D(2.0f, _terrain.GetHeight(0.0f, 0.0f) + 1.0f, 0.0f), Vector3D(-XM_PIDIV2, XM_PI, 0), Vector3D(0.2f, 0.2f, 0.2f));
	
	gameObject = new GameObject("Cowboy", transform, SpaceManGeometry, noSpecMaterial);
	gameObject->SetTextureRV(_pDiffuseManTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalManTextureRV, TX_NORMAL);
	
	_gameObjects.push_back(gameObject);
	
	transform = new Transform(Vector3D(-5.0f, _terrain.GetHeight(0.0f, 0.0f) + 0.6f, 0.0f), Vector3D(0, 0, 0), Vector3D(0.01f, 0.01f, 0.01f));
	
	gameObject = new GameObject("Gun", transform, GunGeometry, metal); 
	gameObject->SetTextureRV(_pDiffuseGunTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalGunTextureRV, TX_NORMAL);
	
	
	_gameObjects.push_back(gameObject);
	
	Vector3D position;

	for (auto i = 0; i < 5; i++)
	{
		position = Vector3D(-4.0f + (i * 2.0f), 0.0f, 10.0f);
		position.y = _terrain.GetHeight(position.x, position.y) + 1.0f;
		transform = new Transform(position, Vector3D(0.0f, 0.0f, 0.0f), Vector3D(1.0f, 1.0f, 1.0f));
		if (i == 1)
		{
			gameObject = new GameObject("Cylinder " + std::to_string(i), transform, cylinderGeometry, shinyMaterial);
		}
		else if (i == 2)
		{
			gameObject = new GameObject("Sphere " + std::to_string(i), transform, sphereGeometry, shinyMaterial);
		}
		else if (i == 3)
		{
			transform->_rotation = Vector3D(XM_PIDIV2, 0, 0);
			gameObject = new GameObject("Torus " + std::to_string(i), transform, torusGeometry, shinyMaterial);
			gameObject->SetShaderToUse(FX_BLOCK_COLOUR);
		}
		else if (i == 4)
		{
			gameObject = new GameObject("Cube Tesselation", transform, cubeGeometry, shinyMaterial);
			gameObject->SetShaderToUse(FX_DISPLACEMENT);
		}
		else
		{
			gameObject = new GameObject("Cube " + std::to_string(i), transform, cubeGeometry, shinyMaterial);
			gameObject->SetShaderToUse(FX_WIREFRAME);
		}
		gameObject->SetTextureRV(_pDiffuseStoneTextureRV,TX_DIFFUSE);
		gameObject->SetTextureRV(_pNormalStoneTextureRV, TX_NORMAL);
		gameObject->SetTextureRV(_pHeightCrateTextureRV, TX_HEIGHTMAP);
	
		_gameObjects.push_back(gameObject);
	}

	//Create the shadow map
	_pShadowMap = new ShadowMap(_pd3dDevice, _shadowMapWidth, _shadowMapHeight);
	_pSSAO = new SSAO(_pd3dDevice, _pImmediateContext, _renderWidth, _renderHeight, _camera->GetFovY(), _camera->GetFarDepth());
	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

    // Compile the normal map vertex shader
    HR(CompileShaderFromFile(L"DX11 Framework.fx", "NormalVS", "vs_5_0", &pVSBlob));
	HR(_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pNormalVertexShader));

	// Compile the normal map pixel shader
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "NormalPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pNormalPixelShader);
	
	//Compile the simple Parralax map vertex shader
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "SimpleParralaxVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pParralaxVertexShader);

	//Compile the simple Parralax map pixel shader
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "SimpleParralaxPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pParralaxPixelShader);

	//Compile the Parralax map vertex shader
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "ParralaxVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pParralaxOcclusionVertexShader);

	//Compile the Parralax map pixel shader
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "ParralaxPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pParralaxOcclusionPixelShader);

	//Compile the block colour pixel shader
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "BlockColourPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pBlockColourPixelShader);

	//Compile the Shadow map vertex shader
	hr = CompileShaderFromFile(L"ShadowMap.fx", "ShadowMapVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pShadowMapVertexShader);

	

	//Compile the SSAO Normal Depth vertex shader
	hr = CompileShaderFromFile(L"SSAONormalDepth.fx", "SSAONormalDepthVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pSSAONormalDepthVertexShader);

	//Compile the SSAO Normal Depth pixel shader
	hr = CompileShaderFromFile(L"SSAONormalDepth.fx", "SSAONormalDepthPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pSSAONormalDepthPixelShader);

	//Compile the SSAO Blur vertex shader
	hr = CompileShaderFromFile(L"SSAOBlur.fx", "BlurVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pSSAOBlurVertexShader);

	//Compile the SSAO Blur pixel shader
	hr = CompileShaderFromFile(L"SSAOBlur.fx", "BlurPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pSSAOBlurPixelShader);
	

    if (FAILED(hr))
        return hr;
	
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	

	//Compile the PassThrough vertex shader
	hr = CompileShaderFromFile(L"PostProcess.fx", "PassThroughVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pPassThroughVertexShader);

	//Compile the No Post Process pixel shader
	hr = CompileShaderFromFile(L"PostProcess.fx", "NoPostProcessPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pNoPostProcessPixelShader);

	//Compile the Gaussian blur pixel shader
	hr = CompileShaderFromFile(L"PostProcess.fx", "GaussianBlurPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pGaussianBlurPixelShader);
	
	//Compile the bloom pixel shader
	hr = CompileShaderFromFile(L"PostProcess.fx", "BloomPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pBloomPixelShader);

	 // Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutPostProcess[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	numElements = ARRAYSIZE(layoutPostProcess);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutPostProcess, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pPostProcessLayout);

	if (FAILED(hr))
        return hr;

	ID3DBlob * pHSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tesselation.fx", "MainHS", "hs_5_0", &pHSBlob);
	hr = _pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &_pHullShader);
	
	ID3DBlob * pDSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tesselation.fx", "DSMAIN", "ds_5_0", &pDSBlob);
	hr = _pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &_pDomainShader);

	hr = CompileShaderFromFile(L"Tesselation.fx", "DisplacementDS", "ds_5_0", &pDSBlob);
	hr = _pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &_pDisplacementDomainShader);

	hr = CompileShaderFromFile(L"Tesselation.fx", "TesselationVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pTesselationVertexShader);

	hr = CompileShaderFromFile(L"Tesselation.fx", "TesselationPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pTesselationPixelShader);

	hr = CompileShaderFromFile(L"Tesselation.fx", "DisplacementPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pDisplacmentPixelShader);

	//Compile the SSAO vertex shader
	hr = CompileShaderFromFile(L"SSAO.fx", "SSAOVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pSSAOVertexShader);

	//Compile the SSAO pixel shader
	hr = CompileShaderFromFile(L"SSAO.fx", "SSAOPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pSSAOPixelShader);

	D3D11_INPUT_ELEMENT_DESC layoutSSOA[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	numElements = ARRAYSIZE(layoutSSOA);

	if (FAILED(hr))
		return hr;

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutSSOA, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pSSOALayout);

	//Terrain Shaders
	hr = CompileShaderFromFile(L"Terrain.fx", "TerrainVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pTerrainVertexShader);

	hr = CompileShaderFromFile(L"Terrain.fx", "TerrainPS", "ps_5_0", &pPSBlob);
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pTerrainPixelShader);

	hr = CompileShaderFromFile(L"Terrain.fx", "MainHS", "hs_5_0", &pHSBlob);
	hr = _pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &_pTerrainHullShader);

	hr = CompileShaderFromFile(L"Terrain.fx", "DS", "ds_5_0", &pDSBlob);
	hr = _pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &_pTerrainDomainShader);

	hr = CompileShaderFromFile(L"ShadowMap.fx", "TerrainDS", "ds_5_0", &pDSBlob);
	hr = _pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &_pTerrainShadowDomainShader);

	//Compile the terrain Shadow map vertex shader
	hr = CompileShaderFromFile(L"ShadowMap.fx", "ShadowMapTerrainVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pTerrainShadowVertexShader);

	hr = CompileShaderFromFile(L"ShadowMap.fx", "TerrainHS", "hs_5_0", &pHSBlob);
	hr = _pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &_pTerrainShadowHullShader);

	D3D11_INPUT_ELEMENT_DESC layoutTerrain[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	numElements = ARRAYSIZE(layoutTerrain);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutTerrain, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pTerrainLayout);

	if (FAILED(hr))
		return hr;

	hr = CompileShaderFromFile(L"SkinnedMesh.fx", "SkinnedVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pSkinnedVertexShader);

	D3D11_INPUT_ELEMENT_DESC layoutSkinned[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	numElements = ARRAYSIZE(layoutSkinned);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layoutSkinned, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pSkinnedLayout);

	if (FAILED(hr))
		return hr;

	pVSBlob->Release();
	pPSBlob->Release();
	pHSBlob->Release();
	pDSBlob->Release();

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerShadow);

	return hr;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, (LONG)_renderWidth, (LONG)_renderHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"Advanced Real Time Rendering", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

	

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

		MessageBox(nullptr,
			L"The FX file cannot compile shader.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 1;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _renderWidth;
    sd.BufferDesc.Height = _renderHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = sampleCount;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    // Setup the viewport
    _vp.Width = (FLOAT)_renderWidth;
    _vp.Height = (FLOAT)_renderHeight;
    _vp.MinDepth = 0.0f;
    _vp.MaxDepth = 1.0f;
    _vp.TopLeftX = 0;
    _vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &_vp);

	InitShadersAndInputLayout();

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TesselationConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pTessConstantBuffer);

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SkinnedConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pSkinnedConstantBuffer);

    if (FAILED(hr))
        return hr;

	//Depth Stencil Description
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _renderWidth;
	depthStencilDesc.Height = _renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	//Texture description
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = _renderWidth;
	textureDesc.Height = _renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	//create the texture
	_pd3dDevice->CreateTexture2D(&textureDesc, nullptr, &_RTTRenderTargetTexture);

	//Setup the description of the render target view
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	hr = _pd3dDevice->CreateRenderTargetView(_RTTRenderTargetTexture, &renderTargetViewDesc, &_RTTRenderTargetView);

	if (FAILED(hr))
		return hr;

	//Setup the shader Resource view description

	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	_pd3dDevice->CreateShaderResourceView(_RTTRenderTargetTexture, &shaderResourceViewDesc, &_RTTshaderResourceView);


	//_pImmediateContext->OMSetRenderTargets(1, &_RTTRenderTargetView, _depthStencilView);

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCull);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_WIREFRAME;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSWireFrame);

	_pCurrentState = RSCull;

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	_pd3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual);

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode);

	cmdesc.FrontCounterClockwise = false;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode);

	

    return S_OK;
}

void Application::Cleanup()
{

    if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pSamplerLinear) _pSamplerLinear->Release();
	if (_pSamplerShadow) _pSamplerShadow->Release();

	//if (_pTextureRV) _pTextureRV->Release();

	//if (_pGroundTextureRV) _pGroundTextureRV->Release();

    if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pTessConstantBuffer) _pTessConstantBuffer->Release();

    if (_pVertexLayout) _pVertexLayout->Release();
	if (_pPostProcessLayout) _pPostProcessLayout->Release();
	if (_pTerrainLayout) _pTerrainLayout->Release();
	if (_pSSOALayout) _pSSOALayout->Release();

    if (_pNormalVertexShader) _pNormalVertexShader->Release();
    if (_pNormalPixelShader) _pNormalPixelShader->Release();
	if (_pParralaxVertexShader) _pParralaxVertexShader->Release();
	if (_pParralaxPixelShader) _pParralaxPixelShader->Release();
	if (_pParralaxOcclusionVertexShader) _pParralaxOcclusionVertexShader->Release();
	if (_pParralaxOcclusionPixelShader) _pParralaxOcclusionPixelShader->Release();
	if (_pTesselationPixelShader) _pTesselationPixelShader->Release();
	if (_pTesselationVertexShader) _pTesselationVertexShader->Release();
	if (_pBlockColourPixelShader) _pBlockColourPixelShader->Release();
	if (_pNoPostProcessPixelShader) _pNoPostProcessPixelShader->Release();
	if (_pBloomPixelShader) _pBloomPixelShader->Release();
	if (_pGaussianBlurPixelShader) _pGaussianBlurPixelShader->Release();
	if (_pPassThroughVertexShader) _pPassThroughVertexShader->Release();
	if (_pShadowMapVertexShader)_pShadowMapVertexShader->Release();
	if (_pHullShader) _pHullShader->Release();
	if (_pDomainShader) _pDomainShader->Release();
	if (_pSSAOBlurVertexShader) _pSSAOBlurVertexShader->Release();
	if (_pDisplacementDomainShader) _pDisplacementDomainShader->Release();
	if (_pSSAOBlurPixelShader) _pSSAOBlurPixelShader->Release();
	if (_pTerrainVertexShader) _pTerrainVertexShader->Release();
	if (_pTerrainHullShader) _pTerrainHullShader->Release();
	if (_pTerrainDomainShader) _pTerrainDomainShader->Release();
	if (_pTerrainPixelShader) _pTerrainPixelShader->Release();
	if (_pTerrainShadowVertexShader) _pTerrainShadowVertexShader->Release();
	if (_pTerrainShadowHullShader) _pTerrainShadowHullShader->Release();
	if (_pTerrainShadowDomainShader) _pTerrainShadowDomainShader->Release();
	if (_pDisplacmentPixelShader) _pDisplacmentPixelShader->Release();

    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();

	if (DSLessEqual) DSLessEqual->Release();
	if (RSCull) RSCull->Release();
	if (RSCullNone) RSCullNone->Release();
	if (RSWireFrame) RSWireFrame->Release();

	if (CCWcullMode) CCWcullMode->Release();
	if (CWcullMode) CWcullMode->Release();

	if (_camera)
	{
		delete _camera;
		_camera = nullptr;
	}

	for (auto gameObject : _gameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}

	if (_character) delete _character;

}

void Application::moveForward(int objectNumber)
{
	Vector3D position = _gameObjects[objectNumber]->GetTransform()->_position;
	position.z -= 0.1f;
	_gameObjects[objectNumber]->GetTransform()->_position = position;
}

void Application::Rotate(int objectNumber)
{
	Quaternion rotation = _gameObjects[objectNumber]->GetTransform()->_rotation;

	rotation.AddScaledVector(Vector3D(0.0f, 1.0f, 0.0f), 0.01f);
	rotation.Normalize();
	_gameObjects[objectNumber]->GetTransform()->_rotation = rotation;
}

void Application::SetShader(Shader shaderToUse)
{
	_gameObjects[0]->SetShaderToUse(shaderToUse);
	_gameObjects[1]->SetShaderToUse(shaderToUse);
}

ID3D11RasterizerState * Application::ViewMode()
{
	//Switch View mode with F1 and F2
	if (GetAsyncKeyState(VK_F1) & 0x8000)//F1
	{
		_pCurrentState = RSWireFrame;
	}
	else if (GetAsyncKeyState(VK_F2) & 0x8000)//F2
	{
		_pCurrentState = RSCull;
	}
	else if (GetAsyncKeyState(VK_F3) & 0x8000)//F3
	{
		_pCurrentState = RSCullNone;
	}

	return(_pCurrentState);
}

void Application::DrawSceneToShadowMap()
{
	XMMATRIX view = XMLoadFloat4x4(&_pShadowMap->GetView());
	XMMATRIX projection = XMLoadFloat4x4(&_pShadowMap->GetProjection());

	ShadowMapConstantBuffer cb;

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	cb.EyePosW = _camera->GetPosition();

	//Render Terrain -------------------------------------------------------------------

	_pImmediateContext->IASetInputLayout(_pTerrainLayout);

	_pImmediateContext->VSSetShader(_pTerrainShadowVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->HSSetShader(_pTerrainShadowHullShader, nullptr, 0);
	_pImmediateContext->DSSetShader(_pTerrainShadowDomainShader, nullptr, 0);

	_pImmediateContext->VSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->DSSetSamplers(0, 1, &_pSamplerLinear);

	_terrain.DrawToShadowMap(_pImmediateContext, cb, _camera);

	//Render animated Model -----------------------------------------------------------

	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_pImmediateContext->IASetInputLayout(_pSkinnedLayout);
	_pImmediateContext->VSSetShader(_pSkinnedVertexShader, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

	SkinnedConstantBuffer skinCb;

	skinCb.ViewProjection = XMMatrixTranspose(XMMatrixMultiply(view, projection));

	XMMATRIX* boneMatrices = reinterpret_cast<XMMATRIX*>(skinCb.WorldMatrixArray);

	_character->GetJointTransforms(boneMatrices);

	skinCb.ViewProjection = XMMatrixTranspose(XMMatrixMultiply(view, projection));

	_pImmediateContext->UpdateSubresource(_pSkinnedConstantBuffer, 0, nullptr, &skinCb, 0, 0);

	_character->Draw(_pImmediateContext);

	//Render All Other Objects ---------------------------------------------------------


	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);

	_pImmediateContext->VSSetShader(_pShadowMapVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto gameObject : _gameObjects)
	{
		cb.World = XMMatrixTranspose(gameObject->GetTransform()->GetWorldMatrix());

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		if (gameObject->GetShaderToUse() == FX_WIREFRAME)
		{
			_pImmediateContext->RSSetState(RSWireFrame);
		}
		else
		{
			_pImmediateContext->RSSetState(RSCull);
		}

		//Draw Object
		gameObject->Draw(_pImmediateContext);

	}
}

void Application::DrawSceneToSSAODepthMap()
{
	XMMATRIX view = XMMatrixTranspose(XMLoadFloat4x4(&_camera->GetView()));
	XMMATRIX projection = XMMatrixTranspose(XMLoadFloat4x4(&_camera->GetProjection()));


	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->VSSetShader(_pSSAONormalDepthVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pSSAONormalDepthPixelShader, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	XMMATRIX world;

	ShadowMapConstantBuffer cb;

	cb.View = view;
	cb.Projection = projection;

	for (auto gameObject : _gameObjects)
	{
		world = XMMatrixTranspose(gameObject->GetTransform()->GetWorldMatrix());
		cb.World = world;
		
		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	
		if (gameObject->GetShaderToUse() == FX_WIREFRAME)
		{
			_pImmediateContext->RSSetState(RSWireFrame);
		}
		else
		{
			_pImmediateContext->RSSetState(RSCull);
		}
	
		//Draw Object
		gameObject->Draw(_pImmediateContext);
	}
}

void Application::DrawImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Createtestwindow
	//ImGui::Begin("Test");
	//ImGui::Text("%i", _lastCursorPosX);
	//ImGui::End();

	_mouseRawX = 0.0f;
	_mouseRawY = 0.0f;

	//ImGui::ShowDemoWindow();

	//Assemble together Draw data
	ImGui::Render();

	//RenderDrawData
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Application::GetWindowPosition(int & X, int & Y)
{
	RECT rect = { NULL };
	if (GetWindowRect(_hWnd, &rect))
	{
		X = rect.left;
		Y = rect.top;
	}
}

void Application::Update(float deltaTime)
{
	// Move gameobject
	if (GetAsyncKeyState('1'))
	{
		moveForward(1);
	}

	if (GetAsyncKeyState('2'))
	{
		Rotate(1);
		Rotate(3);
	}

	if (GetAsyncKeyState('Z'))
	{
		heightMapScale += 0.001f;
	}

	if (GetAsyncKeyState('X'))
	{
		heightMapScale -= 0.001f;
	}

	if (GetAsyncKeyState('3'))
	{
		SetShader(FX_NORMAL);
	}

	if (GetAsyncKeyState('4'))
	{
		SetShader(FX_PARRALAXED);
	}

	if (GetAsyncKeyState('5'))
	{
		SetShader(FX_PARRALAXED_OCCLUSION);
	}

	if (GetAsyncKeyState('6'))
	{
		SetShader(FX_BLOCK_COLOUR);
	}

	if (GetAsyncKeyState('7'))
	{
		SetShader(FX_DISPLACEMENT);
	}

	if (GetAsyncKeyState('8'))
	{
		SetShader(FX_WIREFRAME);
	}

	if (GetAsyncKeyState('C') & 0x8000)
	{
		_walkCamera = !_walkCamera;
	}

	if (GetAsyncKeyState(VK_F4) & 0x8000)//F1
	{
		textureToShow = 0;
	}
	else if (GetAsyncKeyState(VK_F5) & 0x8000)//F2
	{
		textureToShow = 1;
	}
	else if (GetAsyncKeyState(VK_F6) & 0x8000)//F3
	{
		textureToShow = 2;
	}
	else if (GetAsyncKeyState(VK_F7) & 0x8000)//F3
	{
		textureToShow = 3;
	}
	else if (GetAsyncKeyState(VK_F8) & 0x8000)//F3
	{
		textureToShow = 4;
	}


	//Control Camera
	if (GetAsyncKeyState('W') & 0x8000)
		_camera->MoveForward(_cameraSpeed*deltaTime);
	if (GetAsyncKeyState('S') & 0x8000)
		_camera->MoveForward(-_cameraSpeed * deltaTime);
	if (GetAsyncKeyState('A') & 0x8000)
		_camera->MoveRight(-_cameraSpeed * deltaTime);
	if (GetAsyncKeyState('D') & 0x8000)
		_camera->MoveRight(_cameraSpeed*deltaTime);
	if (GetAsyncKeyState('E') & 0x8000)
		_camera->MoveUp(_cameraSpeed*deltaTime);
	if (GetAsyncKeyState('Q') & 0x8000)
		_camera->MoveUp(-_cameraSpeed * deltaTime);

	if (GetAsyncKeyState('I') & 0x8000)
		_camera->Pitch(-_cameraRotaion*deltaTime);
	if (GetAsyncKeyState('K') & 0x8000)
		_camera->Pitch(_cameraRotaion * deltaTime);
	if (GetAsyncKeyState('J') & 0x8000)
		_camera->Yaw(-_cameraRotaion * deltaTime);
	if (GetAsyncKeyState('L') & 0x8000)
		_camera->Yaw(_cameraRotaion*deltaTime);


	if (_rightMouseButtonHeld && (_mouseRawX != 0 || _mouseRawY != 0)) 
	{
		_camera->Yaw((float)_mouseRawX * _mouseSensitivity);
		_camera->Pitch((float)_mouseRawY * _mouseSensitivity);
	}

	

	// Update camera
	//float angleAroundZ = XMConvertToRadians(_cameraOrbitAngleXZ);
	//
	//float x = _cameraOrbitRadius * cos(angleAroundZ);
	//float z = _cameraOrbitRadius * sin(angleAroundZ);
	//
	//XMFLOAT3 cameraPos = _camera->GetPosition();
	//cameraPos.x = x;
	//cameraPos.z = z;

	//_camera->SetPosition(cameraPos);

	if(_walkCamera)
		_camera->SetPosition(Vector3D(_camera->GetPosition().x, _terrain.GetHeight(_camera->GetPosition().x, _camera->GetPosition().z) + 2.0f, _camera->GetPosition().z));
	_camera->Update();

	_character->GetTransform()->_position.y = _terrain.GetHeight(_character->GetTransform()->_position.x, _character->GetTransform()->_position.z);
	_character->Update(deltaTime);

	// Update objects
	for (auto gameObject : _gameObjects)
	{
		gameObject->Update(deltaTime);
	}

	counter+= deltaTime;

	//basicLight.Direction.x = sin(counter)*10;

	_pShadowMap->BuildShadowTransforms(basicLight, _camera->GetPosition());
}

void Application::Draw()
{
	
	//bind the shadow map render target
	_pShadowMap->BindDsvAndSetNullRenderTarget(_pImmediateContext);

	DrawSceneToShadowMap();

	//create a null texture
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	_pImmediateContext->PSSetShaderResources(3, 2, null);

	//Set the view port
	_pImmediateContext->RSSetViewports(1, &_vp);

	//bind the ssao depth map render target
	_pSSAO->SetNormalDepthRenderTarget(_depthStencilView);

	DrawSceneToSSAODepthMap();

	_pImmediateContext->IASetInputLayout(_pSSOALayout);

	_pImmediateContext->VSSetShader(_pSSAOVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pSSAOPixelShader, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

	_pSSAO->ComputeSSAO(_camera);

	_pImmediateContext->VSSetShader(_pSSAOBlurVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pSSAOBlurPixelShader, nullptr, 0);

	//_pSSAO->BlurAmbientMap(4);

	//Set the view port
	_pImmediateContext->RSSetViewports(1, &_vp);

	//Set the render target
	_pImmediateContext->OMSetRenderTargets(1, &_RTTRenderTargetView, _depthStencilView);

	//
    // Clear buffers
    //
	float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_RTTRenderTargetView, ClearColor);//grey
	ClearColor[1] = 0.0f;
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);//purple
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //
    // Setup buffers and render scene
    //
	_pImmediateContext->RSSetState(ViewMode());

	//Render the terrain

	_pImmediateContext->IASetInputLayout(_pTerrainLayout);

	_pImmediateContext->VSSetShader(_pTerrainVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pTerrainPixelShader, nullptr, 0);
	_pImmediateContext->HSSetShader(_pTerrainHullShader, nullptr, 0);
	_pImmediateContext->DSSetShader(_pTerrainDomainShader, nullptr, 0);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->VSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->DSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->PSSetSamplers(2, 1, &_pSamplerShadow);

	ID3D11ShaderResourceView * textureRV;

	textureRV = _pShadowMap->GetShaderResourceView();
	_pImmediateContext->PSSetShaderResources(7, 1, &textureRV);

	_terrain.Draw(_pImmediateContext, basicLight, _camera, _pShadowMap);

	_pImmediateContext->PSSetShaderResources(7, 1, null);

	//render animated Model

	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_pImmediateContext->IASetInputLayout(_pSkinnedLayout);
	//_pImmediateContext->IASetInputLayout(_pVertexLayout);
	_pImmediateContext->VSSetShader(_pSkinnedVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pNormalPixelShader, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->PSSetSamplers(1, 1, &_pSamplerShadow);

	XMFLOAT4X4 viewAsFloats = _camera->GetView();
	XMFLOAT4X4 projectionAsFloats = _camera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);
	XMMATRIX shadowTransform = XMLoadFloat4x4(&_pShadowMap->GetTransform());

	SkinnedConstantBuffer skinCb;
	ConstantBuffer cb;

	XMMATRIX* boneMatrices = reinterpret_cast<XMMATRIX*>(skinCb.WorldMatrixArray);

	_character->GetJointTransforms(boneMatrices);

	skinCb.ViewProjection = XMMatrixTranspose(XMMatrixMultiply(view, projection));
	skinCb.ShadowTransform = XMMatrixTranspose(shadowTransform);
	cb.HasTexture = 1.0f;

	Material material = _character->GetMaterial();

	// Copy material to shader
	cb.surface.AmbientMtrl = material.ambient;
	cb.surface.DiffuseMtrl = material.diffuse;
	cb.surface.SpecularMtrl = material.specular;

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	cb.ShadowTransform = XMMatrixTranspose(shadowTransform);

	cb.light = basicLight;
	cb.EyePosW = _camera->GetPosition();

	//cb.HeightMapScale = 0.01f;
	cb.MaxSamples = 1000;
	cb.MinSamples = 1;

	_pImmediateContext->VSSetConstantBuffers(1, 1, &_pSkinnedConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);

	_pImmediateContext->UpdateSubresource(_pSkinnedConstantBuffer, 0, nullptr, &skinCb, 0, 0);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

	textureRV = _character->GetTextureRV();
	_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
	textureRV = _pNormalManTextureRV;
	_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);

	_character->Draw(_pImmediateContext);

	//Render all other objects

	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);


	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
	_pImmediateContext->PSSetSamplers(1, 1, &_pSamplerShadow);
	_pImmediateContext->DSSetSamplers(0, 1, &_pSamplerLinear);


	textureRV = _pShadowMap->GetShaderResourceView();
	_pImmediateContext->PSSetShaderResources(3, 1, &textureRV);

	// Render all scene objects
	for (auto gameObject : _gameObjects)
	{
		// Get render material
		material = gameObject->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb.World = XMMatrixTranspose(gameObject->GetTransform()->GetWorldMatrix());
		

		switch (gameObject->GetShaderToUse())
		{
		case FX_NORMAL:
			_pImmediateContext->VSSetShader(_pNormalVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pNormalPixelShader, nullptr, 0);
			_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
			_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

			textureRV = gameObject->GetTextureRV(TX_DIFFUSE);
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_NORMAL);
			_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
			cb.HasTexture = 1.0f;
			break;
		case FX_PARRALAXED:
			cb.HeightMapScale = heightMapScale;
			_pImmediateContext->VSSetShader(_pParralaxVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pParralaxPixelShader, nullptr, 0);
			_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
			_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

			textureRV = gameObject->GetTextureRV(TX_DIFFUSE);
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_NORMAL);
			_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_HEIGHTMAP);
			_pImmediateContext->PSSetShaderResources(2, 1, &textureRV);
			cb.HasTexture = 1.0f;
			break;
		case FX_PARRALAXED_OCCLUSION:
			cb.HeightMapScale = heightMapScale;
			_pImmediateContext->VSSetShader(_pParralaxOcclusionVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pParralaxOcclusionPixelShader, nullptr, 0);
			_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
			_pImmediateContext->DSSetShader(nullptr, nullptr, 0);

			textureRV = gameObject->GetTextureRV(TX_DIFFUSE);
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_NORMAL);
			_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_HEIGHTMAP);
			_pImmediateContext->PSSetShaderResources(2, 1, &textureRV);
			cb.HasTexture = 1.0f;
			break;
		case FX_BLOCK_COLOUR:
			_pImmediateContext->VSSetShader(_pNormalVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pBlockColourPixelShader, nullptr, 0);
			//_pImmediateContext->PSSetShader(_pTesselationPixelShader, nullptr, 0);

			break;
		case FX_WIREFRAME:

			_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			_pImmediateContext->HSSetShader(_pHullShader, nullptr, 0);
			_pImmediateContext->DSSetShader(_pDomainShader, nullptr, 0);
			_pImmediateContext->RSSetState(RSWireFrame);
			_pImmediateContext->VSSetShader(_pTesselationVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pTesselationPixelShader, nullptr, 0);
			break;
		case FX_DISPLACEMENT:
			
			cb.HeightMapScale = heightMapScale;
			//_pImmediateContext->UpdateSubresource(_pTessConstantBuffer, 0, nullptr, &Tesscb, 0, 0);
			//
			_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			_pImmediateContext->HSSetShader(_pHullShader, nullptr, 0);
			_pImmediateContext->DSSetShader(_pDisplacementDomainShader, nullptr, 0);
			_pImmediateContext->VSSetShader(_pTesselationVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pDisplacmentPixelShader, nullptr, 0);
			textureRV = gameObject->GetTextureRV(TX_DIFFUSE);
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_NORMAL);
			_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_HEIGHTMAP);
			_pImmediateContext->DSSetShaderResources(2, 1, &textureRV);
			break;
		case FX_SKY:
			break;
		case FX_TERRAIN:
			break;
		default:
			break;
		}

		// Update constant buffer
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

		// Draw object
		gameObject->Draw(_pImmediateContext);
		_pImmediateContext->RSSetState(ViewMode());
		_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	
	_pImmediateContext->PSSetShaderResources(0, 2, null);
	_pImmediateContext->PSSetShaderResources(3, 2, null);

	//Switch to rendering to the back buffer
	_pImmediateContext->RSSetState(RSCull);
	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr);
	
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	_pImmediateContext->IASetInputLayout(_pPostProcessLayout);


	if(textureToShow == 0) textureRV = _RTTshaderResourceView;
	
	if(textureToShow == 1) textureRV = _pSSAO->NormalDepthSRV();
	if (textureToShow == 2) textureRV = _pSSAO->AmbientSRV();
	if (textureToShow == 3) textureRV = _pSSAO->RandomVectorSRV();
	if (textureToShow == 4) textureRV = _pShadowMap->GetShaderResourceView();
	_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
	textureRV = _pSSAO->AmbientSRV();
	//_pImmediateContext->PSSetShaderResources(1, 1, &_pVingetteTextureRV);
	_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
	_pImmediateContext->VSSetShader(_pPassThroughVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pNoPostProcessPixelShader, nullptr, 0);
	_pImmediateContext->HSSetShader(nullptr, nullptr, 0);
	_pImmediateContext->DSSetShader(nullptr, nullptr, 0);
	//_pImmediateContext->PSSetShader(_pGaussianBlurPixelShader, nullptr, 0);
	//_pImmediateContext->PSSetShader(_pBloomPixelShader, nullptr, 0);

	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &PostProcess::GaussianBlur(2.0f, _renderHeight, _renderWidth), 0, 0);
	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &PostProcess::Bloom(true, 150.0f, 2.0f, _renderHeight, _renderWidth), 0, 0);
	
	_pImmediateContext->IASetVertexBuffers(0, 1, &_fullscreenQuad->_vertexBuffer, &_fullscreenQuad->_vertexBufferStride, &_fullscreenQuad->_vertexBufferOffset);
	_pImmediateContext->IASetIndexBuffer(_fullscreenQuad->_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	
	_pImmediateContext->DrawIndexed(_fullscreenQuad->_numberOfIndices, 0, 0);
	
	//set the null texture
	_pImmediateContext->PSSetShaderResources(0, 2, null);
    //
    // Present our back buffer to our front buffer
    //

	DrawImGui();

    _pSwapChain->Present(0, 0);
}