#include "fbxImporter.h"
#include "MainLoader.h"
#include "stdafx.h"
#include <set>

namespace FBXImport
{
	HRESULT FBXImport::LoadFBX(const char * _filePath, vector<string> _animations)
	{
		FBXLoader fbx;
		vector<FBXLoader> animationsfbx;
		animationsfbx.resize(_animations.size());

		MainFBXLoader(_filePath, fbx);
		for (int i = 0; i < (int)animationsfbx.size(); ++i)
			MainFBXLoader(_animations[i].c_str(), animationsfbx[i]);

		vector<string> fileNameVec = GetFileName(_filePath);
		const char * fileName = fileNameVec[fileNameVec.size() - 2].c_str();
		SaveMesh(fileName, fbx, animationsfbx);

		return S_OK;
	}
	vector<string> GetFileName(const char * _filePath)
	{
		vector<string> result;
		set<char> delimiters{ '/', '.' };
		char const* pch = _filePath;
		char const* start = _filePath;

		for (; *pch; ++pch)
		{
			if (delimiters.find(*pch) != delimiters.end())
			{
				if (start != pch)
				{
					string str(start, pch);
					result.push_back(str);
				}
				start = pch + 1;
			}
		}
		result.push_back(start);
		return result;
	}
}
