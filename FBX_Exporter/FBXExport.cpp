#include "FBXExport.h"


FBXExport::FBXExport()
{
	mFBXManager = nullptr;
	mFBXScene = nullptr;
	mTriangleCount = 0;
	mHasAnimation = true;
	QueryPerformanceFrequency(&mCPUFreq);
}

bool FBXExport::Initialize()
{
	mFBXManager = FbxManager::Create();
	if (!mFBXManager)
	{
		return false;
	}

	FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(mFBXManager, IOSROOT);
	mFBXManager->SetIOSettings(fbxIOSettings);

	mFBXScene = FbxScene::Create(mFBXManager, "myScene");

	return true;
}


bool FBXExport::LoadScene(const char* inFileName, const char* inOutputPath)
{
	mInputFilePath = inFileName;
	mOutputFilePath = inOutputPath;

	FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "myImporter");

	if (!fbxImporter)	
		return false;
	

	if (!fbxImporter->Initialize(inFileName, -1, mFBXManager->GetIOSettings()))
		return false;
	

	if (!fbxImporter->Import(mFBXScene))
		return false;
	
	fbxImporter->Destroy();

	return true;
}
bool FBXExport::LoadScene(const char* inFileName)
{
	mInputFilePath = inFileName;

	FbxImporter* fbxImporter = FbxImporter::Create(mFBXManager, "myImporter");

	if (!fbxImporter)
		return false;

	if (!fbxImporter->Initialize(inFileName, -1, mFBXManager->GetIOSettings()))
		return false;

	if (!fbxImporter->Import(mFBXScene))
		return false;
	fbxImporter->Destroy();

	return true;
}

void FBXExport::InitFBX()
{
	ProcessSkeletonHierarchy(mFBXScene->GetRootNode());
	if (mSkeleton.mJoints.empty())
	{
		mHasAnimation = false;
	}
	ProcessGeometry(mFBXScene->GetRootNode());
}


void FBXExport::ExportFBX()
{
	std::string genericFileName = Utilities::GetFileName(mInputFilePath);
	genericFileName = Utilities::RemoveSuffix(genericFileName);

	ProcessSkeletonHierarchy(mFBXScene->GetRootNode());
	if (mSkeleton.mJoints.empty())
	{
		mHasAnimation = false;
	}

	ProcessGeometry(mFBXScene->GetRootNode());

	Optimize();

	std::string outputMeshName = mOutputFilePath + genericFileName + ".itpmesh";
	std::ofstream meshOutput(outputMeshName);
	WriteMeshToStream(meshOutput);

	if (mHasAnimation)
	{
		std::string outputNnimName = mOutputFilePath + genericFileName + ".itpanim";
		std::ofstream animOutput(outputNnimName);
		WriteAnimationToStream(animOutput);
	}
	std::cout << "\n\nExport Done!\n";
}

void FBXExport::ProcessGeometry(FbxNode* inNode)
{
	if (inNode->GetNodeAttribute())
	{
		switch (inNode->GetNodeAttribute()->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			ProcessControlPoints(inNode);
			if (mHasAnimation)
			{
				ProcessJointsAndAnimations(inNode);
			}
			ProcessMesh(inNode);
			AssociateMaterialToMesh(inNode);
			ProcessMaterials(inNode);
			break;
		}
	}

	for (int i = 0; i < inNode->GetChildCount(); ++i)
	{
		ProcessGeometry(inNode->GetChild(i));
	}
}




void FBXExport::ProcessSkeletonHierarchy(FbxNode* inRootNode)
{

	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = inRootNode->GetChild(childIndex);
		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	}
}

void FBXExport::ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex)
{
	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint currJoint;
		currJoint.mParentIndex = inParentIndex;
		currJoint.mMyIndex = myIndex;
		currJoint.mName = inNode->GetName();
		mSkeleton.mJoints.push_back(currJoint);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton.mJoints.size(), myIndex);
	}
}

void FBXExport::ProcessControlPoints(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();

	/////// Insert Stuff
	if (currMesh == nullptr)
	{
		inNode->GetChildCount(false);
		currMesh = inNode->GetChild(0)->GetMesh();
	}


	unsigned int ctrlPointCount = currMesh->GetControlPointsCount();
	for (unsigned int i = 0; i < ctrlPointCount; ++i)
	{
		CtrlPoint* currCtrlPoint = new CtrlPoint();
		DirectX::XMFLOAT3 currPosition;
		currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
		currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
		currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
		currCtrlPoint->mPosition = currPosition;
		mControlPoints[i] = currCtrlPoint;
	}
}

