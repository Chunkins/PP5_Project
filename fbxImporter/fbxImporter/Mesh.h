#pragma once
#include <DirectXMath.h>
#include <vector>
//#include "LinearPrimitives.h"

// Namespaces
using namespace std;

using namespace DirectX;

namespace FBXImport
{
	struct VertexDll
	{
		XMFLOAT4 position;
		XMFLOAT4 normal;
		XMFLOAT4 uv;
	};

	struct Vec3IDll
	{
		int x, y, z;

		Vec3IDll() : x(0), y(0), z(0) {}
		Vec3IDll(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
	};

	class MeshDll
	{
		vector<VertexDll> vertices;
		vector<Vec3IDll> indices;
		vector<unsigned int> tris;

	public:
		MeshDll();
		~MeshDll();

		bool LoadFromFile(const char* _path);
		void BuildMesh(const vector<VertexDll> _vertices, const vector<unsigned int> _indices);

		const vector<VertexDll> GetVertexData() const;
		const vector<unsigned int> GetTris() const;
	};
}
