#include "Model.h"


Model::Model(bool _Display, bool _isFBX)
{
	WVP = XMMatrixIdentity();
	Display = _Display;
	isFBX = _isFBX;

}


Model::~Model()
{
	Clean();
}

void Model::Init(ID3D11Device * t_dev, char * filename, const wchar_t* _textFileNAme)
{
	if (_textFileNAme)
		CreateDDSTextureFromFile(t_dev, _textFileNAme, nullptr, &pSRV);
	vector<Vertex> temp;
	vector<unsigned int> tempTris;
	LoadFromFile("plane.obj", temp, tempTris);
	indexCount = temp.size();

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(Vertex) * indexCount;

	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&subData, sizeof(subData));
	subData.pSysMem = temp.data();

	t_dev->CreateBuffer(&bDesc, &subData, &pVBuffer);

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(unsigned int) * tempTris.size();
	subData.pSysMem = tempTris.data();

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
	tmp.saveFiletoBin(filename, "Bone.txt");
	std::vector<VertexInfo> kindaTMP;
	tmp.loadFilefromBin("Bone.txt", kindaTMP, bonevec, anim);


	//////////////
	// create the vertex buffer
	//////////////
	// get vertex information out of VertexInfo
	indexCount = kindaTMP.size();
	Vertex* OurVertices = new Vertex[indexCount];
	unsigned i = UINT32_MAX; while (++i != indexCount)
	{
		OurVertices[i].boneCount.x = kindaTMP[i].numIndicies;

		OurVertices[i].position.x = kindaTMP[i].pos.x;
		OurVertices[i].position.y = kindaTMP[i].pos.y;
		OurVertices[i].position.z = kindaTMP[i].pos.z;
		OurVertices[i].position.w = 1.f;

		OurVertices[i].normal.x = kindaTMP[i].norm.x;
		OurVertices[i].normal.y = kindaTMP[i].norm.y;
		OurVertices[i].normal.z = kindaTMP[i].norm.z;
		OurVertices[i].normal.w = 1.f;

		OurVertices[i].uv.x = kindaTMP[i].uv.u;
		OurVertices[i].uv.y = 1 - kindaTMP[i].uv.v;

		OurVertices[i].blendWeights.x = kindaTMP[i].blendWeights[0];
		OurVertices[i].blendWeights.y = kindaTMP[i].blendWeights[1];
		OurVertices[i].blendWeights.z = kindaTMP[i].blendWeights[2];
		OurVertices[i].blendWeights.w = kindaTMP[i].blendWeights[3];

		OurVertices[i].blendIndices.x = kindaTMP[i].boneIndices[0];
		OurVertices[i].blendIndices.y = kindaTMP[i].boneIndices[1];
		OurVertices[i].blendIndices.z = kindaTMP[i].boneIndices[2];
		OurVertices[i].blendIndices.w = kindaTMP[i].boneIndices[3];


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

void Model::DrawIndexed(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX& t_camProjection)
{
	if (pSRV)
		t_devcon->PSSetShaderResources(0u, 1u, &pSRV);
	unsigned int offset = 0, stride = sizeof(Vertex);
	XMMATRIX temp = (WVP * t_camProjection);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &temp, 0, 0);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	t_devcon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	t_devcon->DrawIndexed(indexCount, 0, 0);
}

void Model::Draw(ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX& t_camProjection, bool _bones, ID3D11ShaderResourceView* pSRVB, ID3D11Buffer* pVBufferB, unsigned int _count, ID3D11Buffer* _frameMatrices)
{
	if (Display)
	{
		if (pSRV)
			t_devcon->PSSetShaderResources(0, 1, &pSRV);
		unsigned int offset = 0, stride = sizeof(Vertex);
		XMMATRIX temp = (WVP*(*parentWVP) * t_camProjection);
		t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &temp, 0, 0);
		t_devcon->UpdateSubresource(_frameMatrices, 0, NULL, anim.frames[frame].data(), 0, 0);
		t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
		t_devcon->Draw(indexCount, 0);
		if (_bones) {
			t_devcon->PSSetShaderResources(0, 1, &pSRVB);
			t_devcon->IASetVertexBuffers(0, 1, &pVBufferB, &stride, &offset);
			unsigned int i = -1; while (++i != bonevec.size()) {
				XMMATRIX tempB = (XMMATRIX((float*)&anim.frames[frame][i])*WVP*t_camProjection);

				t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &tempB, 0, 0);
				t_devcon->Draw(_count, 0);
			}
		}
	}

}

void Model::Clean()
{

}

bool Model::LoadFromFile(const char* _path, vector<Vertex>& _verts, vector<unsigned int>& _tris)

{
	ifstream reader;
	reader.open(_path);
	if (reader.is_open())
	{
		vector<XMFLOAT4> verts;
		vector<XMFLOAT4> norms;
		vector<XMFLOAT4> uvs;
		vector<Vec3I> indices;
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
					_tris.push_back((unsigned int)indices.size() - 1);
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
			_verts.push_back(vpc);
		}
		return true;
	}
	return false;
}