void FBXExport::ProcessJointsAndAnimations(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();
	unsigned int numOfDeformers = currMesh->GetDeformerCount();
	FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(inNode);

	for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	{
		FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (!currSkin)
		{
			continue;
		}

		unsigned int numOfClusters = currSkin->GetClusterCount();
		for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
		{
			FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
			std::string currJointName = currCluster->GetLink()->GetName();
			unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;

			currCluster->GetTransformMatrix(transformMatrix);	
			currCluster->GetTransformLinkMatrix(transformLinkMatrix);	
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			mSkeleton.mJoints[currJointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;
			mSkeleton.mJoints[currJointIndex].mNode = currCluster->GetLink();

			unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
			for (unsigned int i = 0; i < numOfIndices; ++i)
			{
				BlendingIndexWeightPair currBlendingIndexWeightPair;
				currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
				currBlendingIndexWeightPair.mBlendingWeight = currCluster->GetControlPointWeights()[i];
				mControlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
			}

			FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			mAnimationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
			Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;

			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
			{
				float keyFrameTime = 0;
				FbxTime currTime;//
				currTime.SetFrame(i, FbxTime::eFrames24);//
				*currAnim = new Keyframe();//
				(*currAnim)->mFrameNum = i;
				FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;//
				(*currAnim)->mGlobalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
				(*currAnim)->keyFrameTime = (float)currTime.GetSecondDouble();
				currAnim = &((*currAnim)->mNext);
			}
		}
	}
}

unsigned int FBXExport::FindJointIndexUsingName(const std::string& inJointName)
{
	for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		if (mSkeleton.mJoints[i].mName == inJointName)
		{
			return i;
		}
	}

	throw std::exception("Skeleton information in FBX file is corrupted.");
}


void FBXExport::ProcessMesh(FbxNode* inNode)
{
	FbxMesh* currMesh = inNode->GetMesh();

	if (currMesh == nullptr)
	{
		int stuff = 0;
		stuff = inNode->GetChildCount(false);
		currMesh = inNode->GetChild(0)->GetMesh();
	}




	mTriangleCount = currMesh->GetPolygonCount();
	int vertexCounter = 0;
	mTriangles.reserve(mTriangleCount);

	for (unsigned int i = 0; i < mTriangleCount; ++i)
	{
		DirectX::XMFLOAT3 normal[3];
		DirectX::XMFLOAT3 tangent[3];
		DirectX::XMFLOAT3 binormal[3];
		DirectX::XMFLOAT2 UV[3][2];
		Triangle currTriangle;
		mTriangles.push_back(currTriangle);

		for (unsigned int j = 0; j < 3; ++j)
		{
			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
			CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex];


			ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]);
			for (int k = 0; k < 1; ++k)
			{
				ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]);
			}


			PNTIWVertex temp;
			temp.mPosition = currCtrlPoint->mPosition;
			temp.mNormal = normal[j];
			temp.mUV = UV[j][0];
			for (unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
			{
				VertexBlendingInfo currBlendingInfo;
				currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
				currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight;
				temp.mVertexBlendingInfos.push_back(currBlendingInfo);
			}
			temp.SortBlendingInfoByWeight();

			mVertices.push_back(temp);
			mTriangles.back().mIndices.push_back(vertexCounter);
			++vertexCounter;
		}
	}

	for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	{
		delete itr->second;
	}
	mControlPoints.clear();
}

