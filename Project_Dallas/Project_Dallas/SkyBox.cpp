#include "SkyBox.h"


SkyBox::SkyBox()
{
}

void SkyBox::Init(ID3D11Device * t_dev, char * filename)
{
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(ModelViewProjectionConstantBuffer);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	t_dev->CreateBuffer(&cbbd, NULL, &ConstantinoBiffer);

	ID3D10Blob *VS, *PS;

	D3DCompileFromFile(L"VertexShader.hlsl", NULL, NULL, "main", "vs_4_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"PixelShader.hlsl", NULL, NULL, "main", "ps_4_0", 0, 0, &PS, NULL);

	t_dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &skyboxVertexShader);
	t_dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &skyboxPixelShader);

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	t_dev->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), VS->GetBufferPointer(), VS->GetBufferSize(), &dLayout);

	//devcon->VSSetShader(pVS, 0, 0);
	//devcon->PSSetShader(pPS, 0, 0);
	//devcon->IASetInputLayout(pLayout);
	CreateDDSTextureFromFile(t_dev, L"BeachSkybox.dds", (ID3D11Resource**)&skyboxTexture, &skyboxModelView);

	XMFLOAT3 skyOffset = XMFLOAT3(1, 1, 1);

	static const VertexPositionColor skyboxVertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
	};

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = skyboxVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(skyboxVertices), D3D11_BIND_VERTEX_BUFFER);
	t_dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_skyboxVertexBuffer);

	static const unsigned short skyboxIndices[] =
	{
		2,1,0, // -x
		2,3,1,

		5,6,4, // +x
		7,6,5,

		1,5,0, // -y
		5,4,0,

		6,7,2, // +y
		7,3,2,

		4,6,0, // -z
		6,2,0,

		3,7,1, // +z
		7,5,1,
	};

	m_skyboxIndexCount = ARRAYSIZE(skyboxIndices);
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = skyboxIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(skyboxIndices), D3D11_BIND_INDEX_BUFFER);
	t_dev->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_skyboxIndexBuffer);
}


void SkyBox::Draw(ID3D11DeviceContext * t_devcon, XMMATRIX t_camProjection, XMMATRIX t_camView, XMFLOAT4X4 t_camera, XMMATRIX t_camworld)
{
	ID3D11ShaderResourceView *skyboxTexViews[] = { { skyboxModelView } };
	t_devcon->PSSetShaderResources(0, 1, skyboxTexViews);
	XMStoreFloat4x4(&skyboxConstantBufferData.view, XMMatrixTranspose(t_camView));
	XMStoreFloat4x4(&skyboxConstantBufferData.projection, XMMatrixTranspose(t_camProjection));
	XMStoreFloat4x4(&skyboxConstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(t_camera._41, t_camera._42, t_camera._43)) * XMMatrixScaling(500, 500, 500));

	

	t_devcon->UpdateSubresource(ConstantinoBiffer, 0, NULL, &skyboxConstantBufferData, 0, 0);
	UINT skyboxStride = sizeof(VertexPositionColor);
	UINT skyboxOffset = 0;

	t_devcon->IASetVertexBuffers(0, 1, &m_skyboxVertexBuffer, &skyboxStride, &skyboxOffset);
	t_devcon->IASetIndexBuffer(m_skyboxIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//t_devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//t_devcon->IASetInputLayout(dLayout);
	//t_devcon->VSSetShader(skyboxVertexShader, nullptr, 0);
	t_devcon->VSSetConstantBuffers(0, 1, &ConstantinoBiffer);
	//t_devcon->PSSetShader(skyboxPixelShader, nullptr, 0);
	t_devcon->DrawIndexed(m_skyboxIndexCount, 0, 0);
}

SkyBox::~SkyBox()
{
}
