#pragma once
#ifndef _FBX_EXPORTER_H_
#define _FBX_EXPORTER_H_

#if defined _FBX_EXPORTER_H_
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif

#include "LinearPrimitives.h"
#include "Mesh.h"
#include <fbxsdk.h>

class FBXExporter
{
private:
	struct KeyFrame
	{
		Matrix4x4 transform;
		FbxTime keyTime;
	};
	struct Bone
	{
		int parentIndex;
		Matrix4x4 joint;
		vector<int> boneAffectedVertIndices;
		vector<double> boneVertWeights;
		vector<KeyFrame> Keyframes;
		FbxTime curFrameTime;

		Bone() { parentIndex = 0; }
	};

	FbxScene			*fbxScene;
	FbxManager			*fbxManager;
	FbxAnimEvaluator	*fbxAnimEvaluator;
	int					indices;
	int					*indexBuffer;
	vector<Bone>		bones;
	int					bonesCount;

public:
	FBXExporter();
	~FBXExporter();

	HRESULT LoadFBX(const char* _filePath, vector<Vertex> &_outVertices, std::vector<unsigned int> &_outIndices);
	FbxAMatrix GetGeometryTransformation(FbxNode *_node);
	void ProcessBones(FbxNode* _node, int _index, int _parentIndex);
	void LoadMesh(FbxMesh *_fbxMesh, FbxNode* _root);
};

#endif