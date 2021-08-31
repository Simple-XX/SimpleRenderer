#ifndef s_math_h
#define s_math_h

#include<cstdio>
#include<iostream>
typedef unsigned int IUINT32;
//越界处理 
int CMID(int x, int min, int max);
//计算插值：t 为 [0, 1] 之间的数值 
float interp(float x1, float x2, float t);
float interp_(float& x1, float& x2, float t);
class s_vector
{
public:
	float x, y, z, w;

	s_vector() { x = y = z = w = 0.0; }
	s_vector(float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }
	~s_vector() {};
	float length();
	void show()
	{
		printf("%lf %lf %lf %lf\n", x, y, z, w);
	}
	void reset(float a, float b, float c, float d) { x = a; y = b; z = c; w = d; }

	void equal(s_vector& t)
	{
		x = t.x; y = t.y; z = t.z; w = t.w;
	}
	s_vector& operator=(s_vector& t)
	{
		x = t.x; y = t.y; z = t.z;  w = t.w;
		return *this;
	}
	s_vector& operator+=(s_vector& t);

	//a=b+c;
	void add_two(s_vector& b, s_vector& c);

	void add(s_vector& b);

	//a=b-c;
	void minus_two(s_vector& b, s_vector& c);

	//return the result of dotproduct 
	float dotproduct(s_vector& t);
	//a=a*b;


	void dot_two(s_vector& b, s_vector& c);

	void float_dot(float k);

	void float_dot_two(s_vector& tmp, float k);

	//a=b X c
	void crossproduct(s_vector& b, s_vector& c);
	//interp the vector
	void interp_two(s_vector& a, s_vector& b, float t);

	void normalize();

	void float_divide(float k);
	//插值 
	void interpolate(float alpha, float beta, float gamma, s_vector& vert1, s_vector& vert2, s_vector& vert3, float weight);
	//a=-a;
	void inverse();

	void reflect(s_vector& v, s_vector& n);


};

class s_vector2f
{
public:
	float u, v;
	s_vector2f() { u = v = 0.0; }
	s_vector2f(float a, float b) { u = a; v = b; }
	~s_vector2f() {};
	s_vector2f& operator=(s_vector2f& t)
	{
		u = t.u;
		v = t.v;
		return *this;
	}
	void show()
	{
		printf("%lf %lf\n", u, v);
	}
	void add_two(s_vector2f& b, s_vector2f& c);

	void float_dot(float k);

	void float_dot_two(s_vector2f& tmp, float k);

	void float_divide(float k);

	void interpolate(float alpha, float beta, float gamma, s_vector2f& vert1, s_vector2f& vert2, s_vector2f& vert3, float weight);

};

class s_color
{
public:
	float r, g, b, a;
	s_color() { r = g = b = a = 0.0f; }
	s_color(float x, float y, float z, float w) { r = x; g = y; b = z; a = w; }
	~s_color() {}
	void show()
	{
		printf("%lf %lf %lf %lf\n", r, g, b, a);
	}
	s_color& operator=(s_color& t)
	{
		r = t.r; g = t.g; b = t.b; a = t.a;
		return *this;
	}
	void dot_two(s_color& x, s_color& y);
	void dot_array(s_color& x, float* c);

	void float_dot(float k);

	void add_two(s_color& x, s_color& y);

	void minus_two(s_color& x, s_color& y);

	void float_dot_two(s_color& x, float f);

	void float_divide(float f);

	void interpolate(float alpha, float beta, float gamma, s_color& vert1, s_color& vert2, s_color& vert3, float weight);

};

class s_matrix
{
public:
	float m[4][4];
	s_matrix()
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = 0;
	}
	s_matrix(float x)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = x;
	}
	~s_matrix() {}

	s_matrix& operator=(s_matrix& t)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m[i][j] = t.m[i][j];
			}
		}
		return *this;
	}
	void show()
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				printf("%lf ", m[i][j]);
			}
			printf("\n");
		}
	}
	// m=a+b;
	void add_two(s_matrix& a, s_matrix& b);

	// m=a-b;
	void minus_two(s_matrix& a, s_matrix& b);

	//m=a*b;
	void mul_two(s_matrix& a, s_matrix& b);

	//m=a*f
	void scale(s_matrix& a, float f);

	//set to单位矩阵 
	void set_identity();

	//变成0 
	void set_zero();

	//拿矩阵去左乘坐标的话是写在最右边，坐标去乘矩阵就是写在底下 
	void set_translate(float x, float y, float z);

	void set_scale(float x, float y, float z);

	//let m be the rotate matrix 
	void set_rotate(float x, float y, float z, float theta);
	//let m be the rotate matrix 
	void set_rotate(s_vector v, float theta);
	void inverse();
	//转置 
	void transpose();


	void set_rotate_translate_scale(s_vector& axis, float theta, s_vector& pos, s_vector& scale);

	void set_axis(s_vector& xaxis, s_vector& yaxis, s_vector& zaxis, s_vector& pos);


};

#endif

