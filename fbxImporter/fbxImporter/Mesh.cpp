#include "Mesh.h"
#include <fstream>
#include <sstream>

FBXImport::MeshDll::MeshDll()
{
}

FBXImport::MeshDll::~MeshDll()
{
}

bool FBXImport::MeshDll::LoadFromFile(const char* _path)
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
					indices.push_back(Vec3IDll(a, b, c));
					tris.push_back((unsigned int)indices.size() - 1);
					if (lnStream.eof())
						break;
				}
			}
		}
		reader.close();
		for (unsigned int i = 0; i < indices.size(); i++)
		{
			VertexDll vpc;
			vpc.position = verts[indices[i].x - 1];
			vpc.uv = uvs[indices[i].y - 1];
			vpc.normal = norms[indices[i].z - 1];
			vertices.push_back(vpc);
		}
		return true;
	}
	return false;
}

void FBXImport::MeshDll::BuildMesh(const vector<VertexDll> _vertices, const vector<unsigned int> _indices)
{
	vertices = _vertices;
	tris = _indices;
}

const vector<FBXImport::VertexDll> FBXImport::MeshDll::GetVertexData() const
{
	return vertices;
}

const vector<unsigned int> FBXImport::MeshDll::GetTris() const
{
	return tris;
}