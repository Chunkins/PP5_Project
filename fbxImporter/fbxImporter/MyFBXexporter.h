#pragma once
#include <vector>
#include <fbxsdk.h>
#include <DirectXMath.h>
using namespace std;
using namespace DirectX;

#ifdef FBX_EXPORTS
#define FBX_API __declspec(dllexport)
#else
#define FBX_API __declspec(dllimport)
#endif


namespace FBXexport
{
	
	FbxManager* g_pFbxSdkManager;
	FBX_API  void SetFBXvalues(vector<XMFLOAT4>* verts, vector<XMFLOAT4>* uv, vector<XMFLOAT4>* normal, const char * _filePath);

}