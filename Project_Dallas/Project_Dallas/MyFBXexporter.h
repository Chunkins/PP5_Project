#pragma once
#include <vector>
#include <fbxsdk.h>
#include <DirectXMath.h>
using namespace std;
using namespace DirectX;

#ifdef FBXEXPORTS_EXPORTS
#define FBXEXPORTS_API __declspec(dllexport)
#else
#define FBXEXPORTS_API __declspec(dllimport)
#endif


namespace FBXexports
{
	class Functions
	{
		public:
		FbxManager* fb_pFbxSdkManager;
		FBXEXPORTS_API  void SetFBXvalues(vector<XMFLOAT4>* verts, vector<XMFLOAT4>* uv, vector<XMFLOAT4>* normal, const char * _filePath);
	};

}