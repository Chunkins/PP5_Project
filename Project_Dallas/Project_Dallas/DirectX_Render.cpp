#include "DirectX_Render.h"




DirectX_Render::DirectX_Render()
{
	//Gonna get back to this later don't delete
	/*Model Plane, Box, Teddy;
	Models.push_back(Plane);
	Models.push_back(Box);
	Models.push_back(Teddy);*/
	

	Plane.SetMName("plane.obj");
	Box.SetMName("Box_Idle.fbx");
	Box.SetTName((wchar_t)"TestCube.dds");
	Teddy.SetMName("Teddy_Idle.fbx");
	Teddy.SetTName((wchar_t)"Teddy_D.dds");





	NodeList.push_back(Plane);
	NodeList.push_back(Box);
	NodeList.push_back(Teddy);


	prevTime = 0;
	currTime = 0;
}


DirectX_Render::~DirectX_Render()
{
}

void DirectX_Render::InitPipeline(void)
{
	// load and compile the two shaders
	ID3D10Blob *VS, *PS;
	//auto loadVSTask = DX::ReadDataAsync()


	D3DCompileFromFile(L"ModelVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"LightPixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &PS, NULL);

	// encapsulate both shaders into shader objects
	dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC reallayout[] =
	{
		{ "BONECOUNT", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};



	dev->CreateInputLayout(reallayout, ARRAYSIZE(reallayout), VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
}

void DirectX_Render::Update(void)
{
	NodeList[0].GetModel()->WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(10.f, 10.f, 10.f);
	//Plane .WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(10.f, 10.f, 10.f);
	NodeList[1].GetModel()->WVP = XMMatrixTranslation(-5.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);
	//Box.WVP = XMMatrixTranslation(-5.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);
	//*Box.parentWVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);
	//Box.WVP.r[3].m128_f32[1] += .001f;

	NodeList[2].GetModel()->WVP = XMMatrixTranslation(5.0f / 0.05f, 0.f, 0.f) * XMMatrixScaling(0.05f, 0.05f, 0.05f);
	//Teddy.WVP = XMMatrixTranslation(5.0f / 0.05f, 0.f, 0.f) * XMMatrixScaling(0.05f, 0.05f, 0.05f);

	if (Swap) // If we're swapping between the models place them in the center
	{
		NodeList[1].GetModel()->WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);
		//Box.WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);
		NodeList[2].GetModel()->WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(0.05f, 0.05f, 0.05f);
		//Teddy.WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(0.05f, 0.05f, 0.05f);
	}

	double totalRotation;
	// Light calculations

	float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);

	totalRotation = GetCurrentTime() * radiansPerSecond * .001f;
	float radians = static_cast<float>(fmod(totalRotation, XM_2PI));


	float radius = 7.0f;
	float offset = 0;
	XMFLOAT4 lightPos = XMFLOAT4(sin(totalRotation + offset) * radius, 7.0f, std::cos(totalRotation + offset) * radius, 1.0f);
	XMVECTOR lightDir = XMVectorSet(-lightPos.x, -lightPos.y, -lightPos.z, 1.0f);
	XMStoreFloat3(&directionlightDir, lightDir);

	// spot light calculations
	float radius2 = 6.5f;
	float offset2 = 0;
	XMFLOAT4 lightPos2;//XMFLOAT4(sin(offset2) * radius2, 1.5f, std::cos(offset2) * radius, 1.0f);
	XMStoreFloat4(&lightPos2, camPosition);
	XMVECTOR lightDir2 = XMVectorSet(-lightPos2.x, -lightPos2.y, -lightPos2.z, 1.0f);
	XMStoreFloat3(&spotLightPos, lightDir2);

	// Directional
	if (GetAsyncKeyState('0'))
		m_lightChoice = 0;
	if (GetAsyncKeyState('1'))
		m_lightChoice = 1;
	// Point
	if (GetAsyncKeyState('2'))
		m_lightChoice = 2;
	// Spot and Diirectional
	if (GetAsyncKeyState('3'))
		m_lightChoice = 3;
	//Directional & Point
	if (GetAsyncKeyState('4'))
		m_lightChoice = 4;
	//swap model
	if (GetAsyncKeyState(VK_LSHIFT) & 0x1)
		SwapModel();
	if (GetAsyncKeyState(VK_RSHIFT) & 0x1)
	{
		RSswap = !RSswap;
		ToggleWireFrame();
	}
	if (GetAsyncKeyState('R') & 0x1)
		Reset();
	if (GetAsyncKeyState('F') & 0x1) {
		++Teddy.GetModel()->frame;
		++Box.GetModel()->frame;
		if (Teddy.GetModel()->frame>Teddy.GetModel()->anim.time-1)
		{
			Teddy.GetModel()->frame = 1;
		}
		if (Box.GetModel()->frame>Box.GetModel()->anim.time - 1)
		{
			Box.GetModel()->frame = 1;
		}
	}

	UpdateLights();
	UpdateCamera(.01f, 5.0f);
}

