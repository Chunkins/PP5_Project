#pragma once
#include <Windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <DirectXMath.h>
#include "DLLTransit.h"
#include "PCH.h"
#include "DDSTextureLoader.h"
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
	void Init(ID3D11Device * t_dev, char * filename, const wchar_t*);
	void InitFBX(ID3D11Device * t_dev, char * filename, const wchar_t* _textFileNAme);
	void InitBone(ID3D11Device * t_dev, char * filename, const wchar_t* _textFileNAme);
	void DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection);
	void Draw(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection, bool bones);
	void Clean();
	ID3D11Buffer* squareIndexBuffer;
	ID3D11Buffer *pVBuffer;
	vector< Model> boneBuffers;
	XMMATRIX WVP;
	vector<Vertex> vertices;
	vector<Vec3I> indices;
	vector<unsigned int> tris;
	vector<BoneInfo> bonevec;
	ID3D11ShaderResourceView* pSRV = nullptr;

	bool LoadFromFile(const char* _path);

	const vector<Vertex> GetVertexData() const;
	const vector<unsigned int> GetTris() const;

	Model();
	~Model();
};

