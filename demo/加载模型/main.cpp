#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include<iostream>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <cstring>
#include <windows.h>
#include <tchar.h>
#include<ctime>
#include<iostream>
#include<vector>
#include"s_math.h"
#include "rasterize.h"
#include "render.h"

using namespace std;
//=====================================================================
// Win32 窗口及图形绘制：为 device 提供一个 DibSection 的 FB
//=====================================================================
const float PI = 3.141592;
#define radians(x) x*PI/180.0f
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];	// 当前键盘按下状态
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// 老的 BITMAP
unsigned char* screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR* title);	// 屏幕初始化
int screen_close(void);								// 关闭屏幕
void screen_dispatch(void);							// 处理消息
void screen_update(void);							// 显示 FrameBuffer

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口并设置标题
int screen_init(int w, int h, const TCHAR* title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;
bool mousechange = false;
s_vector front;
static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_MOUSEMOVE:
	{   mousechange = true;
		float xpos = LOWORD(lParam);
	    float ypos = HIWORD(lParam);
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		
			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos;
			lastX = xpos;
			lastY = ypos;
			float sensitivity = 0.05;
			xoffset *= sensitivity;
			yoffset *= sensitivity;
			yaw += xoffset;
			pitch += yoffset;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;



			front.x = cos(radians(yaw)) * cos(radians(pitch));
			front.y = sin(radians(pitch));
			front.z = sin(radians(yaw)) * cos(radians(pitch));
			front.w = 1.0f;
			front.normalize();
		
		break;
	}
	case WM_CLOSE: screen_exit = 1; break;
	case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}


