//=====================================================================
// 渲染设备
//=====================================================================
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <cstring>
#include<cstdio>
#include<algorithm>
#include<cmath>
#include<iostream>
#include <cassert>
#include "s_math.h"
#include "rasterize.h"
#include "render.h"
using namespace std;
//设备初始化，fb为外部帧缓存，非NULL将引用外部帧缓存 
void device_init(device_t* device, int width, int height, void* fb)
{
	int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
	char* ptr = (char*)malloc(need + 64);
	char* framebuf, * zbuf;
	int j;
	assert(ptr);
	device->v_m_num = 0;
	device->is_cull = 1;
	device->framebuffer = (IUINT32**)ptr;
	device->zbuffer = (float**)(ptr + sizeof(void*) * height);
	ptr += sizeof(void*) * height * 2;
	device->texture_di = (IUINT32**)ptr;
	device->texture = (IUINT32**)ptr;
	ptr += sizeof(void*) * 1024;
	framebuf = (char*)ptr;
	zbuf = (char*)ptr + width * height * 4;
	ptr += width * height * 8;
	if (fb != NULL) framebuf = (char*)fb;
	for (j = 0; j < height; j++)
	{
		device->framebuffer[j] = (IUINT32*)(framebuf + width * 4 * j);
		device->zbuffer[j] = (float*)(zbuf + width * 4 * j);
	}
	device->texture_di[0] = (IUINT32*)ptr;
	device->texture_di[1] = (IUINT32*)(ptr + 16);
	device->texture[0] = (IUINT32*)ptr;
	device->texture[1] = (IUINT32*)(ptr + 16);
	memset(device->texture_di[0], 0, 64);
	memset(device->texture[0], 0, 64);
	device->tex_width = 2;
	device->tex_height = 2;
	device->max_u = 1.0f;
	device->max_v = 1.0f;
	device->width = width;
	device->height = height;
	device->background = 0xc0c0c0;
	device->foreground = 0;
	device->transform.init(width, height);
	//transform_init(&device->transform, width, height);
	device->render_state = RENDER_STATE_WIREFRAME;
}
// 删除设备
void device_destory(device_t* device)
{
	if (device->framebuffer)
		free(device->framebuffer);
	device->framebuffer = NULL;
	device->zbuffer = NULL;
	device->texture = NULL;
}
//设置当前纹理 
void device_set_texture(device_t* device, void* bits, long pitch, int w, int h)
{
	char* ptr = (char*)bits;
	int j;
	assert(w <= 1024 && h <= 1024);
	for (j = 0; j < h; ptr += pitch, j++)
		device->texture[j] = (IUINT32*)ptr;
	device->tex_width = w;
	device->tex_height = h;
	device->max_u = (float)(w - 1);
	device->max_v = (float)(h - 1);
}
void device_set_texture_by_photo(device_t* device, IUINT32** texture, long pitch, int w, int h)
{

	int n = h; int m = w;
	device->texture = NULL;
	device->texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		device->texture[i] = new IUINT32[m];



	assert(w <= 1024 && h <= 1024);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
			device->texture[i][j] = texture[i][j];
	}
	device->tex_width = w;
	device->tex_height = h;
	device->max_u = (float)(w - 1);
	device->max_v = (float)(h - 1);
}
void device_set_texture_by_diffuse(device_t* device, IUINT32** texture, long pitch, int w, int h, int count)
{



	//memset(device->texture[0], 0, 64);
	device->material[count].diffuse_texture.tex_width = 2;
	device->material[count].diffuse_texture.tex_height = 2;
	device->material[count].diffuse_texture.max_u = 1.0f;
	device->material[count].diffuse_texture.max_v = 1.0f;
	int n = h; int m = w;
	device->material[count].diffuse_texture.texture = NULL;
	device->material[count].diffuse_texture.texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		device->material[count].diffuse_texture.texture[i] = new IUINT32[m];



	assert(w <= 1024 && h <= 1024);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
			device->material[count].diffuse_texture.texture[i][j] = texture[i][j];
	}
	device->material[count].diffuse_texture.tex_width = w;
	device->material[count].diffuse_texture.tex_height = h;
	device->material[count].diffuse_texture.max_u = (float)(w - 1);
	device->material[count].diffuse_texture.max_v = (float)(h - 1);

}

