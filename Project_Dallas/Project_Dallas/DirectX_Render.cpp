#include "DirectX_Render.h"
#include <fstream>




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

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC reallayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	dev->CreateInputLayout(reallayout, ARRAYSIZE(reallayout), VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);


}

void DirectX_Render::Update(void)
{

	Plane.WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(10.f, 10.f, 10.f);

	Box.WVP = XMMatrixTranslation(0.0f, 0.f, 0.f) * XMMatrixScaling(1.f, 1.f, 1.f);

	UpdateCamera(1.0f, 1.0f);
}

void DirectX_Render::UpdateCamera(float const moveSpd, float rotSpd)
{

	if (GetAsyncKeyState('W'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * .001);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('S'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * .001);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('A'))
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * .001, 0.0f, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('D'))
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * .001, 0.0f, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}
	if (GetAsyncKeyState('X'))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * .001, 0.0f);
		XMMATRIX temp_camera = camProjection;
		XMMATRIX result = XMMatrixMultiply(XMMatrixInverse(nullptr, translation), temp_camera);
		camProjection = result;
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * .001, 0.0f);
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

	dev->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	//Camera information
	camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
	camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Set the Projection matrix
	camProjection = XMMatrixPerspectiveFovLH(70 * 3.14 / 180, SCREEN_WIDTH / SCREEN_HEIGHT, 0.0001f, 1000.0f);

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
	swapchain->Release();
	backbuffer->Release();
	cbPerObjectBuffer->Release();

	dev->Release();
	devcon->Release();
	//	squareIndexBuffer->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
}

void DirectX_Render::RenderFrame(void)
{
	// clear the back buffer to a deep blue
	float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// set shaders
	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	devcon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);
	// select which primtive type we are using
	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->PSSetSamplers(0, 1, &m_sampler);

	//I tried to change this to 2 and it didnt work
	devcon->IASetInputLayout(pLayout);
	//Draw the first cube

	Box.Draw(dev, devcon, cbPerObjectBuffer, camView, camProjection, false);
	Plane.DrawIndexed(dev, devcon, cbPerObjectBuffer, camView, camProjection);

	// switch the back buffer and the front buffer
	swapchain->Present(0, 0);
}

void DirectX_Render::InitGraphics(void)
{
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

	Plane.Init(dev, "plane.obj", nullptr);
	Box.InitFBX(dev, "../Box_Idle.fbx", L"TestCube.dds");






	// copy the vertices into the buffer
//D3D11_MAPPED_SUBRESOURCE ms;
//devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
//memcpy(ms.pData, OurVertices, sizeof(OurVertices));                 // copy the data
//devcon->Unmap(pVBuffer, NULL);
//delete[] indices;
}


