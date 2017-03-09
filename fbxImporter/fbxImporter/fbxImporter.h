#pragma once
#include "stdafx.h"
#include <vector>
#include "Mesh.h"

using namespace std;

#ifdef FBX_EXPORTS
#define FBX_API __declspec(dllexport)
#else
#define FBX_API __declspec(dllimport)
#endif

namespace FBXImport
{
	FBX_API HRESULT LoadFBX(const char* _filePath, vector<string> _animations);
	FBX_API vector<string> GetFileName(const char* _filePath);
}