void device_set_texture_by_specular(device_t* device, IUINT32** texture, long pitch, int w, int h, int count)
{


	device->material[count].specular_texture.tex_width = 2;
	device->material[count].specular_texture.tex_height = 2;
	device->material[count].specular_texture.max_u = 1.0f;
	device->material[count].specular_texture.max_v = 1.0f;
	int n = h; int m = w;
	device->material[count].specular_texture.texture = NULL;
	device->material[count].specular_texture.texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		device->material[count].specular_texture.texture[i] = new IUINT32[m];


	assert(w <= 1024 && h <= 1024);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
			device->material[count].specular_texture.texture[i][j] = texture[i][j];
	}
	device->material[count].specular_texture.tex_width = w;
	device->material[count].specular_texture.tex_height = h;
	device->material[count].specular_texture.max_u = (float)(w - 1);
	device->material[count].specular_texture.max_v = (float)(h - 1);
}

void device_set_texture_by_normal(device_t* device, IUINT32** texture, long pitch, int w, int h, int count)
{
	device->material[count].normal_texture.tex_width = 2;
	device->material[count].normal_texture.tex_height = 2;
	device->material[count].normal_texture.max_u = 1.0f;
	device->material[count].normal_texture.max_v = 1.0f;
	int n = h; int m = w;
	device->material[count].normal_texture.texture = NULL;
	device->material[count].normal_texture.texture = new IUINT32 * [n];

	for (int i = 0; i < n; i++)
		device->material[count].normal_texture.texture[i] = new IUINT32[m];


	assert(w <= 1024 && h <= 1024);
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
			device->material[count].normal_texture.texture[i][j] = texture[i][j];
	}
	device->material[count].normal_texture.tex_width = w;
	device->material[count].normal_texture.tex_height = h;
	device->material[count].normal_texture.max_u = (float)(w - 1);
	device->material[count].normal_texture.max_v = (float)(h - 1);
}

void device_set_pointlight(device_t* device, s_vector& pos, s_vector& color, s_vector& am, s_vector& di, s_vector& spe, int cnt)
{
	device->pointlight[cnt].lightpos = pos;
	device->pointlight[cnt].lightcolor = color;
	device->pointlight[cnt].ambient = am;
	device->pointlight[cnt].diffuse = di;
	device->pointlight[cnt].specular = spe;
}

