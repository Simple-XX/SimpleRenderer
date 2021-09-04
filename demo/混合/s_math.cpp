#include "s_math.h"
#include<cmath>
//越界处理 
int CMID(int x, int min, int max)
{
	return (x < min) ? min : ((x > max) ? max : x);
}
//计算插值：t 为 [0, 1] 之间的数值 
float interp(float x1, float x2, float t)
{
	return x1 + (x2 - x1) * t;
}
float interp_(float& x1, float& x2, float t)
{
	return x1 + (x2 - x1) * t;
}

float s_vector::length()//长度 
{
	return (float)sqrt(x * x + y * y + z * z);
}
//a=b+c;
void s_vector::add_two(s_vector& b, s_vector& c)
{
	x = b.x + c.x;
	y = b.y + c.y;
	z = b.z + c.z;
	w = 1.0;
}
//a+=b;
void s_vector::add(s_vector& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	w = 1.0f;
}

s_vector& s_vector::operator+=(s_vector& t)
{
	x += t.x; y += t.y; z += t.z; w = 1.0;
	return *this;
}
//a=b-c;
void s_vector::minus_two(s_vector& b, s_vector& c)
{
	x = b.x - c.x;
	y = b.y - c.y;
	z = b.z - c.z;
	w = 1.0;
}

//return the result of dotproduct 
float s_vector::dotproduct(s_vector& t)
{
	return x * t.x + y * t.y + z * t.z;
}
//a=a*b;


void s_vector::dot_two(s_vector& b, s_vector& c)
{
	x = b.x * c.x;
	y = b.y * c.y;
	z = b.z * c.z;
	w = 1.0;
}

void s_vector::float_dot(float k)
{
	x *= k;
	y *= k;
	z *= k;
	w *= k;
}

void s_vector::float_dot_two(s_vector& tmp, float k)
{
	x = tmp.x * k;
	y = tmp.y * k;
	z = tmp.z * k;
	w = tmp.w * k;
}

//a=b X c
void s_vector::crossproduct(s_vector& b, s_vector& c)
{
	x = b.y * c.z - b.z * c.y;
	y = b.z * c.x - b.x * c.z;
	z = b.x * c.y - b.y * c.x;
	w = 1.0f;
}
//interp the vector
void s_vector::interp_two(s_vector& a, s_vector& b, float t)
{
	x = interp(a.x, b.x, t);
	y = interp(a.y, b.y, t);
	z = interp(a.z, b.z, t);
	w = 1.0f;
}

void s_vector::normalize()
{
	float length = this->length();
	if (length != 0.0f)
	{
		float inv = 1.0f / length;
		x *= inv;
		y *= inv;
		z *= inv;
	}
}
void s_vector::float_divide(float k)
{
	x = x / k;
	y = y / k;
	z = z / k;
	w = w / k;
}

void s_vector::interpolate(float alpha, float beta, float gamma, s_vector& vert1, s_vector& vert2, s_vector& vert3, float weight)
{
	s_vector tmp1; tmp1.float_dot_two(vert1, alpha);
	s_vector tmp2; tmp2.float_dot_two(vert2, beta);
	s_vector tmp3; tmp3.float_dot_two(vert3, gamma);
	s_vector tmp4; tmp4.add_two(tmp1, tmp2);
	tmp4.add_two(tmp4, tmp3);
	tmp4.float_divide(weight);
	x = tmp4.x;
	y = tmp4.y;
	z = tmp4.z;
	w = tmp4.w;
}

void s_vector::inverse()
{
	x = -x;
	y = -y;
	z = -z;
}

void s_vector::reflect(s_vector& v, s_vector& n)
{
	*this = n;
	float temp = -2.0 * v.dotproduct(n);
	this->float_dot(temp);
	this->add(v);
}


