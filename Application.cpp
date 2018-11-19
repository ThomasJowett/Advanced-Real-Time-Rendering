#include "Application.h"
#include "GeometryGenerator.h"
#include "ObJLoader.h"
#include "PostProcess.h"

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

bool Application::HandleKeyboard(MSG msg)
{
	XMFLOAT3 cameraPosition = _camera->GetPosition();

	switch (msg.wParam)
	{
	case VK_UP:
		_cameraOrbitRadius = max(_cameraOrbitRadiusMin, _cameraOrbitRadius - (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_DOWN:
		_cameraOrbitRadius = min(_cameraOrbitRadiusMax, _cameraOrbitRadius + (_cameraSpeed * 0.2f));
		return true;
		break;

	case VK_RIGHT:
		_cameraOrbitAngleXZ -= _cameraSpeed;
		return true;
		break;

	case VK_LEFT:
		_cameraOrbitAngleXZ += _cameraSpeed;
		return true;
		break;
	}

	return false;
}

Application::Application()
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
	_pConstantBuffer = nullptr;

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

	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Diffuse.dds", nullptr, &_pDiffuseGroundTextureRV);
	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Height.dds", nullptr, &_pHeightGroundTextureRV);
	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Floor_Normal.dds", nullptr, &_pNormalGroundTextureRV);

	//CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\conenormal.dds", nullptr, &_pNormalGroundTextureRV);

	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\SpaceMan_Diffuse.dds", nullptr, &_pDiffuseSpaceManTextureRV);
	
	CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\SpaceMan_Normal.dds", nullptr, &_pNormalSpaceManTextureRV);

	

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

	_camera = new Camera(eye, at, up, (float)_renderWidth, (float)_renderHeight, 0.01f, 200.0f);

	// Setup the scene's light
	basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	basicLight.SpecularPower = 20.0f;
	basicLight.LightPosW = XMFLOAT3(10.0f, 10.0f, -10.0f);

	Mesh cubeGeometry(GeometryGenerator::CreateCube(1.0f, 1.0f,1.0f),_pd3dDevice);

	Mesh planeGeometry(GeometryGenerator::CreateGrid(25.0f, 25.0f, 50, 50, 2, 2), _pd3dDevice);

	Mesh sphereGeometry(GeometryGenerator::CreateSphere(0.5f, 20.0f, 20.0f), _pd3dDevice);

	Mesh cylinderGeometry(GeometryGenerator::CreateCylinder(0.5f,0.5f,1.0f, 20, 2), _pd3dDevice);

	Mesh torusGeometry(GeometryGenerator::CreateTorus(0.7f, 0.25f, 15), _pd3dDevice);

	Mesh SpaceManGeometry(OBJLoader::Load("Resources\\SpaceMan.obj", true), _pd3dDevice);

	Mesh GunGeometry(OBJLoader::Load("Resources\\Gun.obj", true), _pd3dDevice);

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

	_gameObjects.push_back(gameObject);

	transform = new Transform(Vector3D(0.0f, 1.0f, 0.0f), Vector3D(XM_PIDIV2, 0, 0), Vector3D(1.0f, 1.0f, 1.0f));

	gameObject = new GameObject("Crate", transform, cubeGeometry, shinyMaterial);
	gameObject->SetTextureRV(_pDiffuseCrateTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalCrateTextureRV, TX_NORMAL);
	gameObject->SetTextureRV(_pHeightCrateTextureRV, TX_HEIGHTMAP);
	gameObject->SetShaderToUse(FX_PARRALAXED);

	_gameObjects.push_back(gameObject);

	transform = new Transform(Vector3D(5.0f, 1.0f, 0.0f), Vector3D(0, XM_PI, 0), Vector3D(0.01f, 0.01f, 0.01f));

	gameObject = new GameObject("SpaceMan", transform, SpaceManGeometry, shinyMaterial);
	gameObject->SetTextureRV(_pDiffuseSpaceManTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalSpaceManTextureRV, TX_NORMAL);

	_gameObjects.push_back(gameObject);

	transform = new Transform(Vector3D(-5.0f, 1.0f, 0.0f), Vector3D(0, 0, 0), Vector3D(0.01f, 0.01f, 0.01f));

	gameObject = new GameObject("Gun", transform, GunGeometry, metal); 
	gameObject->SetTextureRV(_pDiffuseGunTextureRV, TX_DIFFUSE);
	gameObject->SetTextureRV(_pNormalGunTextureRV, TX_NORMAL);

	_gameObjects.push_back(gameObject);

	Vector3D position;

	for (auto i = 0; i < 5; i++)
	{
		position = Vector3D(-4.0f + (i * 2.0f), 0.5f, 10.0f);
		transform = new Transform(position, Vector3D(0.0f, 0.0f, 0.0f), Vector3D(1.0f, 1.0f, 1.0f));
		if (i == 1)
		{
			gameObject = new GameObject("Cylinder " + i, transform, cylinderGeometry, shinyMaterial);
		}
		else if (i == 2)
		{
			gameObject = new GameObject("Sphere " + i, transform, sphereGeometry, shinyMaterial);
		}
		else if (i == 3)
		{
			transform->_rotation = Vector3D(XM_PIDIV2, 0, 0);
			gameObject = new GameObject("Torus " + i, transform, torusGeometry, shinyMaterial);
		}
		else
		{
			gameObject = new GameObject("Cube " + i, transform, cubeGeometry, shinyMaterial);
			gameObject->SetShaderToUse(FX_WIREFRAME);
		}
		gameObject->SetTextureRV(_pDiffuseStoneTextureRV,TX_DIFFUSE);
		gameObject->SetTextureRV(_pNormalStoneTextureRV, TX_NORMAL);

		_gameObjects.push_back(gameObject);
	}

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

    // Compile the normal map vertex shader
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "NormalVS", "vs_5_0", &pVSBlob);
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pNormalVertexShader);

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

	

	pPSBlob->Release();

    if (FAILED(hr))
        return hr;
	
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

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

	numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pPostProcessLayout);

	if (FAILED(hr))
        return hr;

	ID3DBlob * pHSBlob = nullptr;
	//hr = CompileShaderFromFile(L"Tesselation.fx", "MainHS", "hs_5_0", &pHSBlob);
	//hr = _pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &_pHullShader);

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
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, _renderWidth, _renderHeight};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
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
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_renderWidth;
    vp.Height = (FLOAT)_renderHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

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

	//if (_pTextureRV) _pTextureRV->Release();

	//if (_pGroundTextureRV) _pGroundTextureRV->Release();

    if (_pConstantBuffer) _pConstantBuffer->Release();

    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pNormalVertexShader) _pNormalVertexShader->Release();
    if (_pNormalPixelShader) _pNormalPixelShader->Release();
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
	}

	if (GetAsyncKeyState('8'))
	{
		heightMapScale += 0.001f;
	}

	if (GetAsyncKeyState('9'))
	{
		heightMapScale -= 0.001f;
	}

	// Update camera
	float angleAroundZ = XMConvertToRadians(_cameraOrbitAngleXZ);

	float x = _cameraOrbitRadius * cos(angleAroundZ);
	float z = _cameraOrbitRadius * sin(angleAroundZ);

	XMFLOAT3 cameraPos = _camera->GetPosition();
	cameraPos.x = x;
	cameraPos.z = z;

	_camera->SetPosition(cameraPos);
	_camera->Update();

	// Update objects
	for (auto gameObject : _gameObjects)
	{
		gameObject->Update(deltaTime);
	}

	counter+= deltaTime;

	basicLight.LightPosW.x = sin(counter)*10;
}

