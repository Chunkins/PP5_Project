#pragma once
#include "stdafx.h"
#include "LinearPrimitives.h"
#include "Mesh.h"

class FBXLoader
{
public:
	struct KeyFrame
	{
		XMMATRIX transform;
		float keyTime;

		KeyFrame()
		{
			transform = XMMatrixIdentity();
			keyTime = 0;
		}
	};
	struct Joint
	{
		int parentIndex = 0;
		XMMATRIX world;
		vector<int> jointsAffected;
		vector<float> jointsWeight;
		vector<KeyFrame> Keyframes;
	};
	struct ControlPoints
	{
		int indices[4];
		float weights[4];
	};

	FbxScene *fbxScene;
	FbxManager *fbxManager;
	FbxAnimEvaluator *fbxAnimEvaluator;
	int *indexBuffer;
	vector<Joint> joints;
	int bonesCount;
	vector<FBXImport::VertexDll> vertices;
	vector<unsigned int> indices;

public:
	FBXLoader();
	~FBXLoader();
};

namespace FBXImport
{
	FbxAMatrix GetGeometryTransformation(FbxNode *_node);
	void ProcessBones(FbxNode* _node, int _index, int _parentIndex, FBXLoader &_fbx);
	void LoadMesh(FbxMesh *_fbxMesh, FbxNode* _root, FBXLoader &_fbx);
	void SaveMesh(const string _fileName, FBXLoader &_fbx, vector<FBXLoader> &_animationsfbx);
	HRESULT MainFBXLoader(const char* _filePath, FBXLoader &_fbx);
	void GenerateJoints();
}

