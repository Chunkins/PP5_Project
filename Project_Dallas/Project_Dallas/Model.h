#pragma once
#include <Windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <DirectXMath.h>
using namespace std;
using namespace DirectX;

struct Vertex
{
	
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 uv;
};

struct Vec3Indices
{
	int x, y, z;

	Vec3Indices() : x(0), y(0), z(0) {}
	Vec3Indices(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};

class Model
{
	vector<Vertex> vertices;
	vector<Vec3Indices> indices;
	vector<unsigned int> tris;

	bool LoadFromFile(const char* _path);
	void BuildMesh(const vector<Vertex> _vertices, const vector<unsigned int> _indices);

	const vector<Vertex> GetVertexData() const;
	const vector<unsigned int> GetTris() const;

public:
	Model();
	~Model();
};