// 清空 framebuffer 和 zbuffer
void device_clear(device_t* device, int mode)
{
	int y, x, height = device->height;
	for (y = 0; y < device->height; y++)
	{
		IUINT32* dst = device->framebuffer[y];
		IUINT32 cc = (height - 1 - y) * 230 / (height - 1);
		cc = (cc << 16) | (cc << 8) | cc;
		if (mode == 0) cc = device->background;
		for (x = device->width; x > 0; dst++, x--) dst[0] = cc;
	}
	for (y = 0; y < device->height; y++)
	{
		float* dst = device->zbuffer[y];
		for (x = device->width; x > 0; dst++, x--) dst[0] = 0.0f;
	}
}
void device_pixel(device_t* device, int x, int y, IUINT32 color)
{
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height)
	{
		device->framebuffer[y][x] = color;
	}
}
void device_draw_line(device_t* device, int x1, int y1, int x2, int y2, IUINT32 c)
{
	int x, y, rem = 0;
	if (x1 == x2 && y1 == y2)//如果只是一个点的话 
	{
		device_pixel(device, x1, y1, c);
	}
	else if (x1 == x2)
	{
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) device_pixel(device, x1, y, c);
		device_pixel(device, x2, y2, c);
	}
	else if (y1 == y2)
	{
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) device_pixel(device, x, y1, c);
		device_pixel(device, x2, y2, c);
	}
	else
	{
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		if (dx >= dy)
		{
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++)
			{
				device_pixel(device, x, y, c);
				rem += dy;
				if (rem >= dx)
				{
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1;//模糊接近这个直线 
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
		else
		{
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++)
			{
				device_pixel(device, x, y, c);
				rem += dx;
				if (rem >= dy)
				{
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}

	}
}
// 根据坐标读取纹理
IUINT32 device_texture_read(const device_t* device, float u, float v)
{
	int x, y;
	u = u * device->max_u;
	v = v * device->max_v;
	x = (int)(u + 0.5f);
	y = (int)(v + 0.5f);
	x = CMID(x, 0, device->tex_width - 1);
	y = CMID(y, 0, device->tex_height - 1);
	return device->texture[y][x];
}

IUINT32 device_texture_read_from_material(const s_texture* t_texture, float u, float v)
{
	int x, y;
	u = u * t_texture->max_u;
	v = v * t_texture->max_v;
	x = (int)(u + 0.5f);
	y = (int)(v + 0.5f);
	x = CMID(x, 0, t_texture->tex_width - 1);
	y = CMID(y, 0, t_texture->tex_height - 1);

	return t_texture->texture[y][x];

}

void read_the_texture(s_vector& tmp, const  s_texture* t_texture, float u, float v)
{
	int x, y;
	u = u * t_texture->max_u;
	v = v * t_texture->max_v;
	x = (int)(u + 0.5f);
	y = (int)(v + 0.5f);
	x = CMID(x, 0, t_texture->tex_width - 1);
	y = CMID(y, 0, t_texture->tex_height - 1);

	IUINT32 cc = t_texture->texture[y][x];
	int texture_a = (cc >> 24) & 0xff;
	int texture_r = (cc >> 16) & 0xff;
	int texture_g = (cc >> 8) & 0xff;
	int texture_b = cc & 0xff;
	//printf("%d %d %d\n", texture_r, texture_g, texture_b);
	//printf("%f %f %f\n", ff.color.r, ff.color.g, ff.color.b);
	tmp.x = (float)texture_r / 255.0f;
	tmp.y = (float)texture_g / 255.0f;
	tmp.z = (float)texture_b / 255.0f;
	tmp.w = (float)texture_a / 255.0f;

}

bool computeBarycentric3D(s_vector& tmp, s_vector& p0, s_vector& p1, s_vector& p2, s_vector& pos)
{
	s_vector d1, d2, n;
	d1.minus_two(p1, p0);
	d2.minus_two(p2, p1);
	n.crossproduct(d1, d2);
	float u1, u2, u3, u4;
	float v1, v2, v3, v4;
	if ((fabs(n.x) >= fabs(n.y)) && (fabs(n.x) >= fabs(n.z))) {
		u1 = p0.y - p2.y;
		u2 = p1.y - p2.y;
		u3 = pos.y - p0.y;
		u4 = pos.y - p2.y;
		v1 = p0.z - p2.z;
		v2 = p1.z - p2.z;
		v3 = pos.z - p0.z;
		v4 = pos.z - p2.z;
	}
	else if (fabs(n.y) >= fabs(n.z)) {
		u1 = p0.z - p2.z;
		u2 = p1.z - p2.z;
		u3 = pos.z - p0.z;
		u4 = pos.z - p2.z;
		v1 = p0.x - p2.x;
		v2 = p1.x - p2.x;
		v3 = pos.x - p0.x;
		v4 = pos.x - p2.x;
	}
	else {
		u1 = p0.x - p2.x;
		u2 = p1.x - p2.x;
		u3 = pos.x - p0.x;
		u4 = pos.x - p2.x;
		v1 = p0.y - p2.y;
		v2 = p1.y - p2.y;
		v3 = pos.y - p0.y;
		v4 = pos.y - p2.y;
	}

	float denom = v1 * u2 - v2 * u1;
	if (fabsf(denom) < 1e-6) {
		return 0;
	}
	float oneOverDenom = 1.0f / denom;
	tmp.x = (v4 * u2 - v2 * u4) * oneOverDenom;
	tmp.y = (v1 * u3 - v3 * u1) * oneOverDenom;
	tmp.z = 1.0f - tmp.x - tmp.y;
	return 1;
}
// http://www.cnblogs.com/ThreeThousandBigWorld/archive/2012/07/16/2593892.html
// http://blog.chinaunix.net/uid-26651460-id-3083223.html
// http://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
void calculate_tangent_and_binormal(s_vector& tangent, s_vector& binormal, s_vector& position1, s_vector& position2, s_vector& position3,
	float u1, float v1, float u2, float v2, float u3, float v3)
{
	//side0 is the vector along one side of the triangle of vertices passed in,
	//and side1 is the vector along another side. Taking the cross product of these returns the normal.
	s_vector side0(0.0f, 0.0f, 0.0f, 1.0f);
	side0.minus_two(position1, position2);
	s_vector side1(0.0f, 0.0f, 0.0f, 1.0f);
	side1.minus_two(position3, position1);
	//Calculate face normal
	s_vector normal(0.0f, 0.0f, 0.0f, 0.0f);  normal.crossproduct(side1, side0); normal.normalize();

	//Now we use a formula to calculate the tangent.
	float deltaV0 = v1 - v2;
	float deltaV1 = v3 - v1;
	tangent = side0;
	tangent.float_dot(deltaV1);
	s_vector temp = side1;
	temp.float_dot(deltaV0);

	tangent.minus_two(tangent, temp);    tangent.normalize();

	//Calculate binormal
	float deltaU0 = u1 - u2;
	float deltaU1 = u3 - u1;
	binormal = side0;
	binormal.float_dot(deltaU1);
	temp = side1;
	temp.float_dot(deltaU0);
	binormal.minus_two(binormal, temp);
	binormal.normalize();
	//Now, we take the cross product of the tangents to get a vector which
	//should point in the same direction as our normal calculated above.
	//If it points in the opposite direction (the dot product between the normals is less than zero),
	//then we need to reverse the s and t tangents.
	//This is because the triangle has been mirrored when going from tangent space to object space.
	//reverse tangents if necessary
	s_vector tangentCross;
	tangentCross.crossproduct(tangent, binormal);
	if (tangentCross.dotproduct(normal) < 0.0f)
	{
		tangent.w = -1;
		//vector_inverse(tangent);
		//vector_inverse(binormal);
	}

	//binormal->w = 0.0f;
}
void ff_interpolating(for_fs* dest, for_fs* src1, for_fs* src2, for_fs* src3, float a, float b, float c)
{
	dest->pos.interpolate(a, b, c, src1->pos, src2->pos, src3->pos, 1.0f);
	dest->color.interpolate(a, b, c, src1->color, src2->color, src3->color, 1.0f);
	dest->texcoord.interpolate(a, b, c, src1->texcoord, src2->texcoord, src3->texcoord, 1.0f);
	dest->normal.interpolate(a, b, c, src1->normal, src2->normal, src3->normal, 1.0f);
	dest->storage0.interpolate(a, b, c, src1->storage0, src2->storage0, src3->storage0, 1.0f);
	dest->storage1.interpolate(a, b, c, src1->storage1, src2->storage1, src3->storage1, 1.0f);
	dest->storage2.interpolate(a, b, c, src1->storage2, src2->storage2, src3->storage2, 1.0f);
}


//=====================================================================
// 渲染实现
//=====================================================================

// 绘制扫描线
void device_draw_scanline(device_t* device, scanline_t* scanline, s_vector& point1, s_vector& point2, s_vector& point3, for_fs* ffs, int count)
{
	IUINT32* framebuffer = device->framebuffer[scanline->y];
	float* zbuffer = device->zbuffer[scanline->y];
	int x = scanline->x;
	int w = scanline->w;
	int width = device->width;
	int height = device->height;
	int render_state = device->render_state;
	for (; w > 0; x++, w--)
	{
		if (x >= 0 && x < width)
		{
			float rhw = scanline->v.rhw;
			float ww = 1.0f / rhw;
			s_vector barycenter(0.0f, 0.0f, 0.0f, 1.0f);
			s_vector interpos = scanline->v.pos;
			transform_homogenize_reverse(interpos, interpos, ww, width, height);
			computeBarycentric3D(barycenter, point1, point2, point3, interpos);
			float alpha = barycenter.x; float beta = barycenter.y; float gamma = barycenter.z;

			float Z = 1.0 / (alpha / point1.w + beta / point2.w + gamma / point3.w);
			float zp = alpha * point1.z / point1.w + beta * point2.z / point2.w + gamma * point3.z / point3.w;
			zp *= Z;
			float daozp = 1.0 / zp;
			/*if (zp < depth_buf[get_index(i, j)])
			{
				depth_buf[get_index(i, j)] = zp;
			}*/
			if (daozp >= zbuffer[x])
			{

				zbuffer[x] = daozp;
				for_fs ff;



				ff_interpolating(&ff, &ffs[0], &ffs[1], &ffs[2], barycenter.x, barycenter.y, barycenter.z);
				ff.pos.w = ww;
				ff.normal.normalize();
				s_color color(0.0f, 0.0f, 0.0f, 1.0f);
				if (count == 2)
				{
					bool ban = 0;
					s_vector ori_col(-1.0f, -1.0f, -1.0f, 1.0f);
					if (framebuffer[x] != NULL)
					{
						IUINT32 cc = framebuffer[x];
						int texture_a = (cc >> 24) & 0xff;
						int texture_r = (cc >> 16) & 0xff;
						int texture_g = (cc >> 8) & 0xff;
						int texture_b = cc & 0xff;
						//printf("%d %d %d\n", texture_r, texture_g, texture_b);
						//printf("%f %f %f\n", ff.color.r, ff.color.g, ff.color.b);
						ori_col.x = (float)texture_r / 255.0f;
						ori_col.y = (float)texture_g / 255.0f;
						ori_col.z = (float)texture_b / 255.0f;
						ori_col.w = (float)texture_a / 255.0f;
					}

					f_shader(device, &ff, color, count, ban, ori_col);
					float a = color.a;
					float r = color.r;
					float g = color.g;
					float b = color.b;
					//printf("%f %f %f\n", r, g, b);
					int R = (int)(r * 255.0f);
					int G = (int)(g * 255.0f);
					int B = (int)(b * 255.0f);
					R = CMID(R, 0, 255);
					G = CMID(G, 0, 255);
					B = CMID(B, 0, 255);
					framebuffer[x] = (R << 16) | (G << 8) | (B);
				}
				else if (render_state & RENDER_STATE_COLOR)
				{
					bool ban = 0;
					s_vector ori_col(-1.0f, -1.0f, -1.0f, 1.0f);
					if (framebuffer[x] != NULL)
					{
						IUINT32 cc = framebuffer[x];
						int texture_a = (cc >> 24) & 0xff;
						int texture_r = (cc >> 16) & 0xff;
						int texture_g = (cc >> 8) & 0xff;
						int texture_b = cc & 0xff;
						//printf("%d %d %d\n", texture_r, texture_g, texture_b);
						//printf("%f %f %f\n", ff.color.r, ff.color.g, ff.color.b);
						ori_col.x = (float)texture_r / 255.0f;
						ori_col.y = (float)texture_g / 255.0f;
						ori_col.z = (float)texture_b / 255.0f;
						ori_col.w = (float)texture_a / 255.0f;
					}
					f_shader(device, &ff, color, count, ban, ori_col);
					float a = color.a;
					float r = color.r;
					float g = color.g;
					float b = color.b;
					//printf("%f %f %f\n", r, g, b);
					int R = (int)(r * 255.0f);
					int G = (int)(g * 255.0f);
					int B = (int)(b * 255.0f);
					R = CMID(R, 0, 255);
					G = CMID(G, 0, 255);
					B = CMID(B, 0, 255);
					framebuffer[x] = (R << 16) | (G << 8) | (B);
				}
				else if (render_state & RENDER_STATE_TEXTURE)
				{
					bool ban = 0;
					s_vector ori_col(-1.0f, -1.0f, -1.0f, 1.0f);
					if (framebuffer[x] != NULL)
					{
						IUINT32 cc = framebuffer[x];
						int texture_a = (cc >> 24) & 0xff;
						int texture_r = (cc >> 16) & 0xff;
						int texture_g = (cc >> 8) & 0xff;
						int texture_b = cc & 0xff;
						//printf("%d %d %d\n", texture_r, texture_g, texture_b);
						//printf("%f %f %f\n", ff.color.r, ff.color.g, ff.color.b);
						ori_col.x = (float)texture_r / 255.0f;
						ori_col.y = (float)texture_g / 255.0f;
						ori_col.z = (float)texture_b / 255.0f;
						ori_col.w = (float)texture_a / 255.0f;
					}
					f_shader(device, &ff, color, count, ban, ori_col);
					if (ban == 0)
					{
						float a = color.a;
						float r = color.r;
						float g = color.g;
						float b = color.b;
						int R = (int)(r * 255.0f);
						int G = (int)(g * 255.0f);
						int B = (int)(b * 255.0f);
						R = CMID(R, 0, 255);
						G = CMID(G, 0, 255);
						B = CMID(B, 0, 255);
						framebuffer[x] = (R << 16) | (G << 8) | (B);
					}
				}

			}

		}
		vertex_add(&scanline->v, &scanline->step);
		if (x >= width) break;
	}
}
//主渲染函数 
void device_render_trap(device_t* device, trapezoid_t* trap, s_vector& point1, s_vector& point2, s_vector& point3, for_fs* ffs, int count)
{
	scanline_t scanline;
	int j, top, bottom;
	top = (int)(trap->top + 0.5f);
	bottom = (int)(trap->bottom + 0.5f);
	for (j = top; j < bottom; j++)
	{
		if (j >= 0 && j < device->height)
		{
			trapezoid_edge_interp(trap, (float)j + 0.5f);
			trapezoid_init_scan_line(trap, &scanline, j);
			device_draw_scanline(device, &scanline, point1, point2, point3, ffs, count);
		}
		if (j >= device->height) break;
	}
}
// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t* device, vertex_t* v1,
	vertex_t* v2, vertex_t* v3, int count)
{
	vertex_t* vertexs[3] = { v1,v2,v3 };
	s_vector points[3];
	/*s_vector p1, p2, p3, c1, c2, c3;
	int render_state = device->render_state;
	// 按照 Transform 变化
	device->transform.apply(c1, v1->pos);
	device->transform.apply(c2, v2->pos);
	device->transform.apply(c3, v3->pos);
	// 裁剪，注意此处可以完善为具体判断几个点在 cvv内以及同cvv相交平面的坐标比例
// 进行进一步精细裁剪，将一个分解为几个完全处在 cvv内的三角形
//等于0表示完全在cvv里面(我的理解
	if (transform_check_cvv(c1) != 0) return;
	if (transform_check_cvv(c2) != 0) return;
	if (transform_check_cvv(c3) != 0) return;
	//归一化，得到屏幕坐标
	device->transform.homogenize(p1, c1);
	device->transform.homogenize(p2, c2);
	device->transform.homogenize(p3, c3);
	*/

	//逆转置
	s_matrix tmp;
	tmp = device->transform.world;
	tmp.inverse(); tmp.transpose();


	s_vector c1, c2, c3;
	for_vs vvs[3]; for_fs ffs[3];
	for (int i = 0; i < 3; i++)
	{
		vertex_t* vertex = vertexs[i];
		for_vs* av = &vvs[i];
		apply_to_vector(vertex->pos, vertex->pos, device->transform.world);
		av->pos = vertex->pos;//世界空间的pos
		int a = 0, b = 0;
		if (i == 0) a = 1, b = 2;
		else if (i == 1) a = 0, b = 2;
		else if (i == 2) a = 0, b = 1;
		calculate_tangent_and_binormal(av->tangent, av->binormal, vertex->pos, vertexs[a]->pos, vertexs[b]->pos, vertex->tc.u, vertex->tc.v, vertexs[a]->tc.u, vertexs[a]->tc.v, vertexs[b]->tc.u, vertexs[b]->tc.v);

		apply_to_vector(av->tangent, av->tangent, device->transform.world);
		av->binormal.crossproduct(av->normal, av->tangent);
		av->binormal.float_dot(av->tangent.w);

		apply_to_vector(vertex->pos, vertex->pos, device->transform.vp);
		points[i] = vertex->pos;
		if (i == 0) c1 = vertex->pos;
		if (i == 1) c2 = vertex->pos;
		if (i == 2) c3 = vertex->pos;

		apply_to_vector(vertex->normal, vertex->normal, tmp); // 法向量乘正规矩阵
		vertex->normal.normalize();





		av->normal = vertex->normal; // 世界空间的normal
		av->color = vertex->color;
		av->texcoord = vertex->tc;

		v_shader(device, av, &ffs[i]); // 顶点着色器
		transform_homogenize(vertex->pos, vertex->pos, device->width, device->height);

	}
	if (device->is_cull != 0)
	{
		s_vector t1_t2;  t1_t2.minus_two(v2->pos, v1->pos);
		s_vector t2_t3; t2_t3.minus_two(v3->pos, v2->pos);

		float crossdot = t1_t2.x * t2_t3.y - t2_t3.x * t1_t2.y;
		if (device->is_cull == 1)
		{ //逆时针只要正面
			if (crossdot <= 0.0f) return;
		}
		else if (device->is_cull == 2)
		{  //顺时针只要背面
			if (crossdot > 0.0f) return;
		}
	}

	s_vector point1(points[0].x, points[0].y, points[0].z, points[0].w);
	s_vector point2(points[1].x, points[1].y, points[1].z, points[1].w);
	s_vector point3(points[2].x, points[2].y, points[2].z, points[2].w);
	int render_state = device->render_state;
	if (render_state & (RENDER_STATE_TEXTURE | RENDER_STATE_COLOR))
	{

		trapezoid_t traps[2];
		v1->pos.w = c1.w;
		v2->pos.w = c2.w;
		v3->pos.w = c3.w;
		vertex_rhw_init(v1); //初始化w
		vertex_rhw_init(v2); //初始化w 
		vertex_rhw_init(v3); //初始化w 
		int n = trapezoid_init_triangle(traps, v1, v2, v3);
		point1.w = c1.w;
		point2.w = c2.w;
		point3.w = c3.w;
		if (n >= 1) { device_render_trap(device, &traps[0], point1, point2, point3, ffs, count); }
		if (n >= 2) { device_render_trap(device, &traps[1], point1, point2, point3, ffs, count); }
	}

	if ((render_state & RENDER_STATE_WIREFRAME) && device->framebuffer != NULL)//线框绘制 
	{
		device_draw_line(device, (int)v1->pos.x, (int)v1->pos.y, (int)v2->pos.x, (int)v2->pos.y, device->foreground);
		device_draw_line(device, (int)v1->pos.x, (int)v1->pos.y, (int)v3->pos.x, (int)v3->pos.y, device->foreground);
		device_draw_line(device, (int)v3->pos.x, (int)v3->pos.y, (int)v2->pos.x, (int)v2->pos.y, device->foreground);
	}
}

void v_shader(device_t* device, for_vs* vv, for_fs* ff)
{
	ff->pos = vv->pos;
	ff->normal = vv->normal;
	ff->color = vv->color;
	ff->texcoord = vv->texcoord;

	s_vector tmp1(vv->tangent.x, vv->binormal.x, vv->normal.x, 1.0f);
	ff->storage0 = tmp1;

	tmp1.reset(vv->tangent.y, vv->binormal.y, vv->normal.y, 1.0f);
	ff->storage1 = tmp1;

	tmp1.reset(vv->tangent.z, vv->binormal.z, vv->normal.z, 1.0f);
	ff->storage2 = tmp1;
}

void f_shader(device_t* device, for_fs* ff, s_color& color, int count, bool& is_ban, s_vector& ori_co)
{
	if (count == 2)
	{



		color.r = ff->color.r;
		color.g = ff->color.g;
		color.b = ff->color.b;
		color.a = ff->color.a;



	}
	else
	{
		s_vector init_diffuse(0.64, 0.64, 0.64, 1.0f);
		s_vector init_specular(0.5, 0.5, 0.5, 1.0f);
		float u = ff->texcoord.u; float v = ff->texcoord.v;
		s_vector result(0.0f, 0.0f, 0.0f, 1.0f);
		s_vector lightpos;
		s_vector light_ambient; light_ambient = device->pointlight[0].ambient;
		s_vector light_diffuse; light_diffuse = device->pointlight[0].diffuse;
		s_vector light_specular; light_specular = device->pointlight[0].specular;
		lightpos = device->pointlight[0].lightpos;
		//bump
		/*if (device->material[count].have_normal == 1)
			lightpos.reset(ff->storage0.dotproduct(lightpos), ff->storage1.dotproduct(lightpos), ff->storage2.dotproduct(lightpos), 1.0f);
		*/

		s_vector objectcolor(ff->color.r, ff->color.g, ff->color.b, ff->color.a);
		//环境光照 ambient

		//float r, g, b, a; a = 1.0f;
		//device_texture_read_from_material(device, u, v, r, g, b, a);
		//printf("%lf %lf %lf %lf\n", r, g, b, a);
		s_vector material_ambient;
		if (device->material[count].have_diffuse == 1)
			read_the_texture(material_ambient, &device->material[count].diffuse_texture, u, v);
		else material_ambient = init_diffuse;
		if (material_ambient.w < 0.1f)
		{
			is_ban = 1; return;
		}
		else
			if (ori_co.x != -1.0f)
			{
				float a1 = material_ambient.w; float a2 = 1.0f - a1;
				s_vector tmp1; tmp1 = material_ambient; tmp1.float_dot(a1);
				s_vector tmp2; tmp2 = ori_co; tmp2.float_dot(a2);
				material_ambient.add_two(tmp1, tmp2);
			}

		s_vector ambient;                ambient.dot_two(light_ambient, material_ambient);
		//ambient.show();

		//漫反射光照 diffuse
		s_vector norm;       // = ff->normal
		if (device->material[count].have_normal == 1)
		{
			read_the_texture(norm, &device->material[count].normal_texture, u, v);
			norm.float_dot(2.0f);
			s_vector tmp_1(1.0f, 1.0f, 1.0f, 1.0f);
			norm.minus_two(norm, tmp_1);
			norm.normalize();
			norm.reset(ff->storage0.dotproduct(norm), ff->storage1.dotproduct(norm), ff->storage2.dotproduct(norm), 1.0f);
			norm.normalize();
		}
		else
		{
			norm = ff->normal;
		}

		s_vector fragpos = ff->pos;
		//bump
		/*if (device->material[count].have_normal == 1)
			fragpos.reset(ff->storage0.dotproduct(fragpos), ff->storage1.dotproduct(fragpos), ff->storage2.dotproduct(fragpos), 1.0f);
			*/

		s_vector lightDir; lightDir.minus_two(lightpos, fragpos); lightDir.normalize();
		//lightDir.show();
		float diff = max(norm.dotproduct(lightDir), 0.0f);
		//if (diff > 0.0f) { printf("%lf\n", diff); norm.show(); }
		s_vector material_diffuse; material_diffuse = material_ambient;
		s_vector diffuse; diffuse = light_diffuse; diffuse.float_dot(diff); diffuse.dot_two(diffuse, material_diffuse);

		//specular 镜面高光
		float material_shininess; material_shininess = device->material[count].shininess;
		//get the view pos
		s_vector viewpos = device->camera.viewpos;
		//bump
		/*if (device->material[count].have_normal == 1)
			viewpos.reset(ff->storage0.dotproduct(viewpos), ff->storage1.dotproduct(viewpos), ff->storage2.dotproduct(viewpos), 1.0f);
			*/

		s_vector viewdir; viewdir.minus_two(viewpos, fragpos); viewdir.normalize();
		s_vector in_lightdir; in_lightdir = lightDir; in_lightdir.inverse();
		s_vector reflectdir; reflectdir.reflect(in_lightdir, norm);
		float spec = pow(max(viewdir.dotproduct(reflectdir), 0.0f), material_shininess);
		s_vector specular = light_specular;
		specular.float_dot(spec);

		s_vector material_specular;
		if (device->material[count].have_specular == 1)
			read_the_texture(material_specular, &device->material[count].specular_texture, u, v);
		else material_specular = init_specular;
		specular.dot_two(specular, material_specular);
		//specular.show();
		result.add_two(ambient, diffuse); result.add_two(result, specular);
		//result.dot_two(result, objectcolor);
		//result.show();

		//gamma 矫正
		/*float gamma = 2.2;

		float xx = result.x; float yy = result.y; float zz = result.z;
		xx = pow(xx, 1.0f / gamma);
		yy = pow(yy, 1.0f / gamma);
		zz = pow(zz, 1.0f / gamma);
		result.reset(xx, yy, zz, result.w);
		*/

		color.r = result.x;
		color.g = result.y;
		color.b = result.z;
		color.a = result.w;


		/*
		color.r = ff->color.r;
		color.g = ff->color.g;
		color.b = ff->color.b;
		color.a = ff->color.a;
		*/


	}
}


void draw_plane(device_t* device, int num, vertex_t* mesh, int count)
{
	vertex_t p1, p2, p3;
	for (int i = 0; i < num; i += 3)
	{
		p1 = mesh[i]; p2 = mesh[i + 1]; p3 = mesh[i + 2];
		int index_m = p1.material_idex;
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
			int A = aa;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);
			A = CMID(A, 0, 255);
			texture[y][x] = (A << 24) | (R << 16) | (G << 8) | (B);
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


void init_texture_by_diffuse(device_t* device, char const* path, int count)
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
			int A = aa;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);
			A = CMID(A, 0, 255);
			texture[y][x] = (A << 24) | (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_diffuse(device, texture, height * 4, width, height, count);

}

void init_texture_by_specular(device_t* device, char const* path, int count)
{  //设置镜面高光贴图
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
			int A = aa;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);
			A = CMID(A, 0, 255);
			texture[y][x] = (A << 24) | (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_specular(device, texture, height * 4, width, height, count);
}


void init_texture_by_normal(device_t* device, char const* path, int count)
{
	//设置法向量贴图
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
			int A = aa;
			R = CMID(R, 0, 255);
			G = CMID(G, 0, 255);
			B = CMID(B, 0, 255);
			A = CMID(A, 0, 255);
			texture[y][x] = (A << 24) | (R << 16) | (G << 8) | (B);
			//cout<<(int)r<<" "<<(int)g<<" "<<(int)b<<" "<<(int)a<<endl;
		}
	}
	stbi_image_free(data);
	device_set_texture_by_normal(device, texture, height * 4, width, height, count);
}

