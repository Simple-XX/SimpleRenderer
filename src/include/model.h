
// This file is a part of SimpleXX/SimpleRenderer
// (https://github.com/SimpleXX/SimpleRenderer).
//
// model.h for SimpleXX/SimpleRenderer.

#ifndef __MODEL_H__
#define __MODEL_H__

#include "vector"
#include "string"
#include "vector.hpp"

// 参考: http://read.pudn.com/downloads115/ebook/484936/OpenGL_wenjian.pdf

// v x y z
// 顶点坐标 v x坐标 y坐标 z坐标

// vt u v w
// 顶点在纹理图片中的对应点 vt 纹理图片对应的坐标

// vn x y z
// 顶点法向量 vn x分量 y分量 z分量

// f v/vt/vn v/vt/vn v/vt/vn
// f 顶点索引/纹理坐标索引/顶点法向量索引
// 面的法向量是由构成面的顶点对应的顶点法向量的做矢量和决定的（xyz的坐标分别相加再除以3得到的）

#define MAX 100

// 顶点
struct Vertex {
    int draw;
    // 顶点坐标
    float x;
    float y;
    float z;
    // 顶点对应的
    float nx;
    float ny;
    float nz;
    int   colorIndex;  // 颜色索引
    int   vertexlndex; // 顶点索引
    int   facets[90];  // 存面片数组
    int   facetsNum;   // 面片数
    int   edges[60];   // 存顶点数组
    int   edgesNum;    // 顶点数
};

// 面

// 颜色
struct ColorStruct {
    int   index;          //颜色索引
    float ra, ga, ba;     // 环境光的各分量
    float rd, gd, bd, ad; // 漫反射光的各分量
    float rs, gs, bs;     // 镜面反射光的各分量
    float spec;           // 镜面反射光的强度
};

// 材质
struct MaterialColor {
    char  name[100];
    float ra, ga, ba;     // 环境光的各分量
    float rd, gd, bd, ad; // 漫反射光的各分量
    float rs, gs, bs;     // 镜面反射光的各分量
    float spec;           // 镜面反射光的强度
};

// 模型
struct ModelContext {
    int         faceCount;       // 面的数量
    int         traggleFlag;     // 三角形标注
    Vertex      vertexList[MAX]; // 模型中的顶点列表
    int         vertexCount;     //顶点数量
    Vertex      lineList[MAX];   // 模型中的线列表
    int         lineCount;       //线数量
    Vertex      lineStripList[MAX];
    int         lineStripCount;
    int         edgeList[MAX][2];   // 边界列表
    int         edgeCount;          //边界数量
    Vertex      objVertexList[MAX]; // obj 文件中的顶点
    int         ovCount;            //顶点序数
    int         onCount;            //法向量序数
    ColorStruct colorList[MAX];     // 模型中的颜色列表
    int         ColorCounr;         //颜色数
    // etc.
};

class Model {
private:
    std::vector<Vectorf3>         verts;
    std::vector<std::vector<int>> faces;

public:
    Model(const std::string &_filename);
    ~Model(void);
    int              nverts(void) const;
    int              nfaces(void) const;
    Vectorf3         vert(int i) const;
    std::vector<int> face(int _idx) const;
};

#endif /* __MODEL_H__ */