void FBXExport::ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, DirectX::XMFLOAT2& outUV)
{
	if (inUVLayer >= 2 || inMesh->GetElementUVCount() <= inUVLayer)
	{
		throw std::exception("Invalid UV Layer Number");
	}
	FbxGeometryElementUV* vertexUV = inMesh->GetElementUV(inUVLayer);

	switch (vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexUV->GetIndexArray().GetAt(inCtrlPointIndex);
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(index).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			outUV.x = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[0]);
			outUV.y = static_cast<float>(vertexUV->GetDirectArray().GetAt(inTextureUVIndex).mData[1]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExport::ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outNormal)
{
	if (inMesh->GetElementNormalCount() < 1)
	{
		throw std::exception("Invalid Normal Number");
	}

	FbxGeometryElementNormal* vertexNormal = inMesh->GetElementNormal(0);
	switch (vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexNormal->GetIndexArray().GetAt(inVertexCounter);
			outNormal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
			outNormal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
			outNormal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExport::ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outBinormal)
{
	if (inMesh->GetElementBinormalCount() < 1)
	{
		throw std::exception("Invalid Binormal Number");
	}

	FbxGeometryElementBinormal* vertexBinormal = inMesh->GetElementBinormal(0);
	switch (vertexBinormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inCtrlPointIndex);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexBinormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexBinormal->GetIndexArray().GetAt(inVertexCounter);
			outBinormal.x = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[0]);
			outBinormal.y = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[1]);
			outBinormal.z = static_cast<float>(vertexBinormal->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExport::ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, DirectX::XMFLOAT3& outTangent)
{
	if (inMesh->GetElementTangentCount() < 1)
	{
		throw std::exception("Invalid Tangent Number");
	}

	FbxGeometryElementTangent* vertexTangent = inMesh->GetElementTangent(0);
	switch (vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;

	case FbxGeometryElement::eByPolygonVertex:
		switch (vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
			outTangent.x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
			outTangent.y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
			outTangent.z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
		}
		break;

		default:
			throw std::exception("Invalid Reference");
		}
		break;
	}
}

void FBXExport::Optimize()
{
	std::vector<PNTIWVertex> uniqueVertices;
	for (unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			if (FindVertex(mVertices[i * 3 + j], uniqueVertices) == -1)
			{
				uniqueVertices.push_back(mVertices[i * 3 + j]);
			}
		}
	}

	for (unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			mTriangles[i].mIndices[j] = FindVertex(mVertices[i * 3 + j], uniqueVertices);
		}
	}

	mVertices.clear();
	mVertices = uniqueVertices;
	uniqueVertices.clear();

	std::sort(mTriangles.begin(), mTriangles.end());
}

int FBXExport::FindVertex(const PNTIWVertex& inTargetVertex, const std::vector<PNTIWVertex>& uniqueVertices)
{
	for (unsigned int i = 0; i < uniqueVertices.size(); ++i)
	{
		if (inTargetVertex == uniqueVertices[i])
		{
			return i;
		}
	}

	return -1;
}

void FBXExport::AssociateMaterialToMesh(FbxNode* inNode)
{
	FbxLayerElementArrayTemplate<int>* materialIndices;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	FbxMesh* currMesh = inNode->GetMesh();

	if (currMesh->GetElementMaterial())
	{
		materialIndices = &(currMesh->GetElementMaterial()->GetIndexArray());
		materialMappingMode = currMesh->GetElementMaterial()->GetMappingMode();

		if (materialIndices)
		{
			switch (materialMappingMode)
			{
			case FbxGeometryElement::eByPolygon:
			{
				if (materialIndices->GetCount() == mTriangleCount)
				{
					for (unsigned int i = 0; i < mTriangleCount; ++i)
					{
						unsigned int materialIndex = materialIndices->GetAt(i);
						mTriangles[i].mMaterialIndex = materialIndex;
					}
				}
			}
			break;

			case FbxGeometryElement::eAllSame:
			{
				unsigned int materialIndex = materialIndices->GetAt(0);
				for (unsigned int i = 0; i < mTriangleCount; ++i)
				{
					mTriangles[i].mMaterialIndex = materialIndex;
				}
			}
			break;

			default:
				throw std::exception("Invalid mapping mode for material\n");
			}
		}
	}
}

void FBXExport::ProcessMaterials(FbxNode* inNode)
{
	unsigned int materialCount = inNode->GetMaterialCount();

	for (unsigned int i = 0; i < materialCount; ++i)
	{
		FbxSurfaceMaterial* surfaceMaterial = inNode->GetMaterial(i);
		ProcessMaterialAttribute(surfaceMaterial, i);
		ProcessMaterialTexture(surfaceMaterial, mMaterialLookUp[i]);
	}
}

