#pragma once
#include <d3dx9.h>

#define global // It does serve no purpose at all... It's just to explicitly tell if a variable is global

namespace D3DAbstraction
{
	void InitD3D(HWND& hwnd);
	void Render();
	void Cleanup();
}