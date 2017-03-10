#include "Model.h"



Model::Model()
{
}


Model::~Model()
{
}

void Model::Init(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, char * filename)
{
	if (LoadFromFile("plane.obj"))
	{

	}
	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(GetVertexData()[0]) * GetVertexData().size();

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

	cBufferData.worldMatrix = cube2World;
	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(cBufferData);

	subData.pSysMem = &cBufferData;

	t_dev->CreateBuffer(&bDesc, &subData, &t_cbPerObjectBuffer);
}

void Model::Draw(ID3D11Device * t_dev, ID3D11DeviceContext * t_devcon, ID3D11Buffer * t_cbPerObjectBuffer, XMMATRIX t_camView, XMMATRIX t_camProjection)
{
	WVP = cube2World * t_camView * t_camProjection;
	cb.WVP = XMMatrixTranspose(WVP);
	t_devcon->UpdateSubresource(t_cbPerObjectBuffer, 0, NULL, &cb, 0, 0);
	unsigned int offset = 0, stride = sizeof(Vertex);
	t_devcon->VSSetConstantBuffers(0, 1, &t_cbPerObjectBuffer);
	t_devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
	t_devcon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	//Draw the second cube
	t_devcon->DrawIndexed(GetVertexData().size(), 0, 0);
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