//s_vector2f
void s_vector2f::add_two(s_vector2f& b, s_vector2f& c)
{
	u = b.u + c.u;
	v = b.v + c.v;
}
void s_vector2f::float_dot(float k)
{
	u *= k;
	v *= k;
}
void s_vector2f::float_dot_two(s_vector2f& tmp, float k)
{
	u = tmp.u * k;
	v = tmp.v * k;

}
void s_vector2f::float_divide(float k)
{
	u /= k;
	v /= k;
}
void s_vector2f::interpolate(float alpha, float beta, float gamma, s_vector2f& vert1, s_vector2f& vert2, s_vector2f& vert3, float weight)
{
	s_vector2f tmp1; tmp1.float_dot_two(vert1, alpha);
	s_vector2f tmp2; tmp2.float_dot_two(vert2, beta);
	s_vector2f tmp3; tmp3.float_dot_two(vert3, gamma);
	s_vector2f tmp4; tmp4.add_two(tmp1, tmp2);
	tmp4.add_two(tmp4, tmp3);
	tmp4.float_divide(weight);
	u = tmp4.u;
	v = tmp4.v;
}


void s_color::dot_two(s_color& x, s_color& y)
{
	r = x.r * y.r;
	g = x.g * y.g;
	b = x.b * y.b;
	a = x.a * y.a;
}

void s_color::dot_array(s_color& x, float* c)
{
	r = x.r * c[0];
	g = x.g * c[1];
	b = x.b * c[2];
}
void s_color::float_dot(float k)
{
	r *= k;
	g *= k;
	b *= k;
	a *= k;
}
void s_color::add_two(s_color& x, s_color& y)
{
	r = x.r + y.r;
	g = x.g + y.g;
	b = x.b + y.b;
	a = x.a + y.a;
}
void s_color::minus_two(s_color& x, s_color& y)
{
	r = x.r - y.r;
	g = x.g - y.g;
	b = x.b - y.b;
	a = x.a - y.a;
}
void s_color::float_dot_two(s_color& x, float f)
{
	r = x.r * f;
	g = x.g * f;
	b = x.b * f;
	a = x.a * f;
}
void s_color::float_divide(float f)
{
	r /= f;
	g /= f;
	b /= f;
	a /= f;
}
void s_color::interpolate(float alpha, float beta, float gamma, s_color& vert1, s_color& vert2, s_color& vert3, float weight)
{
	s_color tmp1; tmp1.float_dot_two(vert1, alpha);
	s_color tmp2; tmp2.float_dot_two(vert2, beta);
	s_color tmp3; tmp3.float_dot_two(vert3, gamma);
	s_color tmp4; tmp4.add_two(tmp1, tmp2);
	tmp4.add_two(tmp4, tmp3);
	tmp4.float_divide(weight);
	r = tmp4.r;
	g = tmp4.g;
	b = tmp4.b;
	a = tmp4.a;
}


// m=a+b;
void s_matrix::add_two(s_matrix& a, s_matrix& b)
{

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = a.m[i][j] + b.m[i][j];
}
// m=a-b;
void s_matrix::minus_two(s_matrix& a, s_matrix& b)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = a.m[i][j] - b.m[i][j];
}
//m=a*b;
void s_matrix::mul_two(s_matrix& a, s_matrix& b)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[j][i] = (a.m[j][0] * b.m[0][i]) +
				(a.m[j][1] * b.m[1][i]) +
				(a.m[j][2] * b.m[2][i]) +
				(a.m[j][3] * b.m[3][i]);
		}
	}
}
//m=a*f
void s_matrix::scale(s_matrix& a, float f)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m[i][j] = a.m[i][j] * f;
		}
	}
}
//set to单位矩阵 
void s_matrix::set_identity()
{
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
}
//变成0 
void s_matrix::set_zero()
{
	m[0][0] = m[0][1] = m[0][2] = m[0][3] = 0.0f;
	m[1][0] = m[1][1] = m[1][2] = m[1][3] = 0.0f;
	m[2][0] = m[2][1] = m[2][2] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = m[3][3] = 0.0f;
}
//拿矩阵去左乘坐标的话是写在最右边，坐标去乘矩阵就是写在底下 
void s_matrix::set_translate(float x, float y, float z)
{
	this->set_identity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}