void DirectX_Render::UpdateCamera(float const moveSpd, float rotSpd)
{
	prevTime = currTime;
	currTime = GetCurrentTime();
	float deltaTime = (currTime - prevTime);

	if (GetAsyncKeyState('W'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * deltaTime);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('S'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * deltaTime);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('A'))
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * deltaTime, 0.0f, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('D'))
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * deltaTime, 0.0f, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('X'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * deltaTime, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * deltaTime, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}

	//Mouse input

	bool test = false;
	if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
	{
		GetCursorPos(&m_currMousePos);
		if (m_prevMousePos.x != NULL && m_prevMousePos.y != NULL)
		{
			if (abs(m_currMousePos.x - m_prevMousePos.x) > 0 && abs(m_currMousePos.y - m_prevMousePos.y) > 0)
			{
				test = true;

				float dx = m_currMousePos.x - m_prevMousePos.x;
				float dy = m_currMousePos.y - m_prevMousePos.y;

				XMFLOAT4X4 m_camera;
				XMStoreFloat4x4(&m_camera, camView);
				XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

				m_camera._41 = 0;
				m_camera._42 = 0;
				m_camera._43 = 0;

				XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
				XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

				XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
				temp_camera = XMMatrixMultiply(rotX, XMMatrixInverse(nullptr, temp_camera));
				temp_camera = XMMatrixMultiply(XMMatrixInverse(nullptr, temp_camera), rotY);

				XMStoreFloat4x4(&m_camera, temp_camera);


				m_camera._41 = pos.x;
				m_camera._42 = pos.y;
				m_camera._43 = pos.z;

				camView = XMLoadFloat4x4(&m_camera);
			}
		}

		m_prevMousePos = m_currMousePos;
	}

	else
	{
		m_prevMousePos.x = NULL;
		m_prevMousePos.y = NULL;

	}


}

