#include "rasterize.h"
#include<iostream>
#include<cmath>
//this=a*m
void apply_to_vector(s_vector& tmp, s_vector& a, s_matrix& m)
{
	float X = a.x, Y = a.y, Z = a.z, W = a.w;
	tmp.x = X * m.m[0][0] + Y * m.m[1][0] + Z * m.m[2][0] + W * m.m[3][0];
	tmp.y = X * m.m[0][1] + Y * m.m[1][1] + Z * m.m[2][1] + W * m.m[3][1];
	tmp.z = X * m.m[0][2] + Y * m.m[1][2] + Z * m.m[2][2] + W * m.m[3][2];
	tmp.w = X * m.m[0][3] + Y * m.m[1][3] + Z * m.m[2][3] + W * m.m[3][3];
}
// let m be the look at matrix
void set_look_at_matrix(s_matrix& m, s_vector eye, s_vector at, s_vector up)
{
	s_vector xaxis, yaxis, zaxis;

	zaxis.minus_two(at, eye);
	zaxis.normalize();

	xaxis.crossproduct(up, zaxis);
	xaxis.normalize();

	yaxis.crossproduct(zaxis, xaxis);

	m.m[0][0] = xaxis.x;
	m.m[1][0] = xaxis.y;
	m.m[2][0] = xaxis.z;
	m.m[3][0] = -xaxis.dotproduct(eye);
	//相机位置在x方向上的投影，xaxis是单位向量 
	m.m[0][1] = yaxis.x;
	m.m[1][1] = yaxis.y;
	m.m[2][1] = yaxis.z;
	m.m[3][1] = -yaxis.dotproduct(eye);

	m.m[0][2] = zaxis.x;
	m.m[1][2] = zaxis.y;
	m.m[2][2] = zaxis.z;
	m.m[3][2] = -zaxis.dotproduct(eye);
	//拿坐标去右乘这个矩阵 
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.0f;
	m.m[3][3] = 1.0f;
}
// let m be the perspective matrix
void set_perspective_matrix(s_matrix& m, float fovy, float aspect, float zn, float zf)
{
	float fax = 1.0f / (float)tan(fovy * 0.5f);//half eye
	m.set_zero();
	m.m[0][0] = (float)(fax / aspect);
	m.m[1][1] = (float)(fax);
	m.m[2][2] = zf / (zf - zn);
	m.m[3][2] = -zn * zf / (zf - zn);
	m.m[2][3] = 1;
}

void set_ortho_matrix(s_matrix& m, float l, float r, float b, float t, float zn, float zf)
{
	m.m[0][0] = 2.0f / (r - l);
	m.m[1][1] = 2.0f / (t - b);
	m.m[2][2] = 1.0f / (zf - zn);

	m.m[3][0] = (l + r) / (l - r);
	m.m[3][1] = (t + b) / (b - t);
	m.m[3][2] = zn / (zn - zf);
	m.m[3][3] = 1.0f;
	m.m[0][1] = m.m[0][2] = m.m[0][3] = m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
	m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
}

s_transform::s_transform(int width, int height)
{
	float aspect = (float)width / ((float)height);
	world.set_identity();
	view.set_identity();
	set_perspective_matrix(projection, 3.1415926f * 0.5f, aspect, 1.0f, 500.0f);
	w = (float)width;
	h = (float)height;
	this->update();
}
void s_transform::init(int width, int height)
{
	float aspect = (float)width / ((float)height);
	world.set_identity();
	view.set_identity();
	set_perspective_matrix(projection, 3.1415926f * 0.5f, aspect, 1.0f, 500.0f);
	w = (float)width;
	h = (float)height;
	this->update();
}

void s_transform::update()
{
	s_matrix m;
	m.mul_two(world, view);
	transform.mul_two(m, projection);
	m.mul_two(view, projection);
	vp = m;
}
//y=x*transform
void s_transform::apply(s_vector& y, s_vector& x)
{
	apply_to_vector(y, x, transform);
}

// 归一化，得到屏幕坐标
//可以参考GAMES101第五课 Canonical Cube to Screen矩阵
void s_transform::homogenize(s_vector& y, s_vector& x)
{
	float rhw = 1.0f / x.w;
	y.x = (x.x * rhw + 1.0f) * w * 0.5f;
	y.y = (1.0f - x.y * rhw) * h * 0.5f;
	y.z = x.z * rhw;
	y.w = 1.0f;
}
void s_transform::homogenize_reverse(s_vector& y, s_vector& x, float weight)
{
	y.x = (x.x * 2 / w - 1.0f) * weight;
	y.y = (1.0f - x.y * 2 / h) * weight;
	y.z = x.z * weight;
	y.w = weight;
}

void transform_homogenize(s_vector& y, s_vector& x, float width, float height)
{
	float rhw = 1.0f / x.w;
	y.x = (x.x * rhw + 1.0f) * width * 0.5f;
	y.y = (1.0f - x.y * rhw) * height * 0.5f;
	y.z = x.z * rhw;
	y.w = rhw;
}
//  6). transform_homogenize(ts, y, x)
void transform_homogenize_reverse(s_vector& y, s_vector& x, float w, float width, float height)
{
	y.x = (x.x * 2 / width - 1.0f) * w;
	y.y = (1.0f - x.y * 2 / height) * w;
	y.z = x.z * w;
	y.w = w;
}


int transform_check_cvv(s_vector& v)
{
	float w = v.w;
	int check = 0;
	if (v.z < 0.0f) check |= 1;
	if (v.z > w) check |= 2;
	if (v.x < -w)check |= 4;
	if (v.x > w)check |= 8;
	if (v.y < -w)check |= 16;
	if (v.y > w)check |= 32;
	return check;
}


