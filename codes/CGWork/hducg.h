#include "matrix.h"

//构造二维平移矩阵
Matrix Translation(float tx, float ty);
//构造二维旋转矩阵
Matrix Rotation(float jiaodu);
//构造二维缩放矩阵
Matrix Scale(float sx, float sy);
//构造二维错切矩阵
Matrix Shear(float shx, float shy);

//平移多边形
void pingyi(Vector* juxing, int n, float tx, float ty);
//旋转多边形
void xuanzhuan(Vector* juxing, int n, float jiaodu);
//缩放多边形
void suofang(Vector* juxing, int n, float sx, float sy);
//错切多边形
void cuoqie(Vector* juxing, int n, float shx, float shy);