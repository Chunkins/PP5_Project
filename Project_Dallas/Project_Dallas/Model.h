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
	XMUINT4 boneCount;
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 uv;
	XMFLOAT4 blendWeights;
	XMINT4 blendIndices;
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
	// methods
	Model(bool _Display = true, bool _isFBX = false);
	~Model();
	void Init(ID3D11Device * t_dev, char * filename, const wchar_t*);
	void InitFBX(ID3D11Device * t_dev, char * filename, const wchar_t* _textFileNAme, XMMATRIX*, bool);
	void DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX& t_camProjection);

	void Draw(ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX&, bool, ID3D11ShaderResourceView*, ID3D11Buffer*, unsigned int, ID3D11Buffer*);
	bool LoadFromFile(const char* _path, vector<Vertex>&, vector<unsigned int>&);
	void Clean();

	// members
	ID3D11Buffer* squareIndexBuffer = nullptr;
	ID3D11Buffer *pVBuffer;
	ID3D11ShaderResourceView* pSRV = nullptr;

	unsigned int indexCount, frame = 0u;
	
	XMMATRIX* parentWVP;
	vector<BoneInfo> bonevec;
	XMMATRIX WVP;
	Animation anim;
	bool isFBX; //Know if Model is an FBX or not
	bool Display; //Know if  we want to Display the model (Allows Toggling)

};