void s_matrix::set_scale(float x, float y, float z)
{
	this->set_identity();
	m[0][0] = x;
	m[1][1] = y;
	m[2][2] = z;
}
//let m be the rotate matrix 
void s_matrix::set_rotate(float x, float y, float z, float theta)
{
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	s_vector vec(x, y, z, 1.0f);
	float w = qcos;
	vec.normalize();
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m[1][0] = 2 * x * y - 2 * w * z;
	m[2][0] = 2 * x * z + 2 * w * y;
	m[0][1] = 2 * x * y + 2 * w * z;
	m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m[2][1] = 2 * y * z - 2 * w * x;
	m[0][2] = 2 * x * z - 2 * w * y;
	m[1][2] = 2 * y * z + 2 * w * x;
	m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m[0][3] = m[1][3] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

//let m be the rotate matrix 
void s_matrix::set_rotate(s_vector v, float theta)
{
	float x = v.x, y = v.y, z = v.z;
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	s_vector vec(x, y, z, 1.0f);
	float w = qcos;
	vec.normalize();
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m[1][0] = 2 * x * y - 2 * w * z;
	m[2][0] = 2 * x * z + 2 * w * y;
	m[0][1] = 2 * x * y + 2 * w * z;
	m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m[2][1] = 2 * y * z - 2 * w * x;
	m[0][2] = 2 * x * z - 2 * w * y;
	m[1][2] = 2 * y * z + 2 * w * x;
	m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m[0][3] = m[1][3] = m[2][3] = 0.0f;
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}
void s_matrix::inverse()
{
	float t[3][6];
	int i, j, k;
	float f;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 6; j++) {
			if (j < 3)
				t[i][j] = m[i][j];
			else if (j == i + 3)
				t[i][j] = 1;
			else
				t[i][j] = 0;
		}

	for (i = 0; i < 3; i++) {
		f = t[i][i];
		for (j = 0; j < 6; j++)
			t[i][j] /= f;
		for (j = 0; j < 3; j++) {
			if (j != i) {
				f = t[j][i];
				for (k = 0; k < 6; k++)
					t[j][k] = t[j][k] - t[i][k] * f;
			}
		}
	}

	for (i = 0; i < 3; i++)
		for (j = 3; j < 6; j++)
			m[i][j - 3] = t[i][j];

	m[3][0] = -m[3][0];
	m[3][1] = -m[3][1];
	m[3][2] = -m[3][2];
}
//转置 
void s_matrix::transpose()
{
	for (int i = 0; i < 3; i++)
		for (int j = i + 1; j < 3; j++)
		{
			float temp = m[i][j];
			m[i][j] = m[j][i];
			m[j][i] = temp;
		}
}

void s_matrix::set_rotate_translate_scale(s_vector& axis, float theta, s_vector& pos, s_vector& scale)
{
	this->set_scale(scale.x, scale.y, scale.z);
	s_matrix r, t = *this;
	r.set_rotate(axis, theta);
	this->mul_two(t, r);
	m[3][0] = pos.x;
	m[3][1] = pos.y;
	m[3][2] = pos.z;
}

void s_matrix::set_axis(s_vector& xaxis, s_vector& yaxis, s_vector& zaxis, s_vector& pos)
{
	m[0][0] = xaxis.x;    m[0][1] = xaxis.y;   m[0][2] = xaxis.z;
	m[1][0] = yaxis.x;    m[1][1] = yaxis.y;   m[1][2] = yaxis.z;
	m[2][0] = zaxis.x;    m[2][1] = zaxis.y;   m[2][2] = zaxis.z;
	m[3][0] = pos.x;        m[3][1] = pos.y;    m[3][2] = pos.z;

	m[0][3] = m[1][3] = m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}