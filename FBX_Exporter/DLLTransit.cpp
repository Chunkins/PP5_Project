#include "DLLTransit.h"
#include "FBXExport.h"


void EXP::DLLTransit::saveFiletoBin(const char* inFileName, const char* binFileName)
{
	FBXExport* exporter = new FBXExport();

	exporter->Initialize();
	exporter->LoadScene(inFileName);


	FbxScene* tmpScene = exporter->getScene();
	FbxNode* tmpNode = tmpScene->GetRootNode();

	exporter->InitFBX();

	int tmpNodeCount = tmpScene->GetNodeCount();

	exporter->ProcessControlPoints(tmpNode);
	exporter->ProcessMesh(tmpNode);

	std::vector<PNTIWVertex> SendData = exporter->getverts();
	std::vector<Joint> bone = exporter->getSkeleton().mJoints;

	int vertSize = (int)exporter->getverts().size();
	
	std::ofstream outbinFile(binFileName, std::ios::binary);

	if (outbinFile.is_open())
	{
		outbinFile.write((char*)&vertSize, sizeof(unsigned int));

		for (int i = 0; i < vertSize; i++)
		{
			VertexInfo toReturn;
			toReturn.numIndicies = SendData[i].mVertexBlendingInfos.size();
			outbinFile.write((char*)&toReturn.numIndicies, sizeof(unsigned int));
			for (size_t j = 0; j < toReturn.numIndicies; j++)
			{
				toReturn.blendWeights.push_back((float)SendData[i].mVertexBlendingInfos[j].mBlendingWeight);
				toReturn.boneIndices.push_back(SendData[i].mVertexBlendingInfos[j].mBlendingIndex);
			}
			for (size_t j = 0; j < toReturn.numIndicies; j++)
			{
				outbinFile.write((char*)&toReturn.blendWeights[j], sizeof(float));
				outbinFile.write((char*)&toReturn.boneIndices[j], sizeof(int));
			}
			
			//	Float3 vert;
			toReturn.pos.x = SendData[i].mPosition.x;
			toReturn.pos.y = SendData[i].mPosition.y;
			toReturn.pos.z = SendData[i].mPosition.z;

		
			//	Float3 norm;
			toReturn.norm.x = SendData[i].mNormal.x;
			toReturn.norm.y = SendData[i].mNormal.y;
			toReturn.norm.z = SendData[i].mNormal.z;

			//	Float2 uv;
			toReturn.uv.u = SendData[i].mUV.x;
			toReturn.uv.v = SendData[i].mUV.y;

			outbinFile.write((char*)&toReturn.pos, sizeof(Float3));
			outbinFile.write((char*)&toReturn.norm, sizeof(Float3));
			outbinFile.write((char*)&toReturn.uv, sizeof(Float2));
		}

		// Writing out the bones
		unsigned int numBones = (unsigned int)bone.size();
		outbinFile.write((char*)&numBones, sizeof(unsigned int));

		for (size_t i = 0; i < numBones; i++)
		{
			outbinFile.write((char*)&bone[i].mMyIndex, sizeof(int));
			outbinFile.write((char*)&bone[i].mParentIndex, sizeof(int));

			// FBX InverseMatrixStuff
			Float4x4 invMatrixStuff;
			for (size_t j = 0; j < 4; j++)
			{
				for (size_t k = 0; k < 4; k++)
				{
					invMatrixStuff.m[j][k] = (float)bone[i].mGlobalBindposeInverse[j][k];
				}
			}
			invMatrixStuff.m[3][1] *= -1;
			invMatrixStuff.m[3][2] *= -1;
			outbinFile.write((char*)&invMatrixStuff, sizeof(Float4x4));

			Keyframe *walk = bone[i].mAnimation;
			int numKeyframes = 0;
			while (walk)
			{
				++numKeyframes;
				walk = walk->mNext;
			}
			// num Keyframes
			walk = bone[i].mAnimation;
			outbinFile.write((char*)&numKeyframes, sizeof(int));
			for (int a = 0; a < numKeyframes; a++)
			{
				Float4x4 keyFrameAnimMatrixStuff;
				for (size_t j = 0; j < 4; j++)
				{
					for (size_t k = 0; k < 4; k++)
					{
						keyFrameAnimMatrixStuff.m[j][k] = (float)walk->mGlobalTransform[j][k];
					}
				}
				outbinFile.write((char*)&keyFrameAnimMatrixStuff, sizeof(Float4x4));
				outbinFile.write((char*)&walk->keyFrameTime, sizeof(float));
				walk = walk->mNext;
			}
		}
		// animations
		float animationTime = (float)exporter->getAnimationLength();
		outbinFile.write((char*)&animationTime, sizeof(float));

		outbinFile.close();
	}
}

void EXP::DLLTransit::loadFilefromBin(const char* inFileName, std::vector<VertexInfo> &returnData, std::vector<BoneInfo> &returnBone, Animation* animation)
{
//	VertexInfo
	VertexInfo pushData;
	std::ifstream inbinFile;

	inbinFile.open(inFileName, std::ios::binary | std::ios::in);
	int something = 0;
	unsigned int sizeOfFile = 0;
	inbinFile.read((char*)&sizeOfFile, sizeof(unsigned int));

	for (size_t i = 0; i < sizeOfFile; i++)
	{
		VertexInfo toReturn;
		inbinFile.read((char*)&toReturn.numIndicies, sizeof(unsigned int));

		for (size_t j = 0; j < toReturn.numIndicies; j++)
		{
			float blendwets;
			int boneind;
			inbinFile.read((char*)&blendwets, sizeof(float));
			inbinFile.read((char*)&boneind, sizeof(int));

			toReturn.blendWeights.push_back(blendwets);
			toReturn.boneIndices.push_back(boneind);


		}

	//	Float3 vert;
		inbinFile.read((char*)&toReturn.pos, sizeof(Float3));
	//	Float3 norm;
		inbinFile.read((char*)&toReturn.norm, sizeof(Float3));
	//	Float2 uv;
		inbinFile.read((char*)&toReturn.uv, sizeof(Float2));
		if (inbinFile.eof())
			break;
		returnData.push_back(toReturn);
		
	}

	// reading in the bones
	unsigned int numBones;
	inbinFile.read((char*)&numBones, sizeof(unsigned int));

	for (unsigned int i = 0; i < numBones; i++)
	{
		BoneInfo _joint;
		inbinFile.read((char*)&_joint.index, sizeof(int));
		inbinFile.read((char*)&_joint.parentIndex, sizeof(int));
		inbinFile.read((char*)&_joint.transform, sizeof(Float4x4));

		int numKeyframes;
		inbinFile.read((char*)&numKeyframes, sizeof(int));

		_joint.keyframes = new std::vector<KeyFrame*>();
		for (size_t a = 0; a < (unsigned int)numKeyframes; a++)
		{
			KeyFrame *incKey = new KeyFrame;
			inbinFile.read((char*)&incKey->transform, sizeof(Float4x4));
			inbinFile.read((char*)&incKey->keyTime, sizeof(float));

			_joint.keyframes->push_back(incKey);
		}
		returnBone.push_back(_joint);

		animation = new Animation();
		
	}
	inbinFile.read((char*)&animation->_time, sizeof(float));
	
	inbinFile.close();
}