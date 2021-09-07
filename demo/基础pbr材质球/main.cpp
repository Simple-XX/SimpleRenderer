

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
float pos_uniform = 0.0f;

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
float theta_ = 0.0f;
static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_LBUTTONDOWN:
	{
		//printf("\n%lf\n", theta_);
	//	break;
	}
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

vertex_t mesh_grass[6] =
{
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},3},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},3},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},3},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},3},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},3},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},3}



};

vertex_t mesh_window[6] =
{
		{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},4},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},4},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},4},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},4},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},4},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},4}


};



std::vector<vertex_t> tot_vertex;
std::vector<vertex_t> tot_data;
std::vector<int> indices;
void draw_box(device_t* device, float theta)
{
	int cnt = 0;
	
	cnt++;
	s_matrix m;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(2.0f, 1.0f+pos_uniform, 0.0f, 1.0f);
	s_vector scale(0.6f, 0.6f, 0.6f, 1.0f);
	//m.set_rotate_translate_scale(axis, theta, pos, scale);
	m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
	device->transform.world = m;
	device->transform.update();

	device->material[cnt].shininess = 32.0f;

	device->PBR.albedo.x = 0.5f; device->PBR.albedo.y = 0.0f; device->PBR.albedo.z = 0.0f; device->PBR.albedo.w = 1.0f;
	device->PBR.ao = 1.0f;
	device->PBR.metallic = 1.0f / 7.0f;
	device->PBR.roughness = 1.0f / 7.0f;

	draw_plane_STRIP(device, tot_data, indices, cnt);
	//draw_plane(device,tot_data.size(), tot_data,cnt);

	//to draw the light box
	cnt++;
	axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
	pos.reset(0.0f, 2.0f, 2.0f, 1.0f);
	scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis,1.0f,pos,scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(300.0f, 300.0f, 300.0f, 1.0f);

	s_vector light_ambient(0.5f, 0.5f, 0.5f,1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device,pos2, light_color,light_ambient,light_diffuse,light_specular,0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36,mesh2,cnt);

}

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void get_the_ball(std::vector<vertex_t>& tot_data,std::vector<int>& indices)
{
	const unsigned int X_SEGMENTS = 64;
	const unsigned int Y_SEGMENTS = 64;
	const float PI = 3.14159265359;
	for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
	{
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			vertex_t m;
			m.pos.x = xPos; m.pos.y = yPos;  m.pos.z = zPos; m.pos.w = 1.0f;
			m.tc.u = xSegment; m.tc.v = ySegment;
			m.normal.x = xPos; m.normal.y = yPos; m.normal.z = zPos;
			m.material_idex = 1;
			m.rhw = 1;
			m.color.r = 0.6f; m.color.g = 0.0f; m.color.b = 0.0f; m.color.a = 1.0f;
			//positions.push_back(glm::vec3(xPos, yPos, zPos));
			//uv.push_back(glm::vec2(xSegment, ySegment));
		//	normals.push_back(glm::vec3(xPos, yPos, zPos));
			tot_data.push_back(m);
			
		}
	}
	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
			}
		}
		else
		{
			for (int x = X_SEGMENTS; x >= 0; --x)
			{
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				indices.push_back(y * (X_SEGMENTS + 1) + x);
			}
		}
		oddRow = !oddRow;
	}
		
}

int main()
{ 
	get_the_ball(tot_data,indices);
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
	device.material[1].have_diffuse = 0;
	//init_texture_by_diffuse(&device, "photo/brickwall.jpg",1);
	device.material[1].have_specular = 0;
	//init_texture_by_specular(&device, "photo/container2_specular.png", 1);
	device.material[1].have_normal = 0;

	//init_texture_by_normal(&device, "photo/brickwall_normal.jpg", 1);
	

	

	//init_texture_by_diffuse(&device, "model/arm_dif.png", 3);
		// if you wanna use the obj,please change the number of 3,as it means that it's the 3rd object.
	//加载obj的mesh

	//load_obj(tot_vertex, &device, "model/nanosuit.obj", "model", 3,1);
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

		if (screen_keys[VK_F3]) { pos_uniform+= 0.05f; }
		if (screen_keys[VK_F4]) { pos_uniform -= 0.05f; }
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