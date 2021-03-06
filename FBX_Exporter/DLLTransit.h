#pragma once

#include <vector>
#include <DirectXMath.h>


#ifdef FBXEXPORT_EXPORTS
#define DLLEXPORT1 __declspec(dllexport)
#else
#define DLLEXPORT1 __declspec(dllimport)
#endif // FBXEXPORT_EXPORTS

struct UnsignedInt4
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int w;
};
struct Float2
{
	float u;
	float v;
};
struct Float3
{
	float x;
	float y;
	float z;
};
struct Float4
{
	float x;
	float y;
	float z;
	float w;
};

struct Float4x4
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};

	Float4x4() {
		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				m[i][j] = 0;
	}
	Float4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
	{
		m[0][0] = m00;	m[0][1] = m01;	m[0][2] = m02;	m[0][3] = m03;
		m[1][0] = m10;	m[1][1] = m11;	m[1][2] = m12;	m[1][3] = m13;
		m[2][0] = m20;	m[2][1] = m21;	m[2][2] = m22;	m[2][3] = m23;
		m[3][0] = m30;	m[3][1] = m31;	m[3][2] = m32;	m[3][3] = m33;
	}

	float       operator() (size_t Row, size_t Column) const { return m[Row][Column]; }
	float&      operator() (size_t Row, size_t Column) { return m[Row][Column]; }

	Float4x4& operator= (const Float4x4& _Float4x4)
	{		
		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				m[i][j] = _Float4x4.m[i][j];
		return *this;
	}
};

struct VertexInfo
{
	unsigned int numIndicies;
	std::vector<float> blendWeights;
	std::vector<int> boneIndices;

	Float3 pos;
	Float3 norm;
	Float2 uv;
};
struct BoneInfo
{
	//int index;
	//int parentIndex;
	//BoneInfo *parent;
	//std::vector<BoneInfo*> children;
	std::vector<DirectX::XMMATRIX> frameTransforms;
};
struct Animation
{
	float time =0.f;
	std::vector<float> times;
	std::vector<std::vector<DirectX::XMMATRIX>> frames;
};




namespace EXP
{
	class DLLEXPORT1 DLLTransit
	{
	public:
		void saveFiletoBin(const char* inFileName, const char* binFileName);
		void loadFilefromBin(const char* inFileName, std::vector<VertexInfo> &returnData, std::vector<BoneInfo> &returnBone, Animation& animation);




	};
}