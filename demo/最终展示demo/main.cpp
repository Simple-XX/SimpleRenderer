

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
s_vector front;
float theta_ = 0.0f;
float m1 = 0.0f;
float m2 = 0.0f;
s_vector eye(2.5f, 0.4f, 10.0f, 1.0f), at(-1.0f, 0.0f, 0.0f, 1.0f), up(0.0f, 0.0f, 1.0f, 1.0f);
static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_LBUTTONDOWN:
	{  //printf("%lf %lf\n", me, rou);
		//printf("\n%lf\n", theta_);
	//	break;
	//	eye.show();
	//	s_vector temp; temp.add_two(eye, at); temp.show();
	//	at.show();
	//	up.show();
	//	printf("\n");
	}
	case WM_MOUSEMOVE:
	{   
		float xpos = LOWORD(lParam);
	    float ypos = HIWORD(lParam);
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		else
		{

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos;
			lastX = xpos;
			lastY = ypos;
			float sensitivity = 0.05f;
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
			//at = front;
		}
		
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
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},19},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},19},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},19},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},19},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},19},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},19}



};

vertex_t mesh_window[6] =
{
		{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},18},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},18},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},18},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},18},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},18},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},18}


};

vertex_t mesh_bump[36] = {
	// Positions                  // Texture Coords  //color           //rhw // Normals
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,-1.0f,0.0f},17},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f ,1.0f}, 1,{ 0.0f,  0.0f,-1.0f,0.0f},17},
	{{0.5f,  0.5f, -0.5f, 1.0f}, {1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f ,1.0f},1, {0.0f,  0.0f,-1.0f ,0.0f},17},
	{{0.5f,  0.5f, -0.5f, 1.0f}, { 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f ,1.0f},1, {0.0f,  0.0f,-1.0f,0.0f},17},
	{{0.5f, -0.5f, -0.5f, 1.0f}, {1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,-1.0f ,0.0f},17},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f,  0.0f,-1.0f,0.0f},17},

	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f, 1.0f,0.0f},17},
	{{0.5f, -0.5f,  0.5f, 1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, {0.0f,  0.0f,  1.0f,0.0f},17},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},17},
	{{0.5f,  0.5f,  0.5f, 1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {0.0f,  0.0f,  1.0f,0.0f},17},
	{{-0.5f,  0.5f,  0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,  1.0f,0.0f},17},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f,  0.0f,  1.0f,0.0f},17},

	{{-0.5f,  0.5f,  0.5f, 1.0f}, { 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f}, 1, {-1.0f,  0.0f,  0.0f,0.0f},17},
	{{-0.5f,  0.5f, -0.5f, 1.0f},{ 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},17},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { -1.0f,  0.0f,  0.0f,0.0f},17},
	{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},17},
	{{-0.5f, -0.5f,  0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f}, 1, { -1.0f,  0.0f,  0.0f,0.0f},17},
	{{-0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { -1.0f,  0.0f,  0.0f,0.0f},17},

	{{0.5f,  0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f }, 1, {1.0f,  0.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},17},
	{{0.5f,  0.5f, -0.5f,1.0f},{ 1.0f,  1.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},17},
	{{0.5f,  0.5f,  0.5f,1.0f},{ 1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 1.0f,  0.0f,  0.0f,0.0f},17},

	{{-0.5f, -0.5f, -0.5f,1.0f},{  0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f, -0.5f,1.0f}, { 1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, { 0.0f, -1.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f}, {1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f, -1.0f,  0.0f,0.0f},17},
	{{0.5f, -0.5f,  0.5f,1.0f}, { 1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f, -1.0f,  0.0f,0.0f},17},
	{{-0.5f, -0.5f,  0.5f,1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, {  0.0f, -1.0f,  0.0f,0.0f},17},
	{{-0.5f, -0.5f, -0.5f,1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1, {  0.0f, -1.0f,  0.0f,0.0f},17},

	{{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f, 1.0f,  0.0f,0.0f},17},
	{{-0.5f,  0.5f,  0.5f, 1.0f},  {0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},17},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},17},
	{{0.5f,  0.5f,  0.5f, 1.0f},  {1.0f,  0.0f}, { 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,1.0f,  0.0f,0.0f},17},
	{{0.5f,  0.5f, -0.5f, 1.0f},  {1.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f }, 1,{ 0.0f,1.0f,  0.0f,0.0f},17},
	{{-0.5f,  0.5f, -0.5f, 1.0f},  {0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1,{ 0.0f, 1.0f,  0.0f,0.0f},17}
};

std::vector<vertex_t> tot_vertex;
std::vector<vertex_t> tot_data;
std::vector<int> indices;

std::vector<vertex_t> tot_vertex2;
std::vector<vertex_t> tot_data2;
std::vector<int> indices2;
void draw_box(device_t* device, float theta)
{
	int now_num = device->now_state;
	if (now_num == 7)
	{
		int cnt = 0;

		cnt++;
		s_matrix m;
		s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
		s_vector pos(2.0f, 1.0f, 0.0f, 1.0f);
		s_vector scale(0.6f, 0.6f, 0.6f, 1.0f);
		//m.set_rotate_translate_scale(axis, theta, pos, scale);
		m.set_rotate_translate_scale(axis, theta, pos, scale);
		device->transform.world = m;
		device->transform.update();

		device->material[20].shininess = 32.0f;
		device->PBR.ao = 1.0f;
		

		draw_plane_STRIP(device, tot_data, indices, cnt);
		//draw_plane(device,tot_data.size(), tot_data,cnt);

		//to draw the light box
		cnt++;
		axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
		pos.reset(0.0f, 2.0f, 2.0f, 1.0f);
		scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
		m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
		s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector pos2;
		apply_to_vector(pos2, rightpos, m);
		s_vector light_color(m1, m1, m1, m1);

		s_vector light_ambient(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

		device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
		device->transform.world = m;
		device->transform.update();
		draw_plane(device, 36, mesh2, cnt);
	}
	else if (now_num == 1)
	{
		int cnt = 0;
		cnt++;
		s_matrix m;
		s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
		s_vector pos(2.0f, 0.0f, 0.0f, 1.0f);
		s_vector scale(1.0f, 1.0f, 1.0f, 1.0f);
		m.set_rotate_translate_scale(axis, theta, pos, scale);
	//	m.set_rotate(-1, -0.5, 1, theta);
		device->transform.world = m;
		device->transform.update();
		//六个面 this is the first box 
		device->m1 = m1;
		device->m2 = m2;
		draw_plane(device, 36, mesh, cnt);

		//to draw the light box
		cnt++;
		axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
		pos.reset(2.3f, 1.0f, 1.0f, 1.0f);
		scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
		m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
		s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector pos2;
		apply_to_vector(pos2, rightpos, m);
		s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);
		device_set_pointlight(device, pos2, light_color, 0);
		device->transform.world = m;
		device->transform.update();
		draw_plane(device, 36, mesh2, cnt);
	}
	else if (now_num == 2)
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
		draw_plane(device, 36, mesh, cnt);

		//to draw the light box
		cnt++;
		s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
		s_vector pos(0.0f, 2.0f, 2.0f, 1.0f);
		s_vector scale(0.2f, 0.2f, 0.2f, 1.0f);
		m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
		s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector pos2;
		apply_to_vector(pos2, rightpos, m);
		s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);

		s_vector light_ambient(0.2f, 0.2f, 0.2f, 1.0f);
		s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
		s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

		device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
		device->transform.world = m;
		device->transform.update();
		draw_plane(device, 36, mesh2, cnt);
	}
	else if (now_num == 3)
	{
	int cnt = 0;
	cnt++;
	s_matrix m;


	//to draw the light box
	cnt++;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(4.0f, 2.0f, 2.0f, 1.0f);
	s_vector scale(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);

	s_vector light_ambient(m1,m1, m1, 1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36, mesh2, cnt);

	axis.reset(1.0f, 0.0f, 0.0f, 1.0f);
	pos.reset(0.0f, 1.5f, 0.0f, 1.0f);
	scale.reset(0.3f, 0.3f, 0.3f, 1.0f);

	//画obj
	cnt++;
	m.set_rotate_translate_scale(axis, 3.5f, pos, scale);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, tot_vertex.size(), tot_vertex, cnt);
    }
	else if (now_num == 4)
	{
	int cnt = 0;

	cnt++;
	s_matrix m;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(2.0f, 0.0f, 0.0f, 1.0f);
	s_vector scale(1.0f, 1.0f, 1.0f, 1.0f);
	//m.set_rotate_translate_scale(axis, theta, pos, scale);
	m.set_rotate(-1, -0.5, 1, 2.44f);
	m.set_rotate_translate_scale(axis, 2.44f, pos, scale);
	device->transform.world = m;
	device->transform.update();
	//六个面 this is the first box 

	//设置材质

	//device_set_material(device, m_ambient, m_diffuse, m_specular, m_shininess, cnt);


	device->material[cnt].shininess = 64.0f;
	draw_plane(device, 36, mesh, cnt);

	//to draw the light box
	cnt++;
	axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
	pos.reset(2.3f, 2.0f, 2.0f, 1.0f);
	scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);

	s_vector light_ambient(0.2f, 0.2f, 0.2f, 1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36, mesh2, cnt);

	// 画草
	cnt++;
	axis.reset(1.0f, 0.0f, 0.0f, 1.0f);
	pos.reset(1.0f, 1.0f + m1, -1.0f, 1.0f);
	scale.reset(1.0f, 1.0f, 1.0f, 1.0f);
	m.set_rotate_translate_scale(axis, -0.30f, pos, scale);

	//theta_ = theta;
	device->transform.world = m;
	device->transform.update();
	device->material[19].shininess = 16.0f;
	draw_plane(device, 6, mesh_grass, cnt);
	//画obj
	/*
	cnt++;
	m.set_rotate_translate_scale(axis, theta, pos, scale);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, tot_vertex.size(), tot_vertex, cnt);
	*/

	//画窗子
	cnt++;
	axis.reset(1.0f, 0.0f, 0.0f, 1.0f);
	pos.reset(1.0f, 1.0f, 0.0f, 1.0f);
	scale.reset(1.0f, 1.0f, 1.0f, 1.0f);
	m.set_rotate_translate_scale(axis, -0.30f, pos, scale);
	//theta_ = theta;
	device->transform.world = m;
	device->transform.update();
	device->material[18].shininess = 16.0f;
	draw_plane(device, 6, mesh_window, cnt);
    }
	else if (now_num == 5)
	{
	int cnt = 0;

	cnt++;
	s_matrix m;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(2.0f, 1.0f, 0.0f, 1.0f);
	s_vector scale(1.5f, 1.5f, 1.5f, 1.5f);
	//m.set_rotate_translate_scale(axis, theta, pos, scale);
	m.set_rotate(-1, -0.5, 1, theta);
	device->transform.world = m;
	device->transform.update();
	//六个面 this is the first box 

	//设置材质

	//device_set_material(device, m_ambient, m_diffuse, m_specular, m_shininess, cnt);


	device->material[17].shininess = 32.0f;
	draw_plane(device, 36, mesh_bump, cnt);

	//to draw the light box
	cnt++;
	axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
	pos.reset(0.0f, 2.0f, 2.0f, 1.0f);
	scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(1.0f, 1.0f, 1.0f, 1.0f);

	s_vector light_ambient(0.2f, 0.2f, 0.2f, 1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36, mesh2, cnt);
    }
	else if (now_num == 6)
	{
	int cnt = 0;

	cnt++;
	s_matrix m;
	s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f);
	s_vector pos(2.0f, 1.0f, 0.0f, 1.0f);
	s_vector scale(0.6f, 0.6f, 0.6f, 1.0f);
	//m.set_rotate_translate_scale(axis, theta, pos, scale);
	m.set_rotate_translate_scale(axis, theta, pos, scale);
	device->transform.world = m;
	device->transform.update();

	device->material[17].shininess = 32.0f;

	device->PBR.albedo.x = 0.5f; device->PBR.albedo.y = 0.0f; device->PBR.albedo.z = 0.0f; device->PBR.albedo.w = 1.0f;
	device->PBR.ao = 1.0f;
	device->PBR.metallic = m1 / 7.0f;
	device->PBR.roughness = m2 / 7.0f;

	draw_plane_STRIP(device, tot_data2, indices2, cnt);
	//draw_plane(device,tot_data.size(), tot_data,cnt);

	//to draw the light box
	cnt++;
	axis.reset(-1.0f, -0.5f, 1.0f, 1.0f);
	pos.reset(0.0f, 2.0f, 2.0f, 1.0f);
	scale.reset(0.2f, 0.2f, 0.2f, 1.0f);
	m.set_rotate_translate_scale(axis, 1.0f, pos, scale);
	s_vector rightpos(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector pos2;
	apply_to_vector(pos2, rightpos, m);
	s_vector light_color(1.18f,1.18f, 1.18f, 1.0f);

	s_vector light_ambient(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_diffuse(0.5f, 0.5f, 0.5f, 1.0f);
	s_vector light_specular(1.0f, 1.0f, 1.0f, 1.0f);

	device_set_pointlight(device, pos2, light_color, light_ambient, light_diffuse, light_specular, 0);
	device->transform.world = m;
	device->transform.update();
	draw_plane(device, 36, mesh2, cnt);

    }
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
			m.material_idex = 20;
			m.rhw = 1;
			m.color.r = 0.5f; m.color.g = 0.0f; m.color.b = 0.0f; m.color.a = 1.0f;
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
void get_the_ball2(std::vector<vertex_t>& tot_data, std::vector<int>& indices)
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
			m.material_idex = 16;
			m.rhw = 1;
			m.color.r = 0.5f; m.color.g = 0.0f; m.color.b = 0.0f; m.color.a = 1.0f;
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
	get_the_ball(tot_data, indices);
	get_the_ball2(tot_data2, indices2);
	device_t device;
	int states[] = { RENDER_STATE_TEXTURE,RENDER_STATE_WIREFRAME };
	int indicator = 0;
	int kbhit = 0;
	float alpha = 1;
	float pos = 3.5;

	TCHAR tt[] = _T("GOGOGO");
	TCHAR* title = tt;


	if (screen_init(800, 600, title)) return -1;

	device_init(&device, 800, 600, screen_fb);
	
	device.camera.viewpos = eye;
	/* glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
 glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
 glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);*/
	s_vector the_temp; the_temp.add_two(eye, at);
	eye.reset(2.472f, 0.378f, 10.346f, 1.0f);
	at.reset(0.03139f, 0.0296f, -0.999f, 1.0f);
	up.reset(0.0f, 0.0f, 1.0f, 1.0f);
	camera_at_zero(&device, eye, the_temp, up);

	
	printf("方向键来实现移动\n");
	printf("空格键切换到线框模式\n");
	printf("按shift键来改变状态  初始状态是1\n");
	printf("--------------------------------------------------\n");
	printf("1.手动调参数的phong光照部分\n");
	printf("按F1,F2来旋转立方体\n");

	printf("按F3,F4 来调节环境光照\n");
	printf("按F5,F6 来调节镜面高光(请先旋转立方体以找到高光)\n");
	printf("Loading,Please wait\n");
		device.now_state = 1;
		m1 = 0.01f; m2 = 1.0f;  alpha = 1.0f;
		//表示1号物体所用的材质信息。
		
		
			device.material[20].have_diffuse = 0;
			//init_texture_by_diffuse(&device, "photo/brickwall.jpg",1);
			device.material[20].have_specular = 0;
			//init_texture_by_specular(&device, "photo/container2_specular.png", 1);
			device.material[20].have_normal = 1;

			init_texture_by_normal(&device, "photo/rustediron2_normal.png", 20);
			init_texture_by_albedo(&device, "photo/rustediron2_basecolor.png", 20);
			init_texture_by_metallic(&device, "photo/rustediron2_metallic.png", 20);
			init_texture_by_roughness(&device, "photo/rustediron2_roughness.png", 20);
		

			//表示1号物体所用的材质信息。
			device.material[1].have_diffuse = 1;
			init_texture_by_diffuse(&device, "photo/container2.png", 1);
			device.material[1].have_specular = 1;
			init_texture_by_specular(&device, "photo/container2_specular.png", 1);
		//init_texture_by_normal(&device, "photo/brickwall_normal.jpg", 1);


			device.material[19].have_diffuse = 1;
			init_texture_by_diffuse(&device, "photo/grass.png", 19);
			device.material[19].have_specular = 0;

			device.material[18].have_diffuse = 1;
			init_texture_by_diffuse(&device, "photo/blending_transparent_window.png", 18);
			device.material[18].have_specular = 0;

			//表示1号物体所用的材质信息。
			device.material[17].have_diffuse = 1;
			init_texture_by_diffuse(&device, "photo/brickwall.jpg", 17);
			device.material[17].have_specular = 0;
			//init_texture_by_specular(&device, "photo/container2_specular.png", 1);
			device.material[17].have_normal = 1;

			init_texture_by_normal(&device, "photo/brickwall_normal.jpg", 17);


			//表示1号物体所用的材质信息。
			device.material[16].have_diffuse = 0;
			//init_texture_by_diffuse(&device, "photo/brickwall.jpg",1);
			device.material[16].have_specular = 0;
			//init_texture_by_specular(&device, "photo/container2_specular.png", 1);
			device.material[16].have_normal = 0;

			load_obj(tot_vertex, &device, "model/nanosuit.obj", "model", 3, 1);

		//init_texture_by_diffuse(&device, "model/arm_dif.png", 3);
			// if you wanna use the obj,please change the number of 3,as it means that it's the 3rd object.
		//加载obj的mesh

		//load_obj(tot_vertex, &device, "model/nanosuit.obj", "model", 3,1);
		//init_texture_by_photo(&device, "container2.png");
		device.render_state = RENDER_STATE_TEXTURE;
		int hitchange = 0;  int hit2 = 0;
		while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)
		{
			screen_dispatch();
			device_clear(&device, 1);

			//point_t eye = { x,y,z,1 }, at = { 0,0,0,1 }, up = { 0,0,1,1 };
			s_vector attemp;
			attemp.add_two(eye, at);
			/*if (mousechange == true)
			{
				mousechange = false;
				at = front;
			}*/


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
			if (screen_keys[VK_F1]) { alpha += 0.02f;  }
			if (screen_keys[VK_F2]) { alpha -= 0.02f; }

			if (screen_keys[VK_F3])
			{
				if (device.now_state == 6)
				{
					m1 += 0.25f; if (m1 > 7.0f) m1 = 7.0f;
				}
				else if (device.now_state == 1)
				{
					m1 += 0.01f; if (m1 > 0.3f) m1 = 0.3f;
				}
				else if (device.now_state == 7)
				{
					m1 += 0.05f; if (m1 > 3.0f) m1 = 3.0f;
				}
				else if (device.now_state == 3)
				{
					m1 += 0.05f; if (m1 > 1.0f) m1 = 1.0f;
				}
				else if (device.now_state == 4)
				{
					m1 += 0.025f; if (m1 > 1.0f) m1 = 1.0f;
				}
				else if (device.now_state == 5)
				{
					if (hit2 == 0)
					{
						hit2 = 1;
						if (device.material[17].have_normal == 1) device.material[17].have_normal = 0;
						else device.material[17].have_normal = 1;
					}
					
				}
				else if (device.now_state == 6)
				{
					m1 += 0.5f; if (m1 > 7.0f) m1 = 7.0f;
				}
			}
			else
			{
				if (device.now_state == 5)
				hit2 = 0;
			
			}
			if (screen_keys[VK_F4])
			{
				if (device.now_state == 6)
				{
					m1 -= 0.25f; if (m1 < 0.0f) m1 = 0.0f;
				}
				else if (device.now_state == 1)
				{
					m1 -= 0.01f; if (m1 < 0.01f) m1 = 0.01f;
				}
				else if (device.now_state == 7)
				{
					m1 -= 0.05f; if (m1 < 0.38f) m1 = 0.38f;
				}
				else if (device.now_state == 3)
				{
					m1 -= 0.05f; if (m1 < 0.1f) m1 = 0.1f;
				}
				else if (device.now_state == 4)
				{
					m1 -= 0.025f; if (m1 < -0.1f) m1 = -0.1f;
				}
				else if (device.now_state == 6)
				{
					m1 -= 0.25f; if (m1 < 0.0f) m1 = 0.0f;
				}
			}


			if(screen_keys[VK_F5])
			{ 
				if (device.now_state == 6)
				{
					m2 += 0.25f; if (m2 > 7.0f) m2 = 7.0f;
				}
				else if (device.now_state == 1)
				{
					m2 += 0.05f; if (m2 > 1.0f) m2 = 1.0f;
				}
				else if (device.now_state == 4)
				{
					if (hit2 == 0)
					{
						hit2 = 1;
						if (device.is_cull == 1) device.is_cull = 2;
						else device.is_cull = 1;
					}
					
				}
				else if (device.now_state == 6)
				{
					m2 += 0.5f; if (m2 > 7.0f) m2 = 7.0f;
				}
			}
			else
			{
				if (device.now_state == 4)
				hit2 = 0;
			
			}
			if(screen_keys[VK_F6]) 
			{
				if (device.now_state == 6)
				{
					m2 -= 0.25f; if (m2 < 0.0f)  m2 = 0.0f;
				}
				else if (device.now_state == 1)
				{
					m2 -= 0.05f; if (m2 < 0.1f) m2 = 0.1f;
				}
				else if (device.now_state == 6)
				{
					m2 -= 0.25f; if (m2 < 0.0f) m2 = 0.0f;
				}
			}
			if (screen_keys[VK_SHIFT])
			{
				if (hitchange == 0)
				{
					hitchange = 1;
					device.now_state++;
					if (device.now_state > 7) device.now_state = 1;
					int the_num = device.now_state;
					if (the_num == 1) 
					{
						hit2 = 0;
						device.is_cull = 1;
						eye.reset(2.472f, 0.378f, 10.346f, 1.0f);
						at.reset(0.03139f, 0.0296f, -0.999f, 1.0f);

						m1 = 0.01f; m2 = 1.0f;  alpha = 1.0f;
					
					printf("--------------------------------------------------\n"); 
					printf("1.手动调参数的phong光照部分\n"); 
					printf("按F1,F2来旋转立方体\n");
				
					printf("按F3,F4 来调节环境光照\n");
					printf("按F5,F6 来调节镜面高光(请先旋转立方体以找到高光)\n");
					}
					else if (the_num == 2) 
					{
						hit2 = 0;
						device.is_cull = 1;
						eye.reset(0.6189f, 0.748992, 10.3465, 1.0f);
						at.reset(0.041852,0.0314f,-0.9986f,1.0f);
						alpha = 1.0f; printf("--------------------------------------------------\n"); 
						printf("2.漫反射贴图，镜面高光贴图的材质部分\n"); 
						printf("按F1,F2来旋转立方体\n");
					}
					else if (the_num == 3) 
					{
						hit2 = 0;
						device.is_cull = 1;
						eye.reset(0.54118f, 1.43f, 9.918f, 1.0f);
						at.reset(0.2888f, 0.0314f, -0.9568f, 1.0f);
						alpha = 3.5f; m1 = 0.2f; printf("--------------------------------------------------\n"); printf("3.obj +光照的实例\n");  printf("按F3,F4来调节环境光强度\n");}
					else if (the_num == 4) 
					{
						hit2 = 0;
						eye.reset(0.8868f, 1.471f, 8.4735f, 1.0f);
						at.reset(0.228298f, -0.020942f, -0.97336f, 1.0f);
						device.is_cull = 1;
						alpha = 1.0f; m1 = 0.0f;  printf("--------------------------------------------------\n"); printf("4.混合的展示， 面剔除的展示\n"); printf("按F3,F4来移动小草\n"); printf("按F5来转换剔除的面\n"); }
					else if (the_num == 5)
					{
						hit2 = 0;
						device.is_cull = 1;
						eye.reset(-0.093224f, 1.51399f, 5.7037f, 1.0f);
						at.reset(0.020937f, 0.015707f, -0.999657f, 1.0f);
						alpha = 2.3f; printf("--------------------------------------------------\n"); printf("5.bump的展示\n"); printf("按F1,F2来旋转立方体\n"); printf("按F3来看bump之前的立方体\n"); }
					else if (the_num == 6) 
					{
						hit2 = 0;
						device.is_cull = 1;
						eye.reset(1.1584f, 0.03366f, 8.6363f, 1.0f);
						at.reset(-0.04627f, 0.187f, -0.98f, 1.0f);
						alpha = 1.0f;  printf("--------------------------------------------------\n"); m1 = 6.5f; m2 = 2.0f; printf("6.pbr手动调参数的展示\n"); printf("按F3,F4来调节金属度\n"); printf("按F5,F6来调节粗糙度\n"); }
					else if (the_num == 7)
					{
						hit2 = 0;
						device.is_cull = 1;
						m1 = 0.78f;
						eye.reset(0.557345f, -0.021029f, 9.1009f, 1.0f);
						at.reset(-0.000122f, 0.000541, -1.0f, 1.0f);
						


						printf("--------------------------------------------------\n");
						printf("7.pbr应用材质后的展示\n"); printf("按F1,F2来旋转球体\n");
						printf("按F3,F4来调节光照强度\n");
					}
				}
			}
			else
			{ 
				hitchange = 0;
			}
			if (screen_keys[VK_SPACE])
			{
				if (kbhit == 0)
				{
					kbhit = 1;
					if (++indicator >= 2) indicator = 0;
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