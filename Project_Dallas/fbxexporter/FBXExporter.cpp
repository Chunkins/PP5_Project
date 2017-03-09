#include "FBXExporter.h"
#define DLL_EXPORT

FBXExporter::FBXExporter()
{
	fbxScene = nullptr;
	fbxManager = nullptr;
	fbxAnimEvaluator = nullptr;
	indices = 0;
	indexBuffer = nullptr;
	bonesCount = 0;
}

FBXExporter::~FBXExporter()
{
}

HRESULT FBXExporter::LoadFBX(const char * _filePath, vector<Vertex>& _outVertices, std::vector<unsigned int>& _outIndices)
{
	if (fbxManager == nullptr)
	{
		fbxManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(pIOsettings);
	}

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	fbxScene = FbxScene::Create(fbxManager, "");

	bool bSuccess = importer->Initialize(_filePath, -1, fbxManager->GetIOSettings());
	if (!bSuccess) return E_FAIL;

	bSuccess = importer->Import(fbxScene);
	if (!bSuccess) return E_FAIL;

	importer->Destroy();

	FbxNode* pFbxRootNode = fbxScene->GetRootNode();

	if (pFbxRootNode)
	{
		for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
		{
			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
			if (pFbxChildNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType == FbxNodeAttribute::eSkeleton)
			{
				ProcessBones(pFbxChildNode, 0, -1);
				int x = bones.size();
			}

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
			LoadMesh(pMesh, pFbxChildNode);
			int x = bones.size();

			FbxVector4* vertices = pMesh->GetControlPoints();
			//indices = pMesh->GetPolygonVertexCount();
			//indexBuffer = pMesh->GetPolygonVertices();
			bool unmapped;

			unsigned int indexCount = 0;
			for (int j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				int polygonSize = pMesh->GetPolygonSize(j);
				assert(polygonSize == 3);

				for (int k = 0; k < polygonSize; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex(j, k);

					Vertex vertex;
					vertex.position.x = (float)vertices[iControlPointIndex].mData[0];
					vertex.position.y = (float)vertices[iControlPointIndex].mData[1];
					vertex.position.z = (float)vertices[iControlPointIndex].mData[2];

					FbxVector2 uvCoords;
					FbxStringList uvStringList;
					pMesh->GetUVSetNames(uvStringList);
					pMesh->GetPolygonVertexUV(j, k, uvStringList.GetStringAt(0), uvCoords, unmapped);
					vertex.uv.x = static_cast<float>(uvCoords[0]);
					vertex.uv.y = 1 - static_cast<float>(uvCoords[1]);

					FbxVector4 fbxNormal;
					bool result = pMesh->GetPolygonVertexNormal(j, k, fbxNormal);
					vertex.normal.x = (float)fbxNormal.mData[0];
					vertex.normal.y = (float)fbxNormal.mData[1];
					vertex.normal.z = (float)fbxNormal.mData[2];

					_outVertices.push_back(vertex);
					_outIndices.push_back(indexCount);
					++indexCount;
				}
			}

			int objects = pFbxChildNode->GetSrcObjectCount<FbxSurfaceMaterial>();
			for (int c = 0; c < objects; c++)
			{
				FbxSurfaceMaterial *surfaceMaterial = (FbxSurfaceMaterial*)pFbxChildNode->GetSrcObject<FbxSurfaceMaterial>(c);
				if (surfaceMaterial)
				{
					FbxProperty property = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

					int texture_count = property.GetSrcObjectCount<FbxTexture>();
					for (int j = 0; j < texture_count; j++)
					{
						FbxTexture* texture = FbxCast<FbxTexture>(property.GetSrcObject<FbxFileTexture>(j));
						const char* fileTexture = FbxCast<FbxFileTexture>(texture)->GetFileName();
					}
				}
			}
		}
	}

	return S_OK;
}

FbxAMatrix FBXExporter::GetGeometryTransformation(FbxNode * _node)
{
	if (!_node)
		throw exception("Null for mesh geometry");

	const FbxVector4 lT = _node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = _node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = _node->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

void FBXExporter::ProcessBones(FbxNode * _node, int _index, int _parentIndex)
{
	if (_node->GetNodeAttribute() && _node->GetNodeAttribute()->GetAttributeType() && _node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		bones[_index].parentIndex = _parentIndex;
		bonesCount += 1;
	}
	for (int i = 0; i < _node->GetChildCount(); i++)
	{
		if (bonesCount < bones.size())
			ProcessBones(_node->GetChild(i), bonesCount, _index);
	}
}

void FBXExporter::LoadMesh(FbxMesh *_fbxMesh, FbxNode *_root)
{
	int numDeformers = _fbxMesh->GetDeformerCount();
	FbxSkin* skin = (FbxSkin*)_fbxMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxAMatrix transformMatrix;
	FbxAMatrix transformLinkMatrix;
	FbxAMatrix globalBindposeInverseMatrix;
	FbxAMatrix geometryTransform = GetGeometryTransformation(_root);

	if (skin != nullptr)
	{
		int boneCount = skin->GetClusterCount();
		bones.resize(boneCount);
		for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
		{
			FbxCluster* cluster = skin->GetCluster(boneIndex);
			FbxNode* bone = cluster->GetLink(); // Get a reference to the bone's node
			bones[boneIndex].parentIndex = boneIndex - 1;
			cluster->GetTransformMatrix(transformMatrix);
			cluster->GetTransformLinkMatrix(transformLinkMatrix);
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

			for (int y = 0; y < 4; y++)
				for (int x = 0; x < 4; x++)
					bones[boneIndex].joint.ee[y][x] = globalBindposeInverseMatrix.Get(y, x);

			bones[boneIndex].joint.ee[3][0] *= -1;
			bones[boneIndex].joint.ee[3][1] *= -1;
			bones[boneIndex].joint.ee[3][2] *= -1;

			// Get the bind pose
			FbxAMatrix bindPoseMatrix;
			cluster->GetTransformLinkMatrix(bindPoseMatrix);

			int *boneVertexIndices = cluster->GetControlPointIndices();
			double *boneVertexWeights = cluster->GetControlPointWeights();

			// Iterate through all the vertices, which are affected by the bone
			int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
			for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
			{
				int boneVertexIndex2 = boneVertexIndices[boneVertexIndex];
				bones[boneIndex].boneAffectedVertIndices.push_back(boneVertexIndex2);
				float boneWeight = (float)boneVertexWeights[boneVertexIndex];
				bones[boneIndex].boneVertWeights.push_back(boneWeight);
			}

			FbxAnimStack* currAnimStack = fbxScene->GetSrcObject<FbxAnimStack>(0);
			FbxString animStackName = currAnimStack->GetName();
			char* animationName = animStackName.Buffer();
			FbxTakeInfo* takeInfo = fbxScene->GetTakeInfo(animStackName);
			FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
			FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
			FbxTime animationLength = end.GetFrameCount(FbxTime::eFrames60) - start.GetFrameCount(FbxTime::eFrames60) + 1;

			KeyFrame currAnim;
			for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames60); i <= end.GetFrameCount(FbxTime::eFrames60); ++i)
			{
				FbxTime currTime;
				currTime.SetFrame(i, FbxTime::eFrames60);
				FbxAMatrix currentTransformOffset = _root->EvaluateGlobalTransform(currTime) * geometryTransform;
				currAnim.transform = currentTransformOffset.Inverse() * bone->EvaluateGlobalTransform(currTime) * globalBindposeInverseMatrix;
				currAnim.keyTime = currTime;

				bones[boneIndex].Keyframes.push_back(currAnim);
			}
		}
	}
}