void FBXExport::ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, unsigned int inMaterialIndex)
{
	FbxDouble3 double3;
	FbxDouble double1;
	if (inMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		PhongMaterial* currMaterial = new PhongMaterial();

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Specular Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Specular;
		currMaterial->mSpecular.x = static_cast<float>(double3.mData[0]);
		currMaterial->mSpecular.y = static_cast<float>(double3.mData[1]);
		currMaterial->mSpecular.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Reflection
		double3 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Reflection;
		currMaterial->mReflection.x = static_cast<float>(double3.mData[0]);
		currMaterial->mReflection.y = static_cast<float>(double3.mData[1]);
		currMaterial->mReflection.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;

		// Shininess
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->Shininess;
		currMaterial->mShininess = double1;

		// Specular Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->SpecularFactor;
		currMaterial->mSpecularPower = double1;


		// Reflection Factor
		double1 = reinterpret_cast<FbxSurfacePhong *>(inMaterial)->ReflectionFactor;
		currMaterial->mReflectionFactor = double1;

		mMaterialLookUp[inMaterialIndex] = currMaterial;
	}
	else if (inMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		LambertMaterial* currMaterial = new LambertMaterial();

		// Amibent Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Ambient;
		currMaterial->mAmbient.x = static_cast<float>(double3.mData[0]);
		currMaterial->mAmbient.y = static_cast<float>(double3.mData[1]);
		currMaterial->mAmbient.z = static_cast<float>(double3.mData[2]);

		// Diffuse Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Diffuse;
		currMaterial->mDiffuse.x = static_cast<float>(double3.mData[0]);
		currMaterial->mDiffuse.y = static_cast<float>(double3.mData[1]);
		currMaterial->mDiffuse.z = static_cast<float>(double3.mData[2]);

		// Emissive Color
		double3 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->Emissive;
		currMaterial->mEmissive.x = static_cast<float>(double3.mData[0]);
		currMaterial->mEmissive.y = static_cast<float>(double3.mData[1]);
		currMaterial->mEmissive.z = static_cast<float>(double3.mData[2]);

		// Transparency Factor
		double1 = reinterpret_cast<FbxSurfaceLambert *>(inMaterial)->TransparencyFactor;
		currMaterial->mTransparencyFactor = double1;

		mMaterialLookUp[inMaterialIndex] = currMaterial;
	}
}

void FBXExport::ProcessMaterialTexture(FbxSurfaceMaterial* inMaterial, Material* ioMaterial)
{
	unsigned int textureIndex = 0;
	FbxProperty property;

	FBXSDK_FOR_EACH_TEXTURE(textureIndex)
	{
		property = inMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
		if (property.IsValid())
		{
			unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
			for (unsigned int i = 0; i < textureCount; ++i)
			{
				FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
				if (layeredTexture)
				{
					throw std::exception("Layered Texture is currently unsupported\n");
				}
				else
				{
					FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
					if (texture)
					{
						std::string textureType = property.GetNameAsCStr();
						FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

						if (fileTexture)
						{
							if (textureType == "DiffuseColor")
							{
								ioMaterial->mDiffuseMapName = fileTexture->GetFileName();
							}
							else if (textureType == "SpecularColor")
							{
								ioMaterial->mSpecularMapName = fileTexture->GetFileName();
							}
							else if (textureType == "Bump")
							{
								ioMaterial->mNormalMapName = fileTexture->GetFileName();
							}
						}
					}
				}
			}
		}
	}
}

void FBXExport::PrintMaterial()
{
	for (auto itr = mMaterialLookUp.begin(); itr != mMaterialLookUp.end(); ++itr)
	{
		itr->second->WriteToStream(std::cout);
		std::cout << "\n\n";
	}
}

void FBXExport::PrintTriangles()
{
	for (unsigned int i = 0; i < mTriangles.size(); ++i)
	{
		std::cout << "Triangle# " << i + 1 << " Material Index: " << mTriangles[i].mMaterialIndex << "\n";
	}
}

void FBXExport::CleanupFbxManager()
{
	mFBXScene->Destroy();
	mFBXManager->Destroy();

	mTriangles.clear();

	mVertices.clear();

	mSkeleton.mJoints.clear();

	for (auto itr = mMaterialLookUp.begin(); itr != mMaterialLookUp.end(); ++itr)
	{
		delete itr->second;
	}
	mMaterialLookUp.clear();
}

