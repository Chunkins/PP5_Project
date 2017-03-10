#pragma once
//Pre Compiled Header
//Includes
#include <Windows.h>
#include <crtdbg.h>
#include <d3d11.h>
#include <memory>
#include <ctime>
#include <vector>
#include <map>
#include <chrono>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
//#include <agile.h>


// Libraries
#pragma comment(lib, "d3d11.lib")

// Namespaces
using namespace std;
using namespace DirectX;

struct PerModelVertexData
{
	XMMATRIX worldMatrix;
};

struct cbPerObject
{
	XMMATRIX  WVP;
};