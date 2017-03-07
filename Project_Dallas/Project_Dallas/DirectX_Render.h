#pragma once
#include <d3d11.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <math.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;


// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
// include the Direct3D Library file
#pragma comment (lib, "D3DCompiler.lib")
#pragma comment (lib, "d3d11.lib")
struct VERTEX {
	float x;
	float y;
	float z;
	float color[4];
};



class DirectX_Render
{
private:
	XMMATRIX cube1World;
	XMMATRIX cube2World;
	ID3D11Buffer* cbPerObjectBuffer;
	XMMATRIX Rotation;
	XMMATRIX Scale;
	XMMATRIX Translation;
	float rot = 0.01f;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11Buffer* squareIndexBuffer;
	ID3D11Buffer* squareVertBuffer;
	IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
	ID3D11Device *dev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
	ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
	ID3D11InputLayout *pLayout;            // the pointer to the input layout
	ID3D11VertexShader *pVS;               // the pointer to the vertex shader
	ID3D11PixelShader *pPS;                // the pointer to the pixel shader
	ID3D11Buffer *pVBuffer;                // the pointer to the vertex buffer

	XMMATRIX WVP;
	XMMATRIX World;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	struct cbPerObject
	{
		XMMATRIX  WVP;
	};

	cbPerObject cbPerObj;

	typedef struct D3DXCOLOR {
		FLOAT r;
		FLOAT g;
		FLOAT b;
		FLOAT a;
	} D3DXCOLOR, *LPD3DXCOLOR;
public:
	// a struct to define a single vertex
	struct VERTEX { FLOAT X, Y, Z; float Color[4]; };
	DirectX_Render();
	~DirectX_Render();
	void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
	void CleanD3D(void);        // closes Direct3D and releases memory
	void RenderFrame(void);     // renders a single frame
	void InitGraphics(void);    // creates the shape to render
	void InitPipeline(void);    // loads and prepares the shaders
	void Update(void);
	void UpdateCamera(float const moveSpd, float rotSpd);
};



