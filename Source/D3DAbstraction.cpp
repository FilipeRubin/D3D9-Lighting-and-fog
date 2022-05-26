#include "D3DAbstraction.h"
#include "VertexStructs.h"

extern const int WINDOW_WIDTH;
extern const int WINDOW_HEIGHT;
extern const float ASPECT_RATIO;

constexpr int GetWindowHalfWidth()
{
	return (int)((double)WINDOW_WIDTH / 2.0);
}

constexpr int GetWindowHalfHeight()
{
	return (int)((double)WINDOW_HEIGHT / 2.0);
}

#define WINDOW_HALF_WIDTH GetWindowHalfWidth()
#define WINDOW_HALF_HEIGHT GetWindowHalfHeight()

global IDirect3D9* g_pD3D = nullptr;
global IDirect3DDevice9* g_pd3dDevice = nullptr;

static IDirect3DVertexBuffer9* s_pScenarioVB = nullptr;
static IDirect3DTexture9* s_pFloorTexture = nullptr;
static IDirect3DTexture9* s_pBricksTexture = nullptr;
static D3DMATERIAL9 s_Material;
static D3DLIGHT9 s_Light;

static ID3DXFont* s_pFont = nullptr;
static RECT s_fontRect = { 50, WINDOW_HEIGHT - 170, 0, 0 };

static IDirect3DTexture9* s_pCrosshairTexture = nullptr;
static ID3DXSprite* s_pSprite = nullptr;
static D3DXVECTOR3 s_CrosshairVector = { ((float)WINDOW_WIDTH / 2.0f) - 40.0f, ((float)WINDOW_HEIGHT / 2.0f) - 51.5f, 0.0f };

static TexVertex s_Scenario[]
{
	// Floor
	//   x     y      z    nx    ny    nz      u     v
	{-1.0f, 0.0f, -3.0f, 0.0f, 1.0f, 0.0f,  0.0f, 6.0f},
	{ 1.0f, 0.0f, -3.0f, 0.0f, 1.0f, 0.0f,  2.0f, 6.0f},
	{-1.0f, 0.0f,  3.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f},
	{ 1.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f,  2.0f, 2.0f},
	{ 5.0f, 0.0f,  3.0f, 0.0f, 1.0f, 0.0f,  6.0f, 0.0f},
	{ 5.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f,  6.0f, 2.0f},

	// Walls
	//   x     y      z    nx    ny    nz      u     v
	{-1.0f, 1.5f, -3.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f},
	{-1.0f, 0.0f, -3.0f, 0.0f, 1.0f, 0.0f,  0.0f, 1.5f},
	{-1.0f, 1.5f,  3.0f, 0.0f, 1.0f, 0.0f,  6.0f, 0.0f},
	{-1.0f, 0.0f,  3.0f, 0.0f, 1.0f, 0.0f,  6.0f, 1.5f},
	{ 5.0f, 1.5f,  3.0f, 0.0f, 1.0f, 0.0f, 12.0f, 0.0f},
	{ 5.0f, 0.0f,  3.0f, 0.0f, 1.0f, 0.0f, 12.0f, 1.5f},
	{ 5.0f, 1.5f,  1.0f, 0.0f, 1.0f, 0.0f, 14.0f, 0.0f},
	{ 5.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 14.0f, 1.5f},
	{ 1.0f, 1.5f,  1.0f, 0.0f, 1.0f, 0.0f, 20.0f, 0.0f},
	{ 1.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 20.0f, 1.5f},
	{ 1.0f, 1.5f, -3.0f, 0.0f, 1.0f, 0.0f, 24.0f, 0.0f},
	{ 1.0f, 0.0f, -3.0f, 0.0f, 1.0f, 0.0f, 24.0f, 1.5f},
	{-1.0f, 1.5f, -3.0f, 0.0f, 1.0f, 0.0f, 26.0f, 0.0f},
	{-1.0f, 0.0f, -3.0f, 0.0f, 1.0f, 0.0f, 26.0f, 1.5f}
};