void DirectX_Render::InitD3D(HWND hWnd)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&dev,
		NULL,
		&devcon);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use the back buffer address to create the render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();


	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencilDesc.Width = SCREEN_WIDTH;
	depthStencilDesc.Height = SCREEN_HEIGHT;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 4;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	//Create the Depth/Stencil View


	dev->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
	dev->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);

	//Set our Render Target
	devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);
	// set the render target as the back buffer

	//devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	//lights 
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	dev->CreateBuffer(&lightBufferDesc, nullptr, &m_lightBuffer);


	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	devcon->RSSetViewports(1, &viewport);

	//WVP constant buffer
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dev->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	//framebuffer
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(Float4x4)*40;
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dev->CreateBuffer(&cbbd, NULL, &frameBufer);

	// cameraBuffer
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(Float4x4);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dev->CreateBuffer(&cbbd, NULL, &cameraBuffer);

	//Camera information
	camPosition = XMVectorSet(10.0f, 20.0f, 10.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	camProjection = XMMatrixPerspectiveFovLH(70 * 3.14 / 180, SCREEN_WIDTH / SCREEN_HEIGHT, 0.0001f, 1000.0f);

	InitPipeline();
	InitGraphics();

}
void DirectX_Render::UpdateLights(void)
{
	D3D11_MAPPED_SUBRESOURCE lightData;
	LightBufferType* lightPointer;
	unsigned int bufferNumber;
	XMFLOAT4X4 m_camera;
	XMStoreFloat4x4(&m_camera, camView);
	switch (m_lightChoice)
	{
	case 0:
		devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightData);
		lightPointer = (LightBufferType*)lightData.pData;
		lightPointer->diffuseColor = XMFLOAT4(0.7, .5, .5, 1);
		lightPointer->lightDirection = directionlightDir;
		lightPointer->lightType.x = 1.0;
		lightPointer->lightType.y = m_camera._11;
		lightPointer->lightType.z = m_camera._12;
		lightPointer->lightType.w = m_camera._13;
		devcon->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		devcon->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		break;
	case 1:
		devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightData);
		lightPointer = (LightBufferType*)lightData.pData;
		lightPointer->diffuseColor = XMFLOAT4(0.7, .5, .5, 1);
		lightPointer->lightDirection = directionlightDir;
		lightPointer->lightType.x = 1.0;
		lightPointer->lightType.y = m_camera._41;
		lightPointer->lightType.z = m_camera._42;
		lightPointer->lightType.w = m_camera._43;
		devcon->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		devcon->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		break;
	case 2:
		devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightData);
		lightPointer = (LightBufferType*)lightData.pData;
		lightPointer->diffuseColor = XMFLOAT4(0.2, 1.0, 0.6, 1.0);
		lightPointer->lightDirection = directionlightDir;
		lightPointer->lightType.x = 2.0;
		lightPointer->lightType.y = m_camera._41;
		lightPointer->lightType.z = m_camera._42;
		lightPointer->lightType.w = m_camera._43;
		devcon->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		devcon->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		break;
	case 3:
		devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightData);
		lightPointer = (LightBufferType*)lightData.pData;
		lightPointer->diffuseColor = XMFLOAT4(0.2, 1.0, 0.3, 1.0);
		lightPointer->lightDirection = spotLightPos;
		lightPointer->lightType.x = 3.0;
		lightPointer->lightType.y = m_camera._41;
		lightPointer->lightType.z = m_camera._42;
		lightPointer->lightType.w = m_camera._43;
		devcon->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		devcon->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		break;
	case 4:
		devcon->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightData);
		lightPointer = (LightBufferType*)lightData.pData;
		lightPointer->diffuseColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
		lightPointer->lightDirection = XMFLOAT3(0.2, 3.0, 1.0);
		lightPointer->lightType.x = 4.0;
		lightPointer->lightType.y = m_camera._41;
		lightPointer->lightType.z = m_camera._42;
		lightPointer->lightType.w = m_camera._43;
		devcon->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		devcon->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		break;
	}
}
void DirectX_Render::InitGraphics(void)
{
	World = XMMatrixIdentity();
	//texturing
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	dev->CreateSamplerState(&samplerDesc, &m_sampler);
	CreateDDSTextureFromFile(dev, L"BoneTexture.dds", nullptr, &pSRVB);
	EXP::DLLTransit tmp;
	Animation anime;
	vector<BoneInfo> bonevec;
	std::vector<VertexInfo> kindaTMP;
	tmp.saveFiletoBin("Bone.fbx", "Bone.txt");
	
	tmp.loadFilefromBin("Bone.txt", kindaTMP, bonevec, anime);
	indexCountB = kindaTMP.size();
	Vertex* OurVertices = new Vertex[indexCountB];
	unsigned i = UINT32_MAX; while (++i != indexCountB)
	{
		OurVertices[i].position.x = kindaTMP[i].pos.x;
		OurVertices[i].position.y = kindaTMP[i].pos.y;
		OurVertices[i].position.z = kindaTMP[i].pos.z;
		OurVertices[i].position.w = 1.f;


		OurVertices[i].normal.x = kindaTMP[i].norm.x;
		OurVertices[i].normal.y = kindaTMP[i].norm.y;
		OurVertices[i].normal.z = kindaTMP[i].norm.z;
		OurVertices[i].uv.x = kindaTMP[i].uv.u;
		OurVertices[i].uv.y = 1 - kindaTMP[i].uv.v;
	}
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(Vertex) * indexCountB;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = OurVertices;
	dev->CreateBuffer(&bd, &vertexBufferData, &pVBufferB);// create buffer
	delete[] OurVertices;

	Plane.GetModel()->parentWVP = &World;

	//Plane.WVP = XMMatrixScaling(10.f, 10.f, 10.f);
	//for (size_t i = 0; i < NodeList.size(); i++)
	//{
	//	const wchar_t * tmp = NodeList[i].GetTName();
	//	NodeList[i].GetModel()->Init(dev, NodeList[i].GetMName(), tmp);
	//}

	NodeList[0].GetModel()->Init(dev, "plane.obj", nullptr);
	NodeList[1].GetModel()->InitFBX(dev, "Box_Idle.fbx", L"TestCube.dds", &World, true);
	NodeList[2].GetModel()->InitFBX(dev, "Teddy_Idle.fbx", L"Teddy_D.dds", &World, true);
	//Teddy.InitFBX(dev, "Teddy_Idle.fbx", L"Teddy_D.dds", &World, true);
	NodeList[0].InitRenderStateFill(dev);
	NodeList[1].InitRenderStateWireFrame(dev);
	NodeList[2].InitRenderStateWireFrame(dev);





	//set constant buffers
	devcon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	devcon->VSSetConstantBuffers(1, 1, &frameBufer);
}