//=====================================================================
// 主程序
//=====================================================================
vertex_t mesh[36] = {
	// Positions                  // Texture Coords  //color           //rhw // Normals
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,-1.0f,0.0f},1},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f ,1.0f}, 1,{ 0.0f,  0.0f,-1.0f,0.0f},1},
	{{0.5f,  0.5f, -0.5f, 1.0f}, {1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f ,1.0f},1, {0.0f,  0.0f,-1.0f ,0.0f},1},
	{{0.5f,  0.5f, -0.5f, 1.0f}, { 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f ,1.0f},1, {0.0f,  0.0f,-1.0f,0.0f},1},
	{{0.5f, -0.5f, -0.5f, 1.0f}, {1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,-1.0f ,0.0f},1},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f,  0.0f,-1.0f,0.0f},1},

	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},1},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},1},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},1},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},1},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},1},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},1},

	{{-0.5f,  0.5f,  0.5f, 1.0f}, { 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f}, 1, {-1.0f,  0.0f,  0.0f,0.0f},1},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},1},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { -1.0f,  0.0f,  0.0f,0.0f},1},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},1},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f}, 1, { -1.0f,  0.0f,  0.0f,0.0f},1},
	{{-0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},1},

	{{0.5f,  0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {1.0f,  0.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},1},
	{{0.5f,  0.5f, -0.5f,1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},1},
	{{0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},1},

	{{-0.5f, -0.5f, -0.5f,1.0f},{  0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f, -0.5f,1.0f}, { 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f, -1.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, {1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f, -1.0f,  0.0f,0.0f},1},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f, -1.0f,  0.0f,0.0f},1},
	{{-0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, {  0.0f, -1.0f,  0.0f,0.0f},1},
	{{-0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},1},

	{{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f, 1.0f,  0.0f,0.0f},1},
	{{-0.5f,  0.5f,  0.5f, 1.0f},  {0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},1},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},1},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},1},
	{{0.5f,  0.5f, -0.5f, 1.0f},  {1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f,1.0f,  0.0f,0.0f},1},
	{{-0.5f,  0.5f, -0.5f, 1.0f},  {0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},1}
};
vertex_t mesh2[36] = {
	// Positions                  // Texture Coords  //color           //rhw // Normals  //index_material
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f,  0.0f,-1.0f,0.0f},2},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f ,1.0f}, 1,{ 0.0f,  0.0f,-1.0f,0.0f},2},
	{{0.5f,  0.5f, -0.5f, 1.0f}, {1.0f,  1.0f}, { 1.0f, 1.0f, 1.0f ,1.0f},1, {0.0f,  0.0f,-1.0f ,0.0f},2},
	{{0.5f,  0.5f, -0.5f, 1.0f}, { 1.0f,  1.0f}, { 1.0f, 1.0f, 1.0f ,1.0f},1, {0.0f,  0.0f,-1.0f,0.0f},2},
	{{0.5f, -0.5f, -0.5f, 1.0f}, {1.0f,  0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, {0.0f,  0.0f,-1.0f ,0.0f},2},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{1.0f, 1.0f, 1.0f, 1.0f }, 1,{ 0.0f,  0.0f,-1.0f,0.0f},2},

	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},2},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},2},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},2},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},2},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},2},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},2},

	{{-0.5f,  0.5f,  0.5f, 1.0f}, { 1.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f}, 1, {-1.0f,  0.0f,  0.0f,0.0f},2},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},2},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1, { -1.0f,  0.0f,  0.0f,0.0f},2},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},2},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f}, 1, { -1.0f,  0.0f,  0.0f,0.0f},2},
	{{-0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},2},

	{{0.5f,  0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, {1.0f, 1.0f, 1.0f, 1.0f }, 1, {1.0f,  0.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, {1.0f, 1.0f, 1.0f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},2},
	{{0.5f,  0.5f, -0.5f,1.0f},{ 1.0f,  1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},2},
	{{0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},2},

	{{-0.5f, -0.5f, -0.5f,1.0f},{  0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f, -0.5f,1.0f}, { 1.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1, { 0.0f, -1.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, {1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f, -1.0f,  0.0f,0.0f},2},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1,{ 0.0f, -1.0f,  0.0f,0.0f},2},
	{{-0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, {  0.0f, -1.0f,  0.0f,0.0f},2},
	{{-0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},2},

	{{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f, 1.0f,  0.0f,0.0f},2},
	{{-0.5f,  0.5f,  0.5f, 1.0f},  {0.0f,  0.0f},{1.0f, 1.0f, 1.0f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},2},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f},{ 1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},2},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f}, { 1.0f, 1.0f, 1.0f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},2},
	{{0.5f,  0.5f, -0.5f, 1.0f},  {1.0f,  1.0f},{ 1.0f, 1.0f, 1.0f, 1.0f }, 1,{ 0.0f,1.0f,  0.0f,0.0f},2},
	{{-0.5f,  0.5f, -0.5f, 1.0f},  {0.0f,  1.0f},{1.0f, 1.0f, 1.0f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},2}
};
void draw_plane(device_t* device,int num,vertex_t *mesh,int count)
{
	vertex_t p1, p2, p3;
	for (int i = 0; i < num; i += 3)
	{
		p1 = mesh[i]; p2 = mesh[i + 1]; p3 = mesh[i + 2];
		int index_m= p1.material_idex;
		device_draw_primitive(device, &p1, &p2, &p3, index_m);
	}
	  
}
void draw_plane(device_t* device, int num, vector<vertex_t>& mesh, int count)
{
	vertex_t p1, p2, p3;
	for (int i = 0; i < num; i += 3)
	{
		p1 = mesh[i]; p2 = mesh[i + 1]; p3 = mesh[i + 2];
		int index_m = p1.material_idex;
		device_draw_primitive(device, &p1, &p2, &p3, index_m);
	}
}
std::vector<vertex_t> tot_vertex;
void draw_box(device_t* device, float theta)
{
	int cnt = 0;
	cnt++;
	s_matrix m;
	m.set_rotate(-1, -0.5, 1, theta);
	device->transform.world = m;
	device->transform.update();
	//六个面 this is the first box 
	
	//设置材质
	
	//device_set_material(device, m_ambient, m_diffuse, m_specular, m_shininess, cnt);


	device->material[cnt].shininess = 64.0f;
	draw_plane(device,36,mesh,cnt);

	//to draw the light box
	cnt++;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(0.0f, 2.0f, 2.0f, 1.0f);
	s_vector scale(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis,1.0f,pos,scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);

	s_vector light_ambient(0.2f, 0.2f, 0.2f,1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device,pos2, light_color,light_ambient,light_diffuse,light_specular,0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36,mesh2,cnt);

	//画obj
	cnt++;
	m.set_rotate_translate_scale(axis, theta, pos, scale);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, tot_vertex.size(), tot_vertex, cnt);
}

void camera_at_zero(device_t* device, s_vector eye, s_vector at, s_vector up)
{
	set_look_at_matrix(device->transform.view, eye, at, up);
	device->transform.update();
}