//Gets the movement from the mouse and then locks it in the center of the screen
inline static void GetMouseMovement(int* x, int* y)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	if (x != nullptr)
		*x = WINDOW_HALF_WIDTH - cursorPos.x;
	if (y != nullptr)
		*y = WINDOW_HALF_HEIGHT - cursorPos.y;

	SetCursorPos(WINDOW_HALF_WIDTH, WINDOW_HALF_HEIGHT);
}

inline static void Prepare2D()
{
	D3DXCreateFont(g_pd3dDevice, 30, 0, 500, 0, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE, "Verdana", &s_pFont);

	D3DXCreateSprite(g_pd3dDevice, &s_pSprite);
}

inline static void PrepareMaterials()
{
	// Load textures
	D3DXCreateTextureFromFile(g_pd3dDevice, "Textures\\stone_floor.png", &s_pFloorTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Textures\\bricks.png", &s_pBricksTexture);
	D3DXCreateTextureFromFile(g_pd3dDevice, "Textures\\crosshair.png", &s_pCrosshairTexture);

	s_Material.Ambient.a = 1.0f;
	s_Material.Ambient.r = 1.0f;
	s_Material.Ambient.g = 1.0f;
	s_Material.Ambient.b = 1.0f;
	s_Material.Diffuse.a = 1.0f;
	s_Material.Diffuse.r = 1.0f;
	s_Material.Diffuse.g = 1.0f;
	s_Material.Diffuse.b = 1.0f;

	g_pd3dDevice->SetMaterial(&s_Material);

	ZeroMemory(&s_Light, sizeof(s_Light));
	s_Light.Type = D3DLIGHT_POINT;

	s_Light.Position.x = 0.9f;
	s_Light.Position.y = 0.75f;
	s_Light.Position.z = 1.1f;

	s_Light.Range = 1.0f;

	s_Light.Phi = D3DXToRadian(30.0f);
	s_Light.Theta = D3DXToRadian(1.0f);

	s_Light.Falloff = 0.0f;

	s_Light.Direction.x =  0.0f;
	s_Light.Direction.y = -1.0f;
	s_Light.Direction.z =  0.0f;

	s_Light.Diffuse.a = 1.0;
	s_Light.Diffuse.r = 1.0;
	s_Light.Diffuse.g = 1.0;
	s_Light.Diffuse.b = 1.0;

	g_pd3dDevice->SetLight(0, &s_Light);
	g_pd3dDevice->LightEnable(0, TRUE);
}

inline static void PrepareBuffers()
{
	// Create vertex buffer for the scenario
	g_pd3dDevice->CreateVertexBuffer(sizeof(s_Scenario), D3DUSAGE_WRITEONLY, dwTexFVF, D3DPOOL_DEFAULT, &s_pScenarioVB, nullptr);
	void* pVBOffset;
	s_pScenarioVB->Lock(0u, 0u, (void**)&pVBOffset, 0u);
	memcpy(pVBOffset, s_Scenario, sizeof(s_Scenario));
	s_pScenarioVB->Unlock();
}

inline static void AdjustStates(HWND& hwnd)
{
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(20, 20, 20));

	float fogStart = 0.5f;
	float fogEnd = 5.5f;
	g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fogStart);
	g_pd3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fogEnd);
	g_pd3dDevice->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_XRGB(15, 30, 60));
	g_pd3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);

	D3DXMATRIX projectionMatrix;

	D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DX_PI / 2.0f, ASPECT_RATIO, 0.1f, 100.0f);

	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	PrepareBuffers();
	PrepareMaterials();
	Prepare2D();
}