void vertex_rhw_init(vertex_t* v)
{
	float rhw = 1.0f / v->pos.w;
	v->rhw = rhw;
	v->tc.u *= rhw;
	v->tc.v *= rhw;
	v->color.r *= rhw;
	v->color.g *= rhw;
	v->color.b *= rhw;
}

void vertex_interp(vertex_t* y, vertex_t* x1, vertex_t* x2, float t)
{

	y->pos.interp_two(x1->pos, x2->pos, t);
	y->tc.u = interp(x1->tc.u, x2->tc.u, t);
	y->tc.v = interp(x1->tc.v, x2->tc.v, t);

	y->color.r = interp(x1->color.r, x2->color.r, t);
	y->color.g = interp(x1->color.g, x2->color.g, t);
	y->color.b = interp(x1->color.b, x2->color.b, t);
	y->rhw = interp(x1->rhw, x2->rhw, t);
}

void vertex_division(vertex_t* y, const vertex_t* x1, const vertex_t* x2, float w)
{
	float inv = 1.0f / w;
	y->pos.x = (x2->pos.x - x1->pos.x) * inv;
	y->pos.y = (x2->pos.y - x1->pos.y) * inv;
	y->pos.z = (x2->pos.z - x1->pos.z) * inv;
	y->pos.w = (x2->pos.w - x1->pos.w) * inv;

	y->tc.u = (x2->tc.u - x1->tc.u) * inv;
	y->tc.v = (x2->tc.v - x1->tc.v) * inv;

	y->color.r = (x2->color.r - x1->color.r) * inv;
	y->color.g = (x2->color.g - x1->color.g) * inv;
	y->color.b = (x2->color.b - x1->color.b) * inv;

	y->rhw = (x2->rhw - x1->rhw) * inv;
}

void vertex_add(vertex_t* y, const vertex_t* x)
{
	y->pos.x += x->pos.x;
	y->pos.y += x->pos.y;
	y->pos.z += x->pos.z;
	y->pos.w += x->pos.w;

	y->rhw += x->rhw;
	y->tc.u += x->tc.u;
	y->tc.v += x->tc.v;

	y->color.r += x->color.r;
	y->color.g += x->color.g;
	y->color.b += x->color.b;
}

//将三角形分割 
int trapezoid_init_triangle(trapezoid_t* trap, vertex_t* p1,
	vertex_t* p2, vertex_t* p3)
{
	vertex_t* p;
	float k, x;
	if (p1->pos.y > p2->pos.y) p = p1, p1 = p2, p2 = p;
	if (p1->pos.y > p3->pos.y) p = p1, p1 = p3, p3 = p;
	if (p2->pos.y > p3->pos.y) p = p2, p2 = p3, p3 = p;

	if (p1->pos.y == p2->pos.y && p1->pos.y == p3->pos.y) return 0;
	if (p1->pos.x == p2->pos.x && p1->pos.x == p3->pos.x) return 0;

	if (p1->pos.y == p2->pos.y)
	{
		if (p1->pos.x > p2->pos.x) p = p1, p1 = p2, p2 = p;
		trap[0].top = p1->pos.y;
		trap[0].bottom = p3->pos.y;
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p3;
		trap[0].right.v1 = *p2;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}

	if (p2->pos.y == p3->pos.y)
	{
		if (p2->pos.x > p3->pos.x) p = p2, p2 = p3, p3 = p;
		trap[0].top = p1->pos.y;
		trap[0].bottom = p3->pos.y;
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p2;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}

	trap[0].top = p1->pos.y;
	trap[0].bottom = p2->pos.y;
	trap[1].top = p2->pos.y;
	trap[1].bottom = p3->pos.y;

	k = (p3->pos.y - p1->pos.y) / (p2->pos.y - p1->pos.y);
	x = p1->pos.x + (p2->pos.x - p1->pos.x) * k;
	//中间那个点水平对应长边上的点，和底部点的x比较 
	if (x <= p3->pos.x)//左三角形 
	{
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p2;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p3;

		trap[1].left.v1 = *p2;
		trap[1].left.v2 = *p3;
		trap[1].right.v1 = *p1;
		trap[1].right.v2 = *p3;
	}
	else//右三角形 
	{
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p3;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p2;
		trap[1].left.v1 = *p1;
		trap[1].left.v2 = *p3;
		trap[1].right.v1 = *p2;
		trap[1].right.v2 = *p3;
	}

	return 2;
}
// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点		
void trapezoid_edge_interp(trapezoid_t* trap, float y)
{
	float s1 = trap->left.v2.pos.y - trap->left.v1.pos.y;
	float s2 = trap->right.v2.pos.y - trap->right.v1.pos.y;
	float t1 = (y - trap->left.v1.pos.y) / s1;
	float t2 = (y - trap->right.v1.pos.y) / s2;
	vertex_interp(&trap->left.v, &trap->left.v1, &trap->left.v2, t1);
	vertex_interp(&trap->right.v, &trap->right.v1, &trap->right.v2, t2);
}

void trapezoid_init_scan_line(trapezoid_t* trap, scanline_t* scanline, int y)
{
	float width = trap->right.v.pos.x - trap->left.v.pos.x;
	scanline->x = (int)(trap->left.v.pos.x + 0.5f);
	scanline->w = (int)(trap->right.v.pos.x + 0.5f) - scanline->x;
	scanline->y = y;
	scanline->v = trap->left.v;
	if (trap->left.v.pos.x >= trap->right.v.pos.x) scanline->w = 0;
	vertex_division(&scanline->step, &trap->left.v, &trap->right.v, width);
}
