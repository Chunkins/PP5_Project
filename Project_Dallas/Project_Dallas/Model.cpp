#include "Model.h"


Model::Model()
{
	WVP = XMMatrixIdentity();
}


Model::~Model()
{
	Clean();
}

void Model::Init(ID3D11Device * t_dev, char * filename, const wchar_t* _textFileNAme)
{
	if (_textFileNAme)
		CreateDDSTextureFromFile(t_dev, _textFileNAme, nullptr, &pSRV);
	LoadFromFile("plane.obj");

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(Vertex) * GetVertexData().size();

	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&subData, sizeof(subData));
	const vector<Vertex> vertVec = GetVertexData();
	const Vertex* vertArr = vertVec.data();
	subData.pSysMem = vertArr;

	t_dev->CreateBuffer(&bDesc, &subData, &pVBuffer);

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(GetTris()[0]) * GetTris().size();

	const vector<unsigned int> indVec = GetTris();
	const unsigned int* indArr = indVec.data();
	subData.pSysMem = indArr;

	t_dev->CreateBuffer(&bDesc, &subData, &squareIndexBuffer);
}

void Model::InitFBX(ID3D11Device * t_dev, char * filename, const wchar_t* _textureFileName, XMMATRIX* _parentWVP, bool bones = false)
{
	parentWVP = _parentWVP;
	// texture creation
	if (_textureFileName)
		CreateDDSTextureFromFile(t_dev, _textureFileName, nullptr, &pSRV);

	//load file
	EXP::DLLTransit tmp;
	Animation anime;
	tmp.saveFiletoBin(filename, "Bone.txt");
	std::vector<VertexInfo> kindaTMP;
	tmp.loadFilefromBin("Bone.txt", kindaTMP, bonevec, &anime);

	//bones
	if (bones) {
		boneBuffers.resize(bonevec.size());
		unsigned i = bonevec.size(); while (--i != UINT32_MAX) {
			boneBuffers[i].WVP = XMMATRIX((float*)&bonevec[i].transform);
			boneBuffers[i].InitFBX(t_dev, "Bone.fbx", L"BoneTexture.dds", &WVP);
		}
	}

	//////////////
	// create the vertex buffer
	//////////////
	// get vertex information out of VertexInfo
	indexCount = kindaTMP.size();
	Vertex* OurVertices = new Vertex[indexCount];
	unsigned i = UINT32_MAX; while (++i != indexCount)
	{
		OurVertices[i].position.x = kindaTMP[i].pos.x;
		OurVertices[i].position.y = kindaTMP[i].pos.y;
		OurVertices[i].position.z = kindaTMP[i].pos.z;
		OurVertices[i].position.w = 1.f;

		OurVertices[i].normal.x = -kindaTMP[i].norm.x;
		OurVertices[i].normal.y = -kindaTMP[i].norm.y;
		OurVertices[i].normal.z = -kindaTMP[i].norm.z;
		OurVertices[i].uv.x = kindaTMP[i].uv.u;
		OurVertices[i].uv.y = 1-  kindaTMP[i].uv.v;
	}
	// describe the buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = sizeof(Vertex) * indexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = OurVertices;
	t_dev->CreateBuffer(&bd, &vertexBufferData, &pVBuffer);// create buffer
	delete[] OurVertices;
}

void Model::DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX& t_camView, XMMATRIX& t_camProjection)
{
	if(pSRV)
		t_devcon->PSSetShaderResources(0, 1, &pSRV);
	unsigned int offset = 0, stride = sizeof(Vertex);
	WVP = WVP * t_camView * t_camProjection;
	WVP = XMMatrixTranspose(WVP);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &WVP, 0, 0);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	t_devcon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	t_devcon->DrawIndexed(GetVertexData().size(), 0, 0);
}

void Model::Draw(ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX& t_camView, XMMATRIX& t_camProjection, bool _bones)
{
	if (pSRV)
		t_devcon->PSSetShaderResources(0, 1, &pSRV);
	unsigned int offset = 0, stride = sizeof(Vertex);
	XMMATRIX temp = XMMatrixTranspose(WVP*(*parentWVP)* t_camView * t_camProjection);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &temp, 0, 0);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	t_devcon->Draw(indexCount, 0);
	if (_bones) {
		unsigned int i = -1; while (++i!=boneBuffers.size())
			boneBuffers[i].Draw(t_devcon, t_cbPerObjectBuffer, t_camView, t_camProjection, false);
	}

}

void Model::Clean()
{
	
}

bool Model::LoadFromFile(const char* _path)

{
	ifstream reader;
	reader.open(_path);
	if (reader.is_open())
	{
		vector<XMFLOAT4> verts;
		vector<XMFLOAT4> norms;
		vector<XMFLOAT4> uvs;
		char buffer[255];
		while (!reader.eof())
		{
			reader.getline(buffer, 255);
			stringstream lnStream(buffer);
			string firstWord;
			lnStream >> firstWord;

			if (firstWord == "v")
			{
				XMFLOAT4 coords;
				lnStream >> coords.x >> coords.y >> coords.z;
				coords.w = 1;
				verts.push_back(coords);
			}
			else if (firstWord == "vn")
			{
				XMFLOAT4 coords;
				lnStream >> coords.x >> coords.y >> coords.z;
				coords.w = 1;
				norms.push_back(coords);
			}
			else if (firstWord == "vt")
			{
				XMFLOAT4 coords;
				lnStream >> coords.x >> coords.y;
				coords.y = 1 - coords.y;
				coords.z = coords.w = 1;
				uvs.push_back(coords);
			}
			else if (firstWord == "f")
			{
				while (true)
				{
					char tri[255];
					lnStream >> tri;
					stringstream ss(tri);
					string str;
					getline(ss, str, '/');
					unsigned int a = atoi(str.c_str());
					getline(ss, str, '/');
					unsigned int b = atoi(str.c_str());
					unsigned int c;
					ss >> c;
					indices.push_back(Vec3I(a, b, c));
					tris.push_back((unsigned int)indices.size() - 1);
					if (lnStream.eof())
						break;
				}
			}
		}
		reader.close();
		for (unsigned int i = 0; i < indices.size(); i++)
		{
			//////////////quick lazy fix will go
			norms[indices[i].z - 1].x = 1;
			norms[indices[i].z - 1].z = 1;
			norms[indices[i].z - 1].y = 1;
			norms[indices[i].z - 1].w = 1;
			/////////////////////////////////

			Vertex vpc;
			vpc.position = verts[indices[i].x - 1];
			vpc.uv = uvs[indices[i].y - 1];
			vpc.normal = norms[indices[i].z - 1];
			vertices.push_back(vpc);
		}
		return true;
	}
	return false;
}

const vector<Vertex> Model::GetVertexData() const
{
	return vertices;
}

const vector<unsigned int> Model::GetTris() const
{
	return tris;
}