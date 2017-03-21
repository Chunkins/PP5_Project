#include "RenderNode.h"



RenderNode::RenderNode()
{
	
	m_Model = new Model;
}


RenderNode::~RenderNode()
{
	if (mWireframeRS)
		mWireframeRS->Release();
}



void RenderNode::InitRenderStateWireFrame(ID3D11Device *device)
{
	D3D11_RASTERIZER_DESC wfd;
	ZeroMemory(&wfd, sizeof(D3D11_RASTERIZER_DESC));
	wfd.FillMode = D3D11_FILL_WIREFRAME;
	wfd.CullMode = D3D11_CULL_NONE;
	wfd.DepthClipEnable = true;

	device->CreateRasterizerState(&wfd, &mWireframeRS);
}

void RenderNode::InitRenderStateFill(ID3D11Device * device)
{
	D3D11_RASTERIZER_DESC wfd;
	ZeroMemory(&wfd, sizeof(D3D11_RASTERIZER_DESC));
	wfd.FillMode = D3D11_FILL_SOLID;
	wfd.CullMode = D3D11_CULL_NONE;
	wfd.DepthClipEnable = true;

	device->CreateRasterizerState(&wfd, &mWireframeRS);
}