bool load_obj(std::vector<vertex_t>& tot_vertex, device_t* device, const char* obj_path, const char* pre_mtl_path, int start, bool filp_y)
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
		device->material[start + i].have_normal = 0;
		if (tm.diffuse_texname != "") {
			string the_path = tm.diffuse_texname;
			the_path = "model/" + the_path;
			const char* str = the_path.c_str();
			device->material[start + i].have_diffuse = 1;
			init_texture_by_diffuse(device, str, start + i);
			//init_texture_by_specular(&device, "container2_specular.png", 1);
			//m.diffuse_tex_id = make_texture_by_png(m.diffuse_texname, true);
		}
		if (tm.specular_texname != "") {
			string the_path = tm.specular_texname;
			the_path = "model/" + the_path;
			const char* str = the_path.c_str();
			device->material[start + i].have_specular = 1;
			init_texture_by_specular(device, str, start + i);
		}

		if (tm.bump_texname != "") {
			string the_path = tm.bump_texname;
			the_path = "model/" + the_path;
			const char* str = the_path.c_str();
			device->material[start + i].have_normal = 1;
			init_texture_by_normal(device, str, start + i);
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
				if (filp_y == 1)
					mesh_data.pos.z = -attrib.vertices[(vertex_index[v]) * 3 + 1];
				else mesh_data.pos.z = attrib.vertices[(vertex_index[v]) * 3 + 1];
				mesh_data.pos.x = attrib.vertices[(vertex_index[v]) * 3 + 2];
				mesh_data.pos.w = 1.0f;

				// vt
				mesh_data.tc.u = (float)attrib.texcoords[texcoord_index[v] * 2 + 0];
				mesh_data.tc.v = (float)attrib.texcoords[texcoord_index[v] * 2 + 1];

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