void init_texture(device_t* device)
{
	static IUINT32 texture[256][256];
	int i, j;
	for (j = 0; j < 256; j++)
	{
		for (i = 0; i < 256; i++)
		{
			int x = i / 32;
			int y = j / 32;
			texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
		}
	}
	device_set_texture(device, texture, 256 * 4, 256, 256);
}
void init_texture_by_photo(device_t* device, char const* path)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	//stbi_image_free(data);
	unsigned bytePerPixel = nrChannels;
	//cout << height << " " << width << endl;

	int n = height; int m = width;

	IUINT32 **texture=NULL;
	texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		texture[i] = new IUINT32[m];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			unsigned char* pixelOffset = data + (x + y * width) * bytePerPixel;

			unsigned char r = pixelOffset[0];
			unsigned char g = pixelOffset[1];
			unsigned char b = pixelOffset[2];
			unsigned char a = nrChannels >= 4 ? pixelOffset[3] : 0xff;
			int rr = (int)r; int gg = (int)g;  int bb = (int)b;  int aa = (int)a;
			int R = rr;
			int G = gg;
			int B = bb;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);
			
			texture[y][x]= (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_photo(device, texture, height * 4, width, height);
	for (int i = 0; i < n; i++)
	{
		delete[] texture[i];
	}
	delete[] texture;
}

void init_texture_by_diffuse(device_t* device, char const* path,int count)
{  //设置漫反射贴图
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	//stbi_image_free(data);
	unsigned bytePerPixel = nrChannels;
	//cout << height << " " << width << endl;
	int n = height; int m = width;

	IUINT32** texture = NULL;
	texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		texture[i] = new IUINT32[m];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			unsigned char* pixelOffset = data + (x + y * width) * bytePerPixel;

			unsigned char r = pixelOffset[0];
			unsigned char g = pixelOffset[1];
			unsigned char b = pixelOffset[2];
			unsigned char a = nrChannels >= 4 ? pixelOffset[3] : 0xff;
			int rr = (int)r; int gg = (int)g;  int bb = (int)b;  int aa = (int)a;
			int R = rr;
			int G = gg;
			int B = bb;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);

			texture[y][x] = (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_diffuse(device, texture, height * 4, width,height,count);

}

void init_texture_by_specular(device_t* device, char const* path, int count)
{  //设置漫反射贴图
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	//stbi_image_free(data);
	unsigned bytePerPixel = nrChannels;
	//cout << height << " " << width << endl;
	int n = height; int m = width;

	IUINT32** texture = NULL;
	texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		texture[i] = new IUINT32[m];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			unsigned char* pixelOffset = data + (x + y * width) * bytePerPixel;

			unsigned char r = pixelOffset[0];
			unsigned char g = pixelOffset[1];
			unsigned char b = pixelOffset[2];
			unsigned char a = nrChannels >= 4 ? pixelOffset[3] : 0xff;
			int rr = (int)r; int gg = (int)g;  int bb = (int)b;  int aa = (int)a;
			int R = rr;
			int G = gg;
			int B = bb;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);

			texture[y][x] = (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_specular(device, texture, height * 4, width, height, count);
}

