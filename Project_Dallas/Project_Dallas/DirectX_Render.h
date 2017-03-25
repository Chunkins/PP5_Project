#pragma once
#include <d3d11.h>
#include <DirectXPackedVector.h>
#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <math.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>
#include <vector>
#include <DirectXMath.h>
#include "Model.h"
#include "RenderNode.h"
//#include "DirectXHelper.h"
#include <time.h>
using namespace DirectX;
//using namespace ;


// define the screen resolution
#define SCREEN_WIDTH  800.f
#define SCREEN_HEIGHT 600.f
#define PLANE 0
#define BOX 1
#define TEDDY 2
// include the Direct3D Library file
#pragma comment (lib, "D3DCompiler.lib")
#pragma comment (lib, "d3d11.lib")
//struct VertInfo
//{
//	unsigned int numIndicies;
//	std::vector<float> blendWeight;
//	std::vector<int> boneIndices;
//
//	Float3 vert;
//	Float3 norm;
//	Float2 uv;
//
//};
//struct BoneInfo
//{
//	int index;
//	int parentIndex;
//	BoneInfo * Parent;
//	Float4x4 transform;
//
//	std::vector<BoneInfo*> children;
//	std::vector<KeyFrame*>* keyframes;
//};



class DirectX_Render
{
private:
	bool Swap = false; // Are we currently in model swapping mode
	ID3D11Buffer *pVBufferB;
	ID3D11ShaderResourceView* pSRVB;
	unsigned int indexCountB;
	bool RSswap = false;
	vector<Model> Models; //Container to store all the Models(Not in use yet)
	vector<RenderNode> NodeList; // My bettter container 
	unsigned int frame = 0u;


	RenderNode Plane, Box, Teddy;
	float prevTime, currTime;
	float fakeTime = .001f;
	unsigned int m_lightChoice = 1u;
	float	m_degreesPerSecond = 45.f;
	XMFLOAT3 directionlightDir;
	XMFLOAT3 spotLightPos = XMFLOAT3(-3.f, 1.f, -1.f);
	ID3D11Buffer* cbPerObjectBuffer;
	ID3D11Buffer* frameBufer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* parentBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11SamplerState * m_sampler ;
	float delta_time = .001f;
	float deltaTime = 0.f;
	float rot = 0.01f;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	IDXGISwapChain *swapchain;             // the pointer to the swap chain interface
	ID3D11Device *dev;                     // the pointer to our Direct3D device interface
	ID3D11DeviceContext *devcon;           // the pointer to our Direct3D device context
	ID3D11RenderTargetView *backbuffer;    // the pointer to our back buffer
	ID3D11InputLayout *pLayout;            // the pointer to the input layout
	ID3D11VertexShader *pVS;               // the pointer to the vertex shader
	ID3D11PixelShader *pPS;                // the pointer to the pixel shader
	

	DirectX::XMFLOAT4X4 m_camera;

	XMMATRIX World;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	POINT m_currMousePos;
	POINT m_prevMousePos;

	struct cbPerObject
	{
		XMMATRIX  WVP;
	};
	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding;  // Added extra padding so structure is a multiple of 16 for CreateBuffer function requirements.
		XMFLOAT4 lightType;
	};

	typedef struct D3DXCOLOR {
		FLOAT r;
		FLOAT g;
		FLOAT b;
		FLOAT a;
	} D3DXCOLOR, *LPD3DXCOLOR;
public:
	DirectX_Render();
	~DirectX_Render();
	void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
	void UpdateLights(void);
	void CleanD3D(void);        // closes Direct3D and releases memory
	void RenderFrame(void);     // renders a single frame
	void InitGraphics(void);    // creates the shape to render
	void InitPipeline(void);    // loads and prepares the shaders
	void Update(void);
	void UpdateCamera(float const moveSpd, float rotSpd);
	void SwapModel(); //Cycle Between the FBX Models
	void Reset(); //Return everything in the scene to their Defaults (camera, light, model positions, etc...)
	void ToggleWireFrame();
	


};