inline static void TransformScene()
{
	// CAMERA
	static D3DXMATRIX camera;
	static D3DXMATRIX lookAt;
	static D3DXMATRIX xRotMatrix;
	static D3DXMATRIX yRotMatrix;
	static D3DXMATRIX transMatrix;

	{ // Creating the camera "lookAt" matrix
		static D3DXVECTOR3 eye(-0.4f, 1.0f, 2.2f);
		static D3DXVECTOR3 at(-0.4f, 1.0f, 1.2f);
		static D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&lookAt, &eye, &at, &up);
	}

	static float xRot = 0.0f; // Accumulated rotation for the X-Axis
	static float yRot = 0.0f; // Accumulated rotation for the Y-Axis

	int xOffset = 0; // How much has the mouse moved in the X-Axis since the last frame
	int yOffset = 0; // How much has the mouse moved in the Y-Axis since the last frame

	GetMouseMovement(&yOffset, &xOffset);

	yRot += (float)yOffset * 0.002f;
	xRot += (float)xOffset * 0.002f;

	D3DXMatrixRotationX(&xRotMatrix, xRot);
	D3DXMatrixRotationY(&yRotMatrix, yRot);

	D3DXMatrixIdentity(&camera);
	D3DXMatrixMultiply(&camera, &camera, &lookAt);
	D3DXMatrixMultiply(&camera, &camera, &yRotMatrix);
	D3DXMatrixMultiply(&camera, &camera, &xRotMatrix);

	g_pd3dDevice->SetTransform(D3DTS_VIEW, &camera);
}

inline static void SceneManagement()
{
	static BOOL lightEnabled = false;
	static BOOL pressed = false;

	if ((GetKeyState(VK_LBUTTON) & 0x80))
	{
		if (!pressed)
		{
			g_pd3dDevice->LightEnable(0, lightEnabled);
			lightEnabled = !lightEnabled;
		}

		pressed = true;
	}
	else
	{
		pressed = false;
	}
}

void D3DAbstraction::InitD3D(HWND& hwnd)
{
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp = {};

	d3dpp.Windowed = FALSE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16; // Depth format
	d3dpp.BackBufferWidth = WINDOW_WIDTH;
	d3dpp.BackBufferHeight = WINDOW_HEIGHT;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; // Color format (32 bits in this case)
	d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;

	ShowCursor(FALSE);

	g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);

	AdjustStates(hwnd);
}

void D3DAbstraction::Render()
{
	TransformScene();
	SceneManagement();

	g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(15, 30, 60), 1.0f, NULL);

	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		g_pd3dDevice->SetStreamSource(0, s_pScenarioVB, 0, sizeof(TexVertex));
		g_pd3dDevice->SetFVF(dwTexFVF);
		g_pd3dDevice->SetTexture(0, s_pFloorTexture);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
		g_pd3dDevice->SetTexture(0, s_pBricksTexture);
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 6, 12);

		s_pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		s_pSprite->Draw(s_pCrosshairTexture, nullptr, nullptr, &s_CrosshairVector, D3DCOLOR_XRGB(255, 255, 255));
		s_pSprite->End();

		g_pd3dDevice->EndScene();
	}

	s_pFont->DrawText(nullptr, "Direct3D 9\nLighting, FOG and 2D screen elements test\nMade by Rubin", -1, &s_fontRect, DT_NOCLIP, D3DCOLOR_ARGB(180, 255, 255, 255));

	g_pd3dDevice->Present(nullptr, nullptr, NULL, nullptr);
}

void D3DAbstraction::Cleanup()
{
	if (g_pD3D != nullptr)
	{
		g_pD3D->Release();
	}
	if (g_pd3dDevice != nullptr)
	{
		g_pd3dDevice->Release();
	}
	if (s_pScenarioVB != nullptr)
	{
		s_pScenarioVB->Release();
	}
	if (s_pFloorTexture != nullptr)
	{
		s_pFloorTexture->Release();
	}
	if (s_pBricksTexture != nullptr)
	{
		s_pBricksTexture->Release();
	}
	if (s_pCrosshairTexture != nullptr)
	{
		s_pCrosshairTexture->Release();
	}
	if (s_pFont != nullptr)
	{
		s_pFont->Release();
	}
	if (s_pSprite != nullptr)
	{
		s_pSprite->Release();
	}
}