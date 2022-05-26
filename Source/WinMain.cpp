#include "D3DAbstraction.h"

#define CLASS_NAME "WNDCLASS1"

extern const int WINDOW_WIDTH = 1280;
extern const int WINDOW_HEIGHT = 720;
extern const float ASPECT_RATIO = ((const float)WINDOW_WIDTH / (const float)WINDOW_HEIGHT);

LRESULT WINAPI MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
	WNDCLASS wc = {};

	wc.hInstance = hInstance;
	wc.lpfnWndProc = MsgProc;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&wc);

	HWND hwnd = CreateWindow(CLASS_NAME, "D3D Lighting Test", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, nullptr);

	D3DAbstraction::InitD3D(hwnd);

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			D3DAbstraction::Render();
		}
	}

	D3DAbstraction::Cleanup();
	UnregisterClass(CLASS_NAME, hInstance);
	return 0;
}