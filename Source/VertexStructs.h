#pragma once

#include <d3d9.h>

// extern const DWORD dwColorFVF; not used in this project
extern const DWORD dwTexFVF;

/*

The following struct is not used in this project

struct ColorVertex
{
	float x, y, z;
	D3DCOLOR color;
};

*/

struct TexVertex
{
	float  x,  y,  z;
	float nx, ny, nz;
	float  u,  v;
};