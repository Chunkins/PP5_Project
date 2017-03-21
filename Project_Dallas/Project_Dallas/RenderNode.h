#pragma once
#include "Model.h"




class RenderNode
{
private:
	ID3D11RasterizerState* mWireframeRS;
	Model* m_Model;
	wchar_t   m_TextureName;
	char * m_ModelName;
	int type; // 1 = fbx    2 = obj

public:
	RenderNode();
	~RenderNode();
	void InitRenderStateWireFrame(ID3D11Device * device);
	void InitRenderStateFill(ID3D11Device * device);

	ID3D11RasterizerState* GetRS() { return mWireframeRS; }
	Model* GetModel() { return m_Model; }

	void SetMName(char *  name) { m_ModelName = name; }
	char*  GetMName() { return m_ModelName; }

	void SetTName(wchar_t  name) { m_TextureName = name; }
	wchar_t  GetTName() { return m_TextureName; }


	int GetType() { return type; }
	void SetType(int x) { type = x; }
};