void DirectX_Render::RenderFrame(void)
{
	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	////////////////////////////////////////
	// set model resources for standard models
	///////////////////////////////////////
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->PSSetSamplers(0, 1, &m_sampler);
	devcon->IASetInputLayout(pLayout);
	XMMATRIX camProj = camView* camProjection;



	devcon->RSSetState(NodeList[0].GetRS());
	NodeList[0].GetModel()->DrawIndexed(dev, devcon, cbPerObjectBuffer, camProj);
	devcon->RSSetState(NodeList[1].GetRS());
	NodeList[1].GetModel()->Draw(devcon, cbPerObjectBuffer, camProj, true, pSRVB,pVBufferB,indexCountB,frameBufer);
	devcon->RSSetState(NodeList[2].GetRS());
	NodeList[2].GetModel()->Draw(devcon, cbPerObjectBuffer, camProj, true, pSRVB,pVBufferB,indexCountB,frameBufer);


	/*Box.Draw(devcon, cbPerObjectBuffer, camView, camProjection, true);
	Teddy.Draw(devcon, cbPerObjectBuffer, camView, camProjection, true);
	Plane.DrawIndexed(dev, devcon, cbPerObjectBuffer, camView, camProjection);*/

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}


void DirectX_Render::CleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

	// close and release all existing COM objects
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	swapchain->Release();
	backbuffer->Release();
	cbPerObjectBuffer->Release();
	frameBufer->Release();
	m_lightBuffer->Release();
	cameraBuffer->Release();
	pVBufferB->Release();
	pSRVB->Release();


	dev->Release();
	devcon->Release();
	//	squareIndexBuffer->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
}

void DirectX_Render::SwapModel()
{
	//Temporary
	Swap = true;
	if (NodeList[1].GetModel()->Display)
	{
	
		NodeList[1].GetModel()->Display = false; NodeList[2].GetModel()->Display = true;
	}
	else
	{
		NodeList[1].GetModel()->Display = true; NodeList[2].GetModel()->Display = false;
	}
}
void DirectX_Render::Reset()
{
	
	Swap = false;
	
	NodeList[1].GetModel()->Display = true; NodeList[2].GetModel()->Display = true;
}

void DirectX_Render::ToggleWireFrame()
{
	if (RSswap)
	{
		NodeList[1].InitRenderStateFill(dev);
		NodeList[2].InitRenderStateFill(dev);
	}
	else
	{
		NodeList[1].InitRenderStateWireFrame(dev);
		NodeList[2].InitRenderStateWireFrame(dev);
	}
}