bool load_obj(std::vector<vertex_t>& tot_vertex, device_t* device, const char* obj_path, const char* pre_mtl_path, int start,bool filp_y)
{
	tinyobj::attrib_t attrib; // 所有的数据放在这里
	std::vector<tinyobj::shape_t> shapes;
	// 一个shape,表示一个部分,
	// 其中主要存的是索引坐标 mesh_t类,
	// 放在indices中
	/*
	// -1 means not used.
	typedef struct {
	  int vertex_index;
	  int normal_index;
	  int texcoord_index;
	} index_t;
	*/
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path,
		pre_mtl_path, 1);

	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
		return false;
	}

	if (!err.empty()) {
		std::cerr << "ERR: " << err << std::endl;
		return false;
	}

	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return false;
	}

	/*	//std::cout << "# of vertices  : " << (attrib.vertices.size()) << std::endl;
		std::cout << "# of normals   : " << (attrib.normals.size() / 3) << std::endl;
		std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2)
			<< std::endl;

		std::cout << "# of shapes    : " << shapes.size() << std::endl;
		std::cout << "# of materials : " << materials.size() << std::endl;
	*/
	int cnt = 0;
	//1.获取各种材质和纹理
	for (int i = 0; i < materials.size(); i++)
	{
		v_material m;
		tinyobj::material_t tm = materials[i];

		m.ambient.x = tm.ambient[0];
		m.ambient.y = tm.ambient[1];
		m.ambient.z = tm.ambient[2];

		m.diffuse.x = tm.diffuse[0];
		m.diffuse.y = tm.diffuse[1];
		m.diffuse.z = tm.diffuse[2];
		//  std::cout<<m.diffuse.x<<" "<<m.diffuse.y<<" "<<m.diffuse.z<<endl;
		m.specular.x = tm.specular[0];
		m.specular.y = tm.specular[1];
		m.specular.z = tm.specular[2];

		// std::cout<<m.specular.x<<" "<<m.specular.y<<" "<<m.specular.z<<endl;

		m.shininess = tm.shininess;
		device->material[start + i].have_diffuse = 0;
		device->material[start + i].have_specular = 0;
		if (tm.diffuse_texname != "") {
			string the_path = tm.diffuse_texname;
			the_path = "model/" + the_path;
			const char* str= the_path.c_str();
			device->material[start + i].have_diffuse = 1;
			init_texture_by_diffuse(device, str, start+i);
			//init_texture_by_specular(&device, "container2_specular.png", 1);
			//m.diffuse_tex_id = make_texture_by_png(m.diffuse_texname, true);
		}
		if (tm.specular_texname !="") {
			string the_path = tm.specular_texname;
			the_path = "model/" + the_path;
			const char* str = the_path.c_str();
			device->material[start + i].have_specular = 1;
			init_texture_by_specular(device,str, start + i);
		}
		
		device->material[start + i].shininess = tm.shininess;
		
		/*m->ambient_tex_id = -1;
		m->diffuse_tex_id = -1;
		m->specular_tex_id = -1;

		m->alpha_tex_id = -1;
		*/



		device->v_m_num++;
		device->vmaterial[device->v_m_num] = m;
	}




	// For each shape 遍历每一个部分
	for (size_t i = 0; i < shapes.size(); i++)
	{
		// 这部分的名称
		//printf("shape[%ld].name = %s\n", static_cast<long>(i),
		//	shapes[i].name.c_str());
		// 网格的点数
	//	printf("Size of shape[%ld].mesh.indices: %lu\n", static_cast<long>(i),
	//		static_cast<unsigned long>(shapes[i].mesh.indices.size()));
		//printf("Size of shape[%ld].path.indices: %lu\n", static_cast<long>(i),static_cast<unsigned long>(shapes[i].path.indices.size()));

		//assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());
		//assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.smoothing_group_ids.size());

	//	printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
	//		static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));
	  // std::cout<<"the num "<<i<<"  is  "<<shapes[i].mesh.num_face_vertices.size()<<" "<<shapes[i].mesh.material_ids.size()<<endl;

		int index_offset = 0;

		// For each face
		for (int f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			int fnum = shapes[i].mesh.num_face_vertices[f];
			/*if(f==0||f==1||f==3)
			{  //std::cout<<"fum "<<fnum<<endl;
			  std::cout<<shapes[i].mesh.material_ids[f]<<endl;
			}*/
			// if(fnum!=3) {std::cout<<"here!\n";}
			int m_index = shapes[i].mesh.material_ids[f];
			//if(m_index==0) {std::cout<<"here!\n";}
		   // 获得所索引下标
			tinyobj::index_t idx;
			int vertex_index[3];
			int normal_index[3];
			int texcoord_index[3];
			for (int v = 0; v < fnum; v++) {
				idx = shapes[i].mesh.indices[index_offset + v];
				vertex_index[v] = idx.vertex_index;
				texcoord_index[v] = idx.texcoord_index;
				normal_index[v] = idx.normal_index;
			}
			for (int v = 0; v < fnum; v++) {
				// v
				vertex_t mesh_data;
				mesh_data.pos.y = attrib.vertices[(vertex_index[v]) * 3 + 0];
				if(filp_y==1)
				mesh_data.pos.z = -attrib.vertices[(vertex_index[v]) * 3 + 1];
				else mesh_data.pos.z = attrib.vertices[(vertex_index[v]) * 3 + 1];
				mesh_data.pos.x = attrib.vertices[(vertex_index[v]) * 3 + 2];
				mesh_data.pos.w = 1.0f;

				// vt
				mesh_data.tc.u =(float) attrib.texcoords[texcoord_index[v] * 2 + 0];
				mesh_data.tc.v =(float) attrib.texcoords[texcoord_index[v] * 2 + 1];

				// vn
				mesh_data.normal.x = attrib.normals[normal_index[v] * 3 + 0];
				mesh_data.normal.y = attrib.normals[normal_index[v] * 3 + 1];
				mesh_data.normal.z = attrib.normals[normal_index[v] * 3 + 2];
				mesh_data.normal.w = 0.0f;

				// color
				mesh_data.color.r = 1.0f;
				mesh_data.color.g = 1.0f;
				mesh_data.color.b = 1.0f;
				mesh_data.color.a = 1.0f;

				mesh_data.rhw = 1.0f;
				mesh_data.material_idex = start + m_index;
				tot_vertex.push_back(mesh_data);
				cnt++;
			}

			// 偏移
			index_offset += fnum;
		}

	}


	std::cout << "# Loading Complete #" << std::endl;

	//std::cout <<cnt<<std::endl;
	//PrintInfo(attrib, shapes, materials);
	
	return true;
	

}



