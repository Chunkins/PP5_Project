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
	std::vector<Joint> bones = exporter->getSkeleton().mJoints;

	int vertSize = (int)exporter->getverts().size()/2;

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
		unsigned int boneCount = (unsigned int)bones.size();
		outbinFile.write((char*)&boneCount, sizeof(unsigned int));

		int frameCount = 0;
		Keyframe *walk;
		// in case the first bone lacks an animation
		int i = boneCount;
		while (--i!=-1)
		{
			walk = bones[i].mAnimation;
			if (walk) {
				std::vector<float> times;
				while (walk)
				{
					++frameCount;
					times.push_back(walk->keyFrameTime);
					walk = walk->mNext;
				}
				outbinFile.write((char*)&frameCount, sizeof(int));
				for each (float keytime in times)
				{
					outbinFile.write((char*)&keytime, sizeof(float));
				}
				break;
			}
		}

		for (size_t i = 0; i < boneCount; i++)
		{
			outbinFile.write((char*)&bones[i].mMyIndex, sizeof(int));
			outbinFile.write((char*)&bones[i].mParentIndex, sizeof(int));

			Float4x4 initTransform;
			for (size_t j = 0; j < 4; j++)
			{
				for (size_t k = 0; k < 4; k++)
				{
					initTransform.m[j][k] = (float)bones[i].mGlobalBindposeInverse[j][k];
				}
			}
			//initTransform.m[3][1] *= -1;
			//initTransform.m[3][2] *= -1;
			outbinFile.write((char*)&initTransform, sizeof(Float4x4));

			walk = bones[i].mAnimation;

			int l = -1; while (++l != frameCount)
			{
				if (walk) {
					Float4x4 keyFrameTransform;
					for (size_t j = 0; j < 4; j++)
						for (size_t k = 0; k < 4; k++)
							keyFrameTransform.m[j][k] = (float)walk->mGlobalTransform[j][k];
					//keyFrameTransform.m[3][1] *= -1;
					//keyFrameTransform.m[3][2] *= -1;
					outbinFile.write((char*)&keyFrameTransform, sizeof(Float4x4));
					walk = walk->mNext;
				}
				else
				{
					outbinFile.write((char*)&initTransform, sizeof(Float4x4));
				}
			}

		}
		// animations
		float animationTime = (float)exporter->getAnimationLength();
		outbinFile.write((char*)&animationTime, sizeof(float));

		outbinFile.close();
	}
}

void EXP::DLLTransit::loadFilefromBin(const char* inFileName, std::vector<VertexInfo> &returnData, std::vector<BoneInfo> &returnBone, Animation& animation)
{
	//	VertexInfo
	std::ifstream inbinFile;

	inbinFile.open(inFileName, std::ios::binary | std::ios::in);
	unsigned int sizeOfFile;
	inbinFile.read((char*)&sizeOfFile, sizeof(unsigned int));

	for (size_t i = 0; i < sizeOfFile; i++)
	{
		VertexInfo toReturn;
		inbinFile.read((char*)&toReturn.numIndicies, sizeof(unsigned int));
		size_t l = 0;
		for (; l < toReturn.numIndicies; l++)
		{
			float blendwets;
			int boneind;
			inbinFile.read((char*)&blendwets, sizeof(float));
			inbinFile.read((char*)&boneind, sizeof(int));

			toReturn.blendWeights.push_back(blendwets);
			toReturn.boneIndices.push_back(boneind);
		}
		while (l < 4) {
			toReturn.blendWeights.push_back(0);
			toReturn.boneIndices.push_back(0);
			++l;
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
	unsigned int boneCount;
	inbinFile.read((char*)&boneCount, sizeof(unsigned int));
	int frameCount;
	inbinFile.read((char*)&frameCount, sizeof(int));
	animation.frames.resize(frameCount);
	animation.times.resize(frameCount);
	int j = -1; while (++j != frameCount)
	{
		animation.frames[j].resize(40);
		inbinFile.read((char*)&animation.times[j], sizeof(float));
	}

	int k = -1; while (++k != boneCount)
	{
		BoneInfo joint;
		inbinFile.read((char*)&joint.index, sizeof(int));
		inbinFile.read((char*)&joint.parentIndex, sizeof(int));
		inbinFile.read((char*)&joint.transform, sizeof(Float4x4));
		returnBone.push_back(joint);
		int i = -1; while (++i != frameCount)
		{
			inbinFile.read((char*)&animation.frames[i][k], sizeof(Float4x4));
		}
	}

	inbinFile.read((char*)&animation.time, sizeof(float));

	inbinFile.close();
}