#include "MyFBXexporter.h"

namespace FBXexport
{
	void FBXexport::SetFBXvalues(vector<XMFLOAT4>* verts, vector<XMFLOAT4>* uv, vector<XMFLOAT4>* normal, const char * _filePath)
	{
		

		if (g_pFbxSdkManager == nullptr)
		{
			g_pFbxSdkManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
			g_pFbxSdkManager->SetIOSettings(pIOsettings);
		}

		FbxImporter* importer = FbxImporter::Create(g_pFbxSdkManager, "");
		FbxScene* pFBXscene = FbxScene::Create(g_pFbxSdkManager, "");

		bool bSuccess = importer->Initialize(_filePath, -1, g_pFbxSdkManager->GetIOSettings());
		//if (!bSuccess) return E_FAIL;


		bSuccess = importer->Import(pFBXscene);
	//	if (!bSuccess) return E_FAIL;

		importer->Destroy();

		FbxNode* pFbxRootNode = pFBXscene->GetRootNode();

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
				//LoadMesh(pMesh, pFbxChildNode, fbx);
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

						XMFLOAT4 vertex;
						XMFLOAT4 uvs;
						XMFLOAT4 normals;


						vertex.x = (float)vertices[iControlPointIndex].mData[0];
						vertex.y = (float)vertices[iControlPointIndex].mData[1];
						vertex.z = (float)vertices[iControlPointIndex].mData[2];

						verts->push_back(vertex);

						FbxVector2 uvCoords;
						FbxStringList uvStringList;
						pMesh->GetUVSetNames(uvStringList);
						pMesh->GetPolygonVertexUV(j, k, uvStringList.GetStringAt(0), uvCoords, unmapped);
						uvs.x = static_cast<float>(uvCoords[0]);
						uvs.y = 1 - static_cast<float>(uvCoords[1]);
						uvs.w = (float)iControlPointIndex;

						uv->push_back(uvs);

						FbxVector4 fbxNormal;
						bool result = pMesh->GetPolygonVertexNormal(j, k, fbxNormal);
						normals.x = (float)fbxNormal.mData[0];
						normals.y = (float)fbxNormal.mData[1];
						normals.z = -(float)fbxNormal.mData[2];

						normal->push_back(normals);
						//fbx.vertices.push_back(vertex);
						//fbx.indices.push_back(indexCount);
						++indexCount;
					}
					//if (indexCount)
					//{
					//	unsigned int temp = fbx.indices[indexCount - 1];
					//	fbx.indices[indexCount - 1] = fbx.indices[indexCount - 2];
					//	fbx.indices[indexCount - 2] = temp;
					//}
				}

				//int objects = pFbxChildNode->GetSrcObjectCount<FbxSurfaceMaterial>();
				//for (int c = 0; c < objects; c++)
				//{
				//	FbxSurfaceMaterial *surfaceMaterial = (FbxSurfaceMaterial*)pFbxChildNode->GetSrcObject<FbxSurfaceMaterial>(c);
				//	if (surfaceMaterial)
				//	{
				//		FbxProperty property = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

				//		int texture_count = property.GetSrcObjectCount<FbxTexture>();
				//		for (int j = 0; j < texture_count; j++)
				//		{
				//			FbxTexture* texture = FbxCast<FbxTexture>(property.GetSrcObject<FbxFileTexture>(j));
				//			const char* fileTexture = FbxCast<FbxFileTexture>(texture)->GetFileName();
				//		}
				//	}
				//}
			}
		}
	
	}

}