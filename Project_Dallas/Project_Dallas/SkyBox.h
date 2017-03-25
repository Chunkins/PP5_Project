#pragma once
#include "Model.h"
#include <d3dcompiler.h>


class SkyBox
{
private:
	struct ModelViewProjectionConstantBuffer
	{
		XMFLOAT4X4 model;
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct VertexPositionColor
	{
		XMFLOAT3 pos;
		XMFLOAT3 color;
	};

	ID3D11Buffer* ConstantinoBiffer;
	ID3D11Buffer*		m_skyboxVertexBuffer;
	ID3D11Buffer*		m_skyboxIndexBuffer;
	int32_t										m_skyboxIndexCount;
	ID3D11VertexShader*	skyboxVertexShader;
	ID3D11PixelShader*	skyboxPixelShader;
	ID3D11Texture2D*					skyboxTexture;
	ID3D11ShaderResourceView*			skyboxModelView;
	ID3D11InputLayout* dLayout;

public:
	ModelViewProjectionConstantBuffer			skyboxConstantBufferData;

	ID3D11VertexShader * GetVShader() { return skyboxVertexShader; }
	ID3D11PixelShader * GetPShader() { return skyboxPixelShader; }
	ID3D11InputLayout * GetILayout() { return dLayout; }


	SkyBox();
	void Init(ID3D11Device * t_dev, char * filename);
	void Draw(ID3D11DeviceContext * t_devcon, XMMATRIX t_camProjection, XMMATRIX t_camView, XMFLOAT4X4 t_camera, XMMATRIX t_camworld);
	~SkyBox();
};

