#pragma once

#include <iostream>
#include <Windows.h>
#include <stdint.h>
#include <DirectXMath.h>
#include <vector>
#include <algorithm>
#include <fbxsdk.h>
#include <unordered_map>
#include <string>


class MathHelper
{
public:

	static const DirectX::XMFLOAT3 vector3Epsilon;
	static const DirectX::XMFLOAT2 vector2Epsilon;
	static const DirectX::XMFLOAT3 vector3True;
	static const DirectX::XMFLOAT2 vector2True;

	static bool CompareVector2WithEpsilon(const DirectX::XMFLOAT2& lhs, const DirectX::XMFLOAT2& rhs);
	static bool CompareVector3WithEpsilon(const DirectX::XMFLOAT3& lhs, const DirectX::XMFLOAT3& rhs);
};

class Utilities
{
public:

	// This function should be changed if exporting to another format
	static void WriteMatrix(std::ostream& inStream, FbxMatrix& inMatrix, bool inIsRoot);

	static void PrintMatrix(FbxMatrix& inMatrix);

	static FbxAMatrix GetGeometryTransformation(FbxNode* inNode);

	static std::string GetFileName(const std::string& inInput);

	static std::string RemoveSuffix(const std::string& inInput);
};

class Material
{
public:
	std::string m_Name;
	DirectX::XMFLOAT3 mAmbient;
	DirectX::XMFLOAT3 mDiffuse;
	DirectX::XMFLOAT3 mEmissive;
	double mTransparencyFactor;
	std::string mDiffuseMapName;
	std::string mEmissiveMapName;
	std::string mGlossMapName;
	std::string mNormalMapName;
	std::string mSpecularMapName;

	virtual void WriteToStream(std::ostream& inStream) = 0;
};

class LambertMaterial : public Material
{
public:

	void WriteToStream(std::ostream& inStream)
	{
		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;

		if (!mDiffuseMapName.empty())
		{
			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
		}

		if (!mNormalMapName.empty())
		{
			inStream << "NormalMap: " << mNormalMapName << std::endl;
		}
	}
};

class PhongMaterial : public Material
{
public:
	DirectX::XMFLOAT3 mSpecular;
	DirectX::XMFLOAT3 mReflection;
	double mSpecularPower;
	double mShininess;
	double mReflectionFactor;

	void WriteToStream(std::ostream& inStream)
	{
		inStream << "Ambient: " << mAmbient.x << " " << mAmbient.y << " " << mAmbient.z << std::endl;
		inStream << "Diffuse: " << mDiffuse.x << " " << mDiffuse.y << " " << mDiffuse.z << std::endl;
		inStream << "Emissive: " << mEmissive.x << " " << mEmissive.y << " " << mEmissive.z << std::endl;
		inStream << "Specular: " << mSpecular.x << " " << mSpecular.y << " " << mSpecular.z << std::endl;
		inStream << "SpecPower: " << mSpecularPower << std::endl;
		inStream << "Reflectivity: " << mReflection.x << " " << mReflection.y << " " << mReflection.z << std::endl;

		if (!mDiffuseMapName.empty())
		{
			inStream << "DiffuseMap: " << mDiffuseMapName << std::endl;
		}

		if (!mNormalMapName.empty())
		{
			inStream << "NormalMap: " << mNormalMapName << std::endl;
		}
	}
};

struct PNTVertex
{
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mNormal;
	DirectX::XMFLOAT2 mUV;

	bool operator==(const PNTVertex& rhs) const
	{
		uint32_t position;
		uint32_t normal;
		uint32_t uv;

		DirectX::XMVectorEqualR(&position, XMLoadFloat3(&(this->mPosition)), XMLoadFloat3(&rhs.mPosition));
		DirectX::XMVectorEqualR(&normal, XMLoadFloat3(&(this->mNormal)), XMLoadFloat3(&rhs.mNormal));
		DirectX::XMVectorEqualR(&uv, XMLoadFloat2(&(this->mUV)), XMLoadFloat2(&rhs.mUV));

		return DirectX::XMComparisonAllTrue(position) && DirectX::XMComparisonAllTrue(normal) && DirectX::XMComparisonAllTrue(uv);
	}
};

struct VertexBlendingInfo
{
	unsigned int mBlendingIndex;
	double mBlendingWeight;

	VertexBlendingInfo() :
		mBlendingIndex(0),
		mBlendingWeight(0.0)
	{}

	bool operator < (const VertexBlendingInfo& rhs)
	{
		return (mBlendingWeight > rhs.mBlendingWeight);
	}
};

struct PNTIWVertex
{
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mNormal;
	DirectX::XMFLOAT2 mUV;
	std::vector<VertexBlendingInfo> mVertexBlendingInfos;

	void SortBlendingInfoByWeight()
	{
		std::sort(mVertexBlendingInfos.begin(), mVertexBlendingInfos.end());
	}

	bool operator==(const PNTIWVertex& rhs) const
	{
		bool sameBlendingInfo = true;

		if (!(mVertexBlendingInfos.empty() && rhs.mVertexBlendingInfos.empty()))
		{
			for (unsigned int i = 0; i < 4; ++i)
			{
				if (mVertexBlendingInfos[i].mBlendingIndex != rhs.mVertexBlendingInfos[i].mBlendingIndex ||
					abs(mVertexBlendingInfos[i].mBlendingWeight - rhs.mVertexBlendingInfos[i].mBlendingWeight) > 0.001)
				{
					sameBlendingInfo = false;
					break;
				}
			}
		}

		bool result1 = MathHelper::CompareVector3WithEpsilon(mPosition, rhs.mPosition);
		bool result2 = MathHelper::CompareVector3WithEpsilon(mNormal, rhs.mNormal);
		bool result3 = MathHelper::CompareVector2WithEpsilon(mUV, rhs.mUV);

		return result1 && result2 && result3 && sameBlendingInfo;
	}
};

// Utility
struct BlendingIndexWeightPair
{
	unsigned int mBlendingIndex;
	double mBlendingWeight;

	BlendingIndexWeightPair() :
		mBlendingIndex(0),
		mBlendingWeight(0)
	{}
};


struct CtrlPoint
{
	DirectX::XMFLOAT3 mPosition;
	std::vector<BlendingIndexWeightPair> mBlendingInfo;

	CtrlPoint()
	{
		mBlendingInfo.reserve(4);
	}
};

struct Keyframe
{
	FbxLongLong mFrameNum;
	FbxAMatrix mGlobalTransform;
	Keyframe* mNext;
	float keyFrameTime;

	Keyframe() :
		mNext(nullptr)
	{}
};

struct Joint
{
	std::string mName;
	int mParentIndex;
	int mMyIndex;
	FbxAMatrix mGlobalBindposeInverse;
	Keyframe* mAnimation;
	FbxNode* mNode;

	Joint() :
		mNode(nullptr),
		mAnimation(nullptr)
	{
		mGlobalBindposeInverse.SetIdentity();
		mParentIndex = -1;
	}

	~Joint()
	{
		while (mAnimation)
		{
			Keyframe* temp = mAnimation->mNext;
			delete mAnimation;
			mAnimation = temp;
		}
	}
};

struct Skeleton
{
	std::vector<Joint> mJoints;
};

struct Triangle
{
	std::vector<unsigned int> mIndices;
	std::string mMaterialName;
	unsigned int mMaterialIndex;

	bool operator<(const Triangle& rhs)
	{
		return mMaterialIndex < rhs.mMaterialIndex;
	}
};