void Application::Draw()
{
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

	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);

    ConstantBuffer cb;

	XMFLOAT4X4 viewAsFloats = _camera->GetView();
	XMFLOAT4X4 projectionAsFloats = _camera->GetProjection();

	XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
	XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);
	
	cb.light = basicLight;
	cb.EyePosW = _camera->GetPosition();

	cb.HeightMapScale = 0.01f;
	cb.MaxSamples = 1000;
	cb.MinSamples = 1;

	// Render all scene objects
	for (auto gameObject : _gameObjects)
	{
		// Get render material
		Material material = gameObject->GetMaterial();

		// Copy material to shader
		cb.surface.AmbientMtrl = material.ambient;
		cb.surface.DiffuseMtrl = material.diffuse;
		cb.surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb.World = XMMatrixTranspose(gameObject->GetTransform()->GetWorldMatrix());
		ID3D11ShaderResourceView * textureRV;

		switch (gameObject->GetShaderToUse())
		{
		case FX_NORMAL:
			_pImmediateContext->VSSetShader(_pNormalVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pNormalPixelShader, nullptr, 0);

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

			textureRV = gameObject->GetTextureRV(TX_DIFFUSE);
			_pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_NORMAL);
			_pImmediateContext->PSSetShaderResources(1, 1, &textureRV);
			textureRV = gameObject->GetTextureRV(TX_HEIGHTMAP);
			_pImmediateContext->PSSetShaderResources(2, 1, &textureRV);
			cb.HasTexture = 1.0f;
			break;
		case FX_BLOCK_COLOUR:
			

			break;
		case FX_WIREFRAME:
			_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			_pImmediateContext->HSSetShader(_pHullShader, nullptr, 0);
			_pImmediateContext->RSSetState(RSWireFrame);
			_pImmediateContext->VSSetShader(_pNormalVertexShader, nullptr, 0);
			_pImmediateContext->PSSetShader(_pBlockColourPixelShader, nullptr, 0);
			break;
		case FX_SKY:
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

	//Switch to rendering to the back buffer
	_pImmediateContext->RSSetState(RSCull);
	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr);
	
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	_pImmediateContext->IASetInputLayout(_pPostProcessLayout);
	
	_pImmediateContext->PSSetShaderResources(0, 1, &_RTTshaderResourceView);
	_pImmediateContext->PSSetShaderResources(1, 1, &_pVingetteTextureRV);
	_pImmediateContext->VSSetShader(_pPassThroughVertexShader, nullptr, 0);
	//_pImmediateContext->PSSetShader(_pNoPostProcessPixelShader, nullptr, 0);
	//_pImmediateContext->PSSetShader(_pGaussianBlurPixelShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pBloomPixelShader, nullptr, 0);

	//_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &PostProcess::GaussianBlur(5.0f, _renderHeight, _renderWidth), 0, 0);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &PostProcess::Bloom(true, 150.0f, 2.0f, _renderHeight, _renderWidth), 0, 0);
	
	_pImmediateContext->IASetVertexBuffers(0, 1, &_fullscreenQuad->_vertexBuffer, &_fullscreenQuad->_vertexBufferStride, &_fullscreenQuad->_vertexBufferOffset);
	_pImmediateContext->IASetIndexBuffer(_fullscreenQuad->_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	
	_pImmediateContext->DrawIndexed(_fullscreenQuad->_numberOfIndices, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);
}