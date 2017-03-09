#include "MainLoader.h"

//FBXLoader fbx;
namespace FBXImport
{
	FbxAMatrix GetGeometryTransformation(FbxNode * _node)
	{
		if (!_node)
			throw exception("Null for mesh geometry");

		const FbxVector4 lT = _node->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = _node->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = _node->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(lT, lR, lS);
	}

	void ProcessBones(FbxNode * _node, int _index, int _parentIndex, FBXLoader &_fbx)
	{
		if (_node->GetNodeAttribute() && _node->GetNodeAttribute()->GetAttributeType() && _node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			_fbx.joints[_index].parentIndex = _parentIndex;
			_fbx.bonesCount += 1;
		}
		for (int i = 0; i < _node->GetChildCount(); i++)
		{
			if (_fbx.bonesCount < (int)_fbx.joints.size())
				ProcessBones(_node->GetChild(i), _fbx.bonesCount, _index, _fbx);
		}
	}

	void LoadMesh(FbxMesh *_fbxMesh, FbxNode *_root, FBXLoader &_fbx)
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
			_fbx.joints.resize(boneCount);
			for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
			{
				FbxCluster* cluster = skin->GetCluster(boneIndex);
				FbxNode* bone = cluster->GetLink(); // Get a reference to the bone's node
				_fbx.joints[boneIndex].parentIndex = boneIndex - 1;
				cluster->GetTransformMatrix(transformMatrix);
				cluster->GetTransformLinkMatrix(transformLinkMatrix);
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

				XMFLOAT4X4 tmp;
				XMStoreFloat4x4(&tmp, _fbx.joints[boneIndex].world);

				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						tmp.m[y][x] = (float)globalBindposeInverseMatrix.Get(y, x);
					}
				}

				_fbx.joints[boneIndex].world = XMLoadFloat4x4(&tmp);

				// Lebo fix
				XMFLOAT4X4 _world;
				XMStoreFloat4x4(&_world, _fbx.joints[boneIndex].world);
				_world._13 *= -1;
				_world._23 *= -1;
				_world._43 *= -1;
				_world._34 *= -1;
				_world._32 *= -1;
				_world._31 *= -1;
				_world._42 *= -1;
				_fbx.joints[boneIndex].world = XMLoadFloat4x4(&_world);

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
					float boneWeight = (float)boneVertexWeights[boneVertexIndex];
					_fbx.joints[boneIndex].jointsAffected.push_back(boneVertexIndex2);
					_fbx.joints[boneIndex].jointsWeight.push_back(boneWeight);
				}

				FbxAnimStack* currAnimStack = _fbx.fbxScene->GetSrcObject<FbxAnimStack>(0);
				FbxString animStackName = currAnimStack->GetName();
				char* animationName = animStackName.Buffer();
				FbxTakeInfo* takeInfo = _fbx.fbxScene->GetTakeInfo(animStackName);
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				FbxTime animationLength = end.GetFrameCount(FbxTime::eFrames60) - start.GetFrameCount(FbxTime::eFrames60) + 1;

				FBXLoader::KeyFrame currAnim;
				for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames60); i <= end.GetFrameCount(FbxTime::eFrames60); ++i)
				{
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames60);
					FbxAMatrix currentTransformOffset = _root->EvaluateGlobalTransform(currTime) * geometryTransform;
					FbxMatrix _transform = currentTransformOffset.Inverse() * bone->EvaluateGlobalTransform(currTime) * globalBindposeInverseMatrix;

					for (int y = 0; y < 4; y++)
					{
						for (int x = 0; x < 4; x++)
						{
							currAnim.transform.r[y].m128_f32[x] = (float)(_transform.Get(y, x));
						}
					}

					float milli = (float)currTime.GetMilliSeconds() / 1000;
					currAnim.keyTime = milli;
					_fbx.joints[boneIndex].Keyframes.push_back(currAnim);
				}
			}
		}
	}

	void SaveMesh(const string _fileName, FBXLoader &_fbx, vector<FBXLoader> &_animationsfbx)
	{
		fstream file;
		file.open("../Assets/meshes/" + _fileName + ".binmesh", ios_base::binary | ios_base::out);

		if (file.is_open())
		{
			// write vertices
			unsigned int numVertices = (unsigned int)_fbx.vertices.size();
			file.write((char*)&numVertices, sizeof(numVertices));

			for (unsigned int i = 0; i < numVertices; ++i)
			{
				// vertex position
				XMFLOAT4 position = _fbx.vertices[i].position;
				file.write((char*)&position, sizeof(XMFLOAT4));

				// vertex uvs
				XMFLOAT4 uvs = _fbx.vertices[i].uv;
				file.write((char*)&uvs, sizeof(XMFLOAT4));

				// vertex normals
				XMFLOAT4 normals = _fbx.vertices[i].normal;
				file.write((char*)&normals, sizeof(XMFLOAT4));
			}

			// write indices
			unsigned int numIndices = (unsigned int)_fbx.indices.size();
			file.write((char*)&numIndices, sizeof(numIndices));

			for (unsigned int i = 0; i < numIndices; ++i)
			{
				unsigned int indice = _fbx.indices[i];
				file.write((char*)&indice, sizeof(unsigned int));
			}

			// write skeleton (joints)
			unsigned int numJoints = (unsigned int)_fbx.joints.size();
			file.write((char*)&numJoints, sizeof(unsigned int));

			for (unsigned int i = 0; i < numJoints; ++i)
			{
				// parent index for this joint
				int parentIndex = _fbx.joints[i].parentIndex;
				file.write((char*)&parentIndex, sizeof(int));

				// joint world position in mesh
				XMMATRIX jointWorld = _fbx.joints[i].world;
				file.write((char*)&jointWorld, sizeof(Mat4x4));

				// affected joints by this joint
				unsigned int numAffectedJoints = (unsigned int)_fbx.joints[i].jointsAffected.size();
				file.write((char*)&numAffectedJoints, sizeof(unsigned int));

				for (unsigned int j = 0; j < numAffectedJoints; ++j)
				{
					int affectedJoint = _fbx.joints[i].jointsAffected[j];

					file.write((char*)&affectedJoint, sizeof(int));
				}

				// joints weight
				unsigned int numJointsWeight = (unsigned int)_fbx.joints[i].jointsWeight.size();
				file.write((char*)&numJointsWeight, sizeof(unsigned int));

				for (unsigned int j = 0; j < numJointsWeight; ++j)
				{
					float jointWeight = _fbx.joints[i].jointsWeight[j];
					file.write((char*)&jointWeight, sizeof(float));
				}

				// keyframes
				unsigned int numKeyframes = (unsigned int)_fbx.joints[i].Keyframes.size();
				file.write((char*)&numKeyframes, sizeof(unsigned int));

				for (unsigned int j = 0; j < numKeyframes; ++j)
				{
					// transform position for frame
					XMMATRIX transform = _fbx.joints[i].Keyframes[j].transform;
					file.write((char*)&transform, sizeof(Mat4x4));

					// key time for the frame
					float keytime = _fbx.joints[i].Keyframes[j].keyTime;
					file.write((char*)&keytime, sizeof(float));
				}
			}

			// write num of animations
			unsigned int numAnimations = (unsigned int)_animationsfbx.size();
			file.write((char*)&numAnimations, sizeof(unsigned int));

			for (unsigned int anim = 0; anim < numAnimations; ++anim)
			{
				for (unsigned int i = 0; i < numJoints; ++i)
				{
					// parent index for this joint
					int parentIndex = _animationsfbx[anim].joints[i].parentIndex;
					file.write((char*)&parentIndex, sizeof(int));

					// joint world position in mesh
					XMMATRIX jointWorld = _animationsfbx[anim].joints[i].world;
					file.write((char*)&jointWorld, sizeof(Mat4x4));

					// affected joints by this joint
					unsigned int numAffectedJoints = (unsigned int)_animationsfbx[anim].joints[i].jointsAffected.size();
					file.write((char*)&numAffectedJoints, sizeof(unsigned int));

					for (unsigned int j = 0; j < numAffectedJoints; ++j)
					{
						int affectedJoint = _animationsfbx[anim].joints[i].jointsAffected[j];
						file.write((char*)&affectedJoint, sizeof(int));
					}

					// joints weight
					unsigned int numJointsWeight = (unsigned int)_animationsfbx[anim].joints[i].jointsWeight.size();
					file.write((char*)&numJointsWeight, sizeof(unsigned int));

					for (unsigned int j = 0; j < numJointsWeight; ++j)
					{
						float jointWeight = _animationsfbx[anim].joints[i].jointsWeight[j];
						file.write((char*)&jointWeight, sizeof(float));
					}

					// keyframes
					unsigned int numKeyframes = (unsigned int)_animationsfbx[anim].joints[i].Keyframes.size();
					file.write((char*)&numKeyframes, sizeof(unsigned int));

					for (unsigned int j = 0; j < numKeyframes; ++j)
					{
						// transform position for frame
						XMMATRIX transform = _animationsfbx[anim].joints[i].Keyframes[j].transform;
						file.write((char*)&transform, sizeof(Mat4x4));

						// key time for the frame
						float keytime = _animationsfbx[anim].joints[i].Keyframes[j].keyTime;
						file.write((char*)&keytime, sizeof(float));
					}
				}
			}

			file.close();
		}
	}

	HRESULT MainFBXLoader(const char * _filePath, FBXLoader & _fbx)
	{
		FBXLoader fbx;

		if (fbx.fbxManager == nullptr)
		{
			fbx.fbxManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(fbx.fbxManager, IOSROOT);
			fbx.fbxManager->SetIOSettings(pIOsettings);
		}

		FbxImporter* importer = FbxImporter::Create(fbx.fbxManager, "");
		fbx.fbxScene = FbxScene::Create(fbx.fbxManager, "");

		bool bSuccess = importer->Initialize(_filePath, -1, fbx.fbxManager->GetIOSettings());
		if (!bSuccess) return E_FAIL;

		bSuccess = importer->Import(fbx.fbxScene);
		if (!bSuccess) return E_FAIL;

		importer->Destroy();

		FbxNode* pFbxRootNode = fbx.fbxScene->GetRootNode();

		if (pFbxRootNode)
		{
			for (int i = 0; i < pFbxRootNode->GetChildCount(); i++)
			{
				FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
				if (pFbxChildNode->GetNodeAttribute() == NULL)
					continue;

				FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

				//if (AttributeType == FbxNodeAttribute::eSkeleton)
				//	ProcessBones(pFbxChildNode, 0, -1, fbx);

				if (AttributeType != FbxNodeAttribute::eMesh)
					continue;

				FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
				LoadMesh(pMesh, pFbxChildNode, fbx);
				FbxVector4* vertices = pMesh->GetControlPoints();
				bool unmapped;

				unsigned int indexCount = 0;
				for (int j = 0; j < pMesh->GetPolygonCount(); j++)
				{
					int polygonSize = pMesh->GetPolygonSize(j);
					assert(polygonSize == 3);

					for (int k = 0; k < polygonSize; k++)
					{
						int iControlPointIndex = pMesh->GetPolygonVertex(j, k);

						VertexDll vertex;
						vertex.position.x = (float)vertices[iControlPointIndex].mData[0];
						vertex.position.y = (float)vertices[iControlPointIndex].mData[1];
						vertex.position.z = (float)vertices[iControlPointIndex].mData[2];

						FbxVector2 uvCoords;
						FbxStringList uvStringList;
						pMesh->GetUVSetNames(uvStringList);
						pMesh->GetPolygonVertexUV(j, k, uvStringList.GetStringAt(0), uvCoords, unmapped);
						vertex.uv.x = static_cast<float>(uvCoords[0]);
						vertex.uv.y = 1 - static_cast<float>(uvCoords[1]);
						vertex.uv.w = (float)iControlPointIndex;

						FbxVector4 fbxNormal;
						bool result = pMesh->GetPolygonVertexNormal(j, k, fbxNormal);
						vertex.normal.x = (float)fbxNormal.mData[0];
						vertex.normal.y = (float)fbxNormal.mData[1];
						vertex.normal.z = -(float)fbxNormal.mData[2];

						fbx.vertices.push_back(vertex);
						fbx.indices.push_back(indexCount);
						++indexCount;
					}
					if (indexCount)
					{
						unsigned int temp = fbx.indices[indexCount - 1];
						fbx.indices[indexCount - 1] = fbx.indices[indexCount - 2];
						fbx.indices[indexCount - 2] = temp;
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

		_fbx = fbx;
		return S_OK;
	}

	void FBXImport::GenerateJoints()
	{
	}
}

FBXLoader::FBXLoader()
{
	fbxScene = nullptr;
	fbxManager = nullptr;
	fbxAnimEvaluator = nullptr;
	indexBuffer = nullptr;
	bonesCount = 0;
}

FBXLoader::~FBXLoader()
{
}