#pragma once
// Basically a precompiled header. Include anything here that you'll need universally
//Includes dll
#include <Windows.h>
#include <crtdbg.h>
#include <d3d11.h>
#include <memory>
#include <vector>
#include <fstream>

// Namespaces
using namespace std;

#define ZMem(variable) ZeroMemory(&variable, sizeof(variable))
