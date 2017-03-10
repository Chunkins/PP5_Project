#include "Model.h"


Model::Model()

{
	squareIndexBuffer = nullptr;
}


Model::~Model()
{

}

void Model::Init(ID3D11Device * t_dev, char * filename)
{
	if (LoadFromFile("plane.obj"))
	{

	}

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

void Model::InitFBX(ID3D11Device * t_dev, char * filename)
{
	EXP::DLLTransit tmp;
	Animation anime;
	tmp.saveFiletoBin(filename, "../Bone.txt");
	std::vector<VertexInfo> kindaTMP;
	tmp.loadFilefromBin("../Bone.txt", kindaTMP, bonevec, &anime);
	boneBuffers.resize(bonevec.size());
	unsigned i = bonevec.size(); while (--i!=UINT32_MAX)
	{
		boneBuffers[i].InitBone(t_dev, "../Bone.fbx");
	}
	// create a triangle using the VERTEX struct
	indexCount = kindaTMP.size();

	Vertex* OurVertices = new Vertex[indexCount];
	i = UINT32_MAX; while (++i != indexCount)
	{
		OurVertices[i].position.x = kindaTMP[i].pos.x;
		OurVertices[i].position.y = kindaTMP[i].pos.y;
		OurVertices[i].position.z = kindaTMP[i].pos.z;
		OurVertices[i].uv.x = kindaTMP[i].uv.u;
		OurVertices[i].uv.y = kindaTMP[i].uv.v;
		OurVertices[i].normal.x = kindaTMP[i].norm.x;
		OurVertices[i].normal.y = kindaTMP[i].norm.y;
		OurVertices[i].normal.z = kindaTMP[i].norm.z;
	}

	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.ByteWidth = sizeof(Vertex) * indexCount;             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer



	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = OurVertices;
	t_dev->CreateBuffer(&bd, &vertexBufferData, &pVBuffer);       // create the buffer
	delete[] OurVertices;
}

void Model::InitBone(ID3D11Device * t_dev, char * filename)
{
	EXP::DLLTransit tmp;
	Animation anime;
	tmp.saveFiletoBin(filename, "../Bone.txt");
	std::vector<VertexInfo> kindaTMP;
	tmp.loadFilefromBin("../Bone.txt", kindaTMP, bonevec, &anime);

	// create a triangle using the VERTEX struct
	indexCount = kindaTMP.size();

	Vertex* OurVertices = new Vertex[indexCount];
	unsigned i = UINT32_MAX; while (++i != indexCount)
	{
		OurVertices[i].position.x = kindaTMP[i].pos.x;
		OurVertices[i].position.y = kindaTMP[i].pos.y;
		OurVertices[i].position.z = kindaTMP[i].pos.z;
		OurVertices[i].uv.x = kindaTMP[i].uv.u;
		OurVertices[i].uv.y = kindaTMP[i].uv.v;
		OurVertices[i].normal.x = kindaTMP[i].norm.x;
		OurVertices[i].normal.y = kindaTMP[i].norm.y;
		OurVertices[i].normal.z = kindaTMP[i].norm.z;
	}

	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.ByteWidth = sizeof(Vertex) * indexCount;             // size is the VERTEX struct * 3
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer



	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = OurVertices;
	t_dev->CreateBuffer(&bd, &vertexBufferData, &pVBuffer);       // create the buffer
	delete[] OurVertices;
}

void Model::DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection)
{
	WVP = cube2World * t_camView * t_camProjection;
	cb.WVP = XMMatrixTranspose(WVP);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &cb, 0, 0);
	unsigned int offset = 0, stride = sizeof(Vertex);
	t_devcon->VSSetConstantBuffers(0, 1, &t_cbPerObjectBuffer);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	t_devcon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	t_devcon->DrawIndexed(GetVertexData().size(), 0, 0);
}

void Model::Draw(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection, XMMATRIX _worldPos, bool _bones)
{
	unsigned int offset = 0, stride = sizeof(Vertex);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	WVP = _worldPos * t_camView * t_camProjection;
	WVP = XMMatrixTranspose(WVP);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &WVP, 0, 0);
	t_devcon->VSSetConstantBuffers(0, 1, &t_cbPerObjectBuffer);
	if (_bones == false)
	{
		t_devcon->Draw(indexCount, 0);

	}

	if (_bones) {
		unsigned int i = -1; while (++i!=boneBuffers.size())
		{
			XMMATRIX scaleDown = XMMATRIX((float*)&bonevec[i].transform);
			scaleDown = scaleDown*XMMatrixTranslationFromVector(WVP.r[3]);
			scaleDown = scaleDown*XMMatrixScaling(.8f, .8f, .8f);
			boneBuffers[i].Draw(t_dev, t_devcon, t_cbPerObjectBuffer, t_camView, t_camProjection, scaleDown, false);
		}
	}

}

void Model::Clean()
{
	boneBuffers.clear();
	if (squareIndexBuffer)
		squareIndexBuffer->Release();
	if (pVBuffer)
		pVBuffer->Release();
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

void Model::BuildMesh(const vector<Vertex> _vertices, const vector<unsigned int> _indices)
{
	vertices = _vertices;
	tris = _indices;
}

const vector<Vertex> Model::GetVertexData() const
{
	return vertices;
}

const vector<unsigned int> Model::GetTris() const
{
	return tris;
}