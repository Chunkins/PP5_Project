#pragma once
#include <Windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <DirectXMath.h>
#include "DLLTransit.h"
#include "PCH.h"
using namespace std;
using namespace DirectX;

struct Vertex
{
	
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 uv;
};

struct Vec3I
{
	int x, y, z;

	Vec3I() : x(0), y(0), z(0) {}
	Vec3I(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};

class Model
{
public:
	unsigned int indexCount;
	void Init(ID3D11Device * t_dev, char * filename);
	void InitFBX(ID3D11Device * t_dev, char * filename);
	void InitBone(ID3D11Device * t_dev, char * filename);
	void DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection);
	void Draw(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection, XMMATRIX _worldPos, bool bones);
	void Clean();
	ID3D11Buffer* squareIndexBuffer;
	ID3D11Buffer *pVBuffer;
	vector< Model> boneBuffers;
	PerModelVertexData cBufferData;
	XMMATRIX cube2World;
	cbPerObject cb;
	XMMATRIX WVP;
	vector<Vertex> vertices;
	vector<Vec3I> indices;
	vector<unsigned int> tris;
	vector<BoneInfo> bonevec;

	bool LoadFromFile(const char* _path);
	void BuildMesh(const vector<Vertex> _vertices, const vector<unsigned int> _indices);

	const vector<Vertex> GetVertexData() const;
	const vector<unsigned int> GetTris() const;

	Model();
	~Model();
};

