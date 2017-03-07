#pragma once

#include <vector>

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
	//	Float4x4 toReturn;
		
		for (size_t i = 0; i < 4; i++)
			for (size_t j = 0; j < 4; j++)
				m[i][j] = _Float4x4.m[i][j];
		return *this;
	}
};

struct KeyFrame
{
	float keyTime;
	Float4x4 transform;
};
struct VertexInfo
{
	unsigned int numIndicies;
	std::vector<float> blendWeights;
	std::vector<int> boneIndices;

	//	unsigned int CtrlPntIndeics;

	Float3 vert;
	Float3 norm;
	Float2 uv;
	//	Bone bones;
};

struct BoneInfo
{
	int index;
	int parentIndex;
	BoneInfo *Parent;
	Float4x4 transform;

	std::vector<BoneInfo*> children;
	std::vector<KeyFrame*>* keyframes;
};

struct Animation
{
	float _time;
	std::vector<BoneInfo*> bones;
};


namespace EXP
{
	class DLLEXPORT1 DLLTransit
	{
	private:
		/*bool LoadScene(const char* inFileName, const char* inOutputPath);
		bool LoadScene(const char* inFileName);*/
		//		void getVerticies(std::vector<VertexInfo> &returnData, const char* inFileName);
	public:

		//		std::vector<VertexInfo> getVerticies(std::vector<VertexInfo> &returnData, const char* inFileName);
		void saveFiletoBin(const char* inFileName, const char* binFileName);
		void loadFilefromBin(const char* inFileName, std::vector<VertexInfo> &returnData, std::vector<BoneInfo> &returnBone, Animation* animation);




	};
}