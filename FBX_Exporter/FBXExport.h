#pragma once

#include "MathFunctions.h"
#include <fstream>
#include <sstream>
#include <iomanip>



#ifdef FBXEXPORT_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif // FBXEXPORT_EXPORTS


class FBXExport
{
public:
	FBXExport();

	bool Initialize();
	bool LoadScene(const char* inFileName, const char* inOutputPath);
	bool LoadScene(const char* inFileName);
	void ExportFBX();
	FbxScene* getScene() { return mFBXScene; }
	Skeleton getSkeleton() { return mSkeleton; }
	void InitFBX();


private:

	FbxManager* mFBXManager;
	FbxScene* mFBXScene;
	std::string mInputFilePath;
	std::string mOutputFilePath;
	bool mHasAnimation;
	std::unordered_map<unsigned int, CtrlPoint*> mControlPoints;
	unsigned int mTriangleCount;
	std::vector<Triangle> mTriangles;
	std::vector<PNTIWVertex> mVertices;
	Skeleton mSkeleton;
	std::unordered_map<unsigned int, Material*> mMaterialLookUp;
	FbxLongLong mAnimationLength;
	std::string mAnimationName;
	LARGE_INTEGER mCPUFreq;

public:
	void ProcessControlPoints(FbxNode* inNode);
	void ProcessMesh(FbxNode* inNode);
	std::vector<PNTIWVertex> getverts() { return mVertices; }
	void ProcessSkeletonHierarchy(FbxNode* inRootNode);
	FbxLongLong getAnimationLength() { return mAnimationLength; };
private:
	void ProcessGeometry(FbxNode* inNode);
	void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);
	void ProcessJointsAndAnimations(FbxNode* inNode);
	unsigned int FindJointIndexUsingName(const std::string& inJointName);
	void ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, DirectX::XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outNormal);
	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outBinormal);
	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outTangent);
	void Optimize();
	int FindVertex(const PNTIWVertex& inTargetVertex, const std::vector<PNTIWVertex>& uniqueVertices);

	void AssociateMaterialToMesh(FbxNode* inNode);
	void ProcessMaterials(FbxNode* inNode);
	void ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, unsigned int inMaterialIndex);
	void ProcessMaterialTexture(FbxSurfaceMaterial* inMaterial, Material* ioMaterial);
	void PrintMaterial();
	void PrintTriangles();

	void CleanupFbxManager();
	void WriteMeshToStream(std::ostream& inStream);
	void WriteAnimationToStream(std::ostream& inStream);
};

