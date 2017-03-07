#include "DirectX_Render.h"



DirectX_Render::DirectX_Render()
{
}


DirectX_Render::~DirectX_Render()
{
}

void DirectX_Render::InitPipeline(void)
{
	// load and compile the two shaders
	ID3D10Blob *VS, *PS;

	D3DCompileFromFile(L"shaders.shader", NULL, NULL, "VShader", "vs_4_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"shaders.shader", NULL, NULL, "PShader", "ps_4_0", 0, 0, &PS, NULL);




	// encapsulate both shaders into shader objects
	dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

	// set the shader objects
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);
}

void DirectX_Render::Update(void)
{

		//Keep the cubes rotating
		rot += .0005f;
		if (rot > 6.26f)
			rot = 0.0f;

		//Reset cube1World
		cube1World = XMMatrixIdentity();

		//Define cube1's world space matrix
		XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		Rotation = XMMatrixRotationAxis(rotaxis, rot);
		Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

		//Set cube1's world space using the transformations
		cube1World = Translation * Rotation;

		//Reset cube2World
		cube2World = XMMatrixIdentity();

		//Define cube2's world space matrix
		Rotation = XMMatrixRotationAxis(rotaxis, -rot);
		Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);

		//Set cube2's world space matrix
		cube2World = Rotation * Scale;

		UpdateCamera(1.0f, 0.75f);
}

void DirectX_Render::UpdateCamera(float const moveSpd, float rotSpd)
{
	
	if (GetAsyncKeyState('W'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * .001);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
	}
	if (GetAsyncKeyState('S'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * .001);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
	}
	if (GetAsyncKeyState('A'))
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * .001, 0.0f, 0.0f);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
	}
	if (GetAsyncKeyState('D'))
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * .001, 0.0f, 0.0f);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
	}
	if (GetAsyncKeyState('X'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * .001, 0.0f);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * .001, 0.0f);
		XMMATRIX temp_camera = camView;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr,translation), temp_camera);
		camView = result;
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
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Create the Depth/Stencil View
	dev->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	dev->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	//Set our Render Target
	//devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);
	// set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);


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

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	dev->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	//Camera information
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	camProjection = XMMatrixPerspectiveFovLH(0.4f*3.14f, SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 1000.0f);

	InitPipeline();
	InitGraphics();
}

void DirectX_Render::CleanD3D(void)
{
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

												   // close and release all existing COM objects
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	pVBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	cbPerObjectBuffer->Release();

	dev->Release();
	devcon->Release();
	squareIndexBuffer->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
}

void DirectX_Render::RenderFrame(void)
{
	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);
	//Refresh the Depth/Stencil view
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// do 3D rendering on the back buffer here
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

	// select which primtive type we are using
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set the WVP matrix and send it to the constant buffer in effect file
	WVP = cube1World * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	devcon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devcon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw the first cube
	devcon->DrawIndexed(36, 0, 0);

	WVP = cube2World * camView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	devcon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	devcon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw the second cube
	devcon->DrawIndexed(36, 0, 0);

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}

void DirectX_Render::InitGraphics(void)
{


	// create a triangle using the VERTEX struct

	VERTEX OurVertices[] =
	{
		{ -1.0f, -1.0f, -1.0f,{ 1.0f, 0.0f, 0.0f, 1.0f } },
		{ -1.0f,  1.0f, -1.0f,{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ 1.0f,  1.0f, -1.0f,{ 0.0f, 0.0f, 1.0f, 1.0f } },
		{ 1.0f, -1.0f, -1.0f,{ 1.0f, 1.0f, 0.0f, 1.0f } },
		{ -1.0f, -1.0f, 1.0f,{ 0.0f, 1.0f, 1.0f, 1.0f } },
		{ -1.0f, 1.0f, 1.0f,{ 1.0f, 1.0f, 1.0f, 1.0f } },
		{ 1.0f, 1.0f, 1.0f,{ 1.0f, 0.0f, 1.0f, 1.0f } },
		{ 1.0f, -1.0f, 1.0f,{ 1.0f, 0.0f, 0.0f, 1.0f } },


	};

	DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices;
	dev->CreateBuffer(&indexBufferDesc, &iinitData, &squareIndexBuffer);

	devcon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(VERTEX) * 8;             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer



	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = OurVertices;
	dev->CreateBuffer(&bd, &vertexBufferData, &pVBuffer);       // create the buffer

												   // copy the vertices into the buffer
	//D3D11_MAPPED_SUBRESOURCE ms;
	//devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	//memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
	//devcon->Unmap(pVBuffer, NULL);
}