void FBXExport::WriteMeshToStream(std::ostream& inStream)
{

	inStream << "<?xml version='1.0' encoding='UTF-8' ?>" << std::endl;
	inStream << "<itpmesh>" << std::endl;
	if (mHasAnimation)
	{
		inStream << "\t<!-- position, normal, skinning weights, skinning indices, texture-->" << std::endl;
		inStream << "\t<format>pnst</format>" << std::endl;
	}
	else
	{
		inStream << "\t<format>pnt</format>" << std::endl;
	}
	inStream << "\t<texture>" << mMaterialLookUp[0]->mDiffuseMapName << "</texture>" << std::endl;
	inStream << "\t<triangles count='" << mTriangleCount << "'>" << std::endl;

	for (unsigned int i = 0; i < mTriangleCount; ++i)
	{
		// We need to change the culling order
		inStream << "\t\t<tri>" << mTriangles[i].mIndices[0] << "," << mTriangles[i].mIndices[2] << "," << mTriangles[i].mIndices[1] << "</tri>" << std::endl;
	}
	inStream << "\t</triangles>" << std::endl;


	inStream << "\t<vertices count='" << mVertices.size() << "'>" << std::endl;
	for (unsigned int i = 0; i < mVertices.size(); ++i)
	{
		inStream << "\t\t<vtx>" << std::endl;
		inStream << "\t\t\t<pos>" << mVertices[i].mPosition.x << "," << mVertices[i].mPosition.y << "," << -mVertices[i].mPosition.z << "</pos>" << std::endl;
		inStream << "\t\t\t<norm>" << mVertices[i].mNormal.x << "," << mVertices[i].mNormal.y << "," << -mVertices[i].mNormal.z << "</norm>" << std::endl;
		if (mHasAnimation)
		{
			inStream << "\t\t\t<sw>" << static_cast<float>(mVertices[i].mVertexBlendingInfos[0].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[1].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[2].mBlendingWeight) << "," << static_cast<float>(mVertices[i].mVertexBlendingInfos[3].mBlendingWeight) << "</sw>" << std::endl;
			inStream << "\t\t\t<si>" << mVertices[i].mVertexBlendingInfos[0].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[1].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[2].mBlendingIndex << "," << mVertices[i].mVertexBlendingInfos[3].mBlendingIndex << "</si>" << std::endl;
		}
		inStream << "\t\t\t<tex>" << mVertices[i].mUV.x << "," << 1.0f - mVertices[i].mUV.y << "</tex>" << std::endl;
		inStream << "\t\t</vtx>" << std::endl;
	}

	inStream << "\t</vertices>" << std::endl;
	inStream << "</itpmesh>" << std::endl;
}

void FBXExport::WriteAnimationToStream(std::ostream& inStream)
{
	inStream << "<?xml version='1.0' encoding='UTF-8' ?>" << std::endl;
	inStream << "<itpanim>" << std::endl;
	inStream << "\t<skeleton count='" << mSkeleton.mJoints.size() << "'>" << std::endl;
	for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		inStream << "\t\t<joint id='" << i << "' name='" << mSkeleton.mJoints[i].mName << "' parent='" << mSkeleton.mJoints[i].mParentIndex << "'>\n";
		inStream << "\t\t\t";
		FbxVector4 translation = mSkeleton.mJoints[i].mGlobalBindposeInverse.GetT();
		FbxVector4 rotation = mSkeleton.mJoints[i].mGlobalBindposeInverse.GetR();
		translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
		rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
		mSkeleton.mJoints[i].mGlobalBindposeInverse.SetT(translation);
		mSkeleton.mJoints[i].mGlobalBindposeInverse.SetR(rotation);
		FbxMatrix out = mSkeleton.mJoints[i].mGlobalBindposeInverse;

		Utilities::WriteMatrix(inStream, out.Transpose(), true);
		inStream << "\t\t</joint>\n";
	}
	inStream << "\t</skeleton>\n";
	inStream << "\t<animations>\n";
	inStream << "\t\t<animation name='" << mAnimationName << "' length='" << mAnimationLength << "'>\n";
	for (unsigned int i = 0; i < mSkeleton.mJoints.size(); ++i)
	{
		inStream << "\t\t\t" << "<track id = '" << i << "' name='" << mSkeleton.mJoints[i].mName << "'>\n";
		Keyframe* walker = mSkeleton.mJoints[i].mAnimation;
		while (walker)
		{
			inStream << "\t\t\t\t" << "<frame num='" << walker->mFrameNum - 1 << "'>\n";
			inStream << "\t\t\t\t\t";
			FbxVector4 translation = walker->mGlobalTransform.GetT();
			FbxVector4 rotation = walker->mGlobalTransform.GetR();
			translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
			rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
			walker->mGlobalTransform.SetT(translation);
			walker->mGlobalTransform.SetR(rotation);
			FbxMatrix out = walker->mGlobalTransform;
			Utilities::WriteMatrix(inStream, out.Transpose(), true);
			inStream << "\t\t\t\t" << "</frame>\n";
			walker = walker->mNext;
		}
		inStream << "\t\t\t" << "</track>\n";
	}
	inStream << "\t\t</animation>\n";
	inStream << "</animations>\n";
	inStream << "</itpanim>";
}