float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{

	device_t device;
	int states[] = { RENDER_STATE_TEXTURE,RENDER_STATE_COLOR,RENDER_STATE_WIREFRAME };
	int indicator = 0;
	int kbhit = 0;
	float alpha = 1;
	float pos = 3.5;

	TCHAR tt[] = _T("GOGOGO");
	TCHAR* title = tt;


	if (screen_init(800, 600, title)) return -1;

	device_init(&device, 800, 600, screen_fb);
	s_vector eye(2.5f,0.4f, 10.0f, 1.0f), at(-1.0f, 0.0f, 0.0f, 1.0f), up(0.0f, 0.0f, 1.0f, 1.0f);
	device.camera.viewpos = eye;
	/* glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
 glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
 glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);*/
	camera_at_zero(&device, eye, at, up);

	//表示1号物体所用的材质信息。
	device.material[1].have_diffuse = 1;
	init_texture_by_diffuse(&device, "photo/container2.png",1);
	device.material[1].have_specular = 1;
	init_texture_by_specular(&device, "photo/container2_specular.png", 1);

	//init_texture_by_diffuse(&device, "model/arm_dif.png", 3);

	//加载obj的mesh

	load_obj(tot_vertex, &device, "model/nanosuit.obj", "model", 3,1);
	//init_texture_by_photo(&device, "container2.png");
	device.render_state = RENDER_STATE_TEXTURE;

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
	{
		screen_dispatch();
		device_clear(&device, 1);

		//point_t eye = { x,y,z,1 }, at = { 0,0,0,1 }, up = { 0,0,1,1 };
		s_vector attemp;
		attemp.add_two(eye, at);
		if (mousechange == true)
		{
			mousechange = false;
			at = front;
		}

	
		camera_at_zero(&device, eye, attemp, up);


		clock_t nowtime = clock();
		float currentFrame = (float)nowtime / CLOCKS_PER_SEC;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float cameraSpeed = 2.7f * deltaTime;
		s_vector att;
		s_vector temp = { cameraSpeed,cameraSpeed,cameraSpeed,1.0f };
		att.dot_two(temp, at);
		if (screen_keys[VK_UP]) { eye.add_two(eye, att);/* eye += att;/*vector_add(&eye, &eye, &att);*/ }
		if (screen_keys[VK_DOWN]) { eye.minus_two(eye, att); /*eye -= att; /*vector_sub(&eye, &eye, &att); */ }

		s_vector the_y;
		the_y.crossproduct(at, up);
		the_y.normalize();
		the_y.dot_two(the_y, temp);
		if (screen_keys[VK_LEFT]) { eye.add_two(eye, the_y);/*eye += the_y;/* vector_add(&eye, &eye, &the_y);*/ }
		if (screen_keys[VK_RIGHT]) { eye.minus_two(eye, the_y);/*eye += the_y; /*vector_sub(&eye, &eye, &the_y);*/ }
		if (screen_keys[VK_F1]) { alpha += 0.05f; }
		if (screen_keys[VK_F2]) { alpha -= 0.05f; }

		if (screen_keys[VK_SPACE])
		{
			if (kbhit == 0)
			{
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_state = states[indicator];
			}
		}
		else
		{
			kbhit = 0;
		}

		draw_box(&device, alpha);
		screen_update();
		Sleep(1);
	}
	return 0;
}