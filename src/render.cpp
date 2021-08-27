//=====================================================================
// 渲染设备
//=====================================================================
#include <cstring>
#include<cstdio>
#include<iostream>
#include <cassert>
#include "s_math.h"
#include "rasterize.h"
#include "render.h"


//设备初始化，fb为外部帧缓存，非NULL将引用外部帧缓存 
void device_init(device_t* device, int width, int height, void* fb)
{
	int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
	char* ptr = (char*)malloc(need + 64);
	char* framebuf, * zbuf;
	int j;
	assert(ptr);
	device->framebuffer = (IUINT32**)ptr;
	device->zbuffer = (float**)(ptr + sizeof(void*) * height);
	ptr += sizeof(void*) * height * 2;
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
	device->texture[0] = (IUINT32*)ptr;
	device->texture[1] = (IUINT32*)(ptr + 16);
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
void device_draw_scanline(device_t* device, scanline_t* scanline, s_vector& point1, s_vector& point2, s_vector& point3, for_fs* ffs)
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
			
			float Z = 1.0 / (alpha / point1.w + beta /point2.w + gamma /point3.w);
			float zp = alpha * point1.z / point1.w + beta * point2.z / point2.w + gamma * point3.z / point3.w;
			zp *= Z;
			float daozp =1.0/zp;
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
				if (render_state & RENDER_STATE_COLOR)
				{
					float a = ff.color.a;
					float r = ff.color.r;
					float g = ff.color.g;
					float b = ff.color.b;
					int R = (int)(r * 255.0f);
					int G = (int)(g * 255.0f);
					int B = (int)(b * 255.0f);
					R = CMID(R, 0, 255);
					G = CMID(G, 0, 255);
					B = CMID(B, 0, 255);
					framebuffer[x] = (R << 16) | (G << 8) | (B);
				}
				if (render_state & RENDER_STATE_TEXTURE)
				{
					float u = ff.texcoord.u ;
					float v = ff.texcoord.v;
					IUINT32 cc = device_texture_read(device, u, v);
					framebuffer[x] = cc;
				}
			}
		}
		vertex_add(&scanline->v, &scanline->step);
		if (x >= width) break;
	}
}
//主渲染函数 
void device_render_trap(device_t* device, trapezoid_t* trap, s_vector& point1, s_vector& point2, s_vector& point3, for_fs* ffs)
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
			device_draw_scanline(device, &scanline, point1, point2, point3, ffs);
		}
		if (j >= device->height) break;
	}
}
// 根据 render_state 绘制原始三角形
void device_draw_primitive(device_t* device, vertex_t* v1,
	vertex_t* v2, vertex_t* v3)
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
		points[i]= vertex->pos;
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
		if (n >= 1) { device_render_trap(device, &traps[0], point1, point2, point3, ffs); }
		if (n >= 2) {device_render_trap(device, &traps[1], point1, point2, point3, ffs);  }
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
void f_shader(device_t* device, for_fs* ff, s_color& color)
{

}

