#ifndef rasterize_h
#define rasterize_h

#include "s_math.h"

//this=a*m
void apply_to_vector(s_vector& tmp, s_vector& a, s_matrix& m);
// let m be the look at matrix
void set_look_at_matrix(s_matrix& m, s_vector eye, s_vector at, s_vector up);

// let m be the perspective matrix
void set_perspective_matrix(s_matrix& m, float fovy, float aspect, float zn, float zf);

void set_ortho_matrix(s_matrix& m, float l, float r, float b, float t, float zn, float zf);

class s_transform
{
public:
	s_matrix world/*世界坐标变换*/, view/*摄像机坐标变换*/, projection/*投影变换*/;
	s_matrix transform;//transform=world* view * projection
	s_matrix vp;//view*projection
	float w, h;//屏幕大小
	s_transform(int width, int height);
	s_transform() {}
	~s_transform() {}
	void init(int width, int height);
	s_transform& operator=(s_transform& t)
	{
		world = t.world;
		view = t.view;
		projection = t.projection;
		transform = t.transform;
		w = t.w;
		h = t.h;
		return *this;
	}
	void update();
	//y=x*transform
	void apply(s_vector& y, s_vector& x);
	// 归一化，得到屏幕坐标
//可以参考GAMES101第五课 Canonical Cube to Screen矩阵
	void homogenize(s_vector& y, s_vector& x);
	//归一化反转 
	void homogenize_reverse(s_vector& y, s_vector& x, float weight);
};

void transform_homogenize(s_vector& y, s_vector& x, float width, float height);

void transform_homogenize_reverse(s_vector& y, s_vector& x, float w, float width, float height);

int transform_check_cvv(s_vector& v);

typedef struct { s_vector pos; s_vector2f tc; s_color color; float rhw; s_vector normal; } vertex_t;
void vertex_rhw_init(vertex_t* v);

void vertex_interp(vertex_t* y, vertex_t* x1, vertex_t* x2, float t);

void vertex_division(vertex_t* y, const vertex_t* x1, const vertex_t* x2, float w);

void vertex_add(vertex_t* y, const vertex_t* x);

typedef struct { vertex_t v, v1, v2; }edge_t;
typedef struct { float top, bottom; edge_t left, right; } trapezoid_t;
typedef struct { vertex_t v, step; int x, y, w; } scanline_t;


//将三角形分割 
int trapezoid_init_triangle(trapezoid_t* trap, vertex_t* p1,
	vertex_t* p2, vertex_t* p3);

// 按照 Y 坐标计算出左右两条边纵坐标等于 Y 的顶点		
void trapezoid_edge_interp(trapezoid_t* trap, float y);

void trapezoid_init_scan_line(trapezoid_t* trap, scanline_t* scanline, int y);



#endif