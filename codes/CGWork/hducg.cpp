#include "hducg.h"
#define M_PI 3.14159265358979323846

// 构造二维平移矩阵
Matrix Translation(float tx, float ty)
{
	Matrix t(3, 3);
	t[0][2] = tx;
	t[1][2] = ty;
	t[0][0] = t[1][1] = t[2][2] = 1;
	return t;
}

// TODO:构造二维旋转矩阵
Matrix Rotation(float jiaodu)
{
	Matrix m(3, 3);
	m[0][0] = m[1][1] = cos(jiaodu * M_PI / 180);
	m[0][1] = sin(jiaodu * M_PI / 180);
	m[1][0] = -sin(jiaodu * M_PI / 180);
	m[2][2] = 1;
	return m;
}

// TODO:构造二维缩放矩阵
Matrix Scale(float sx, float sy)
{
	Matrix m(3, 3);
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = 1;
	return m;
}

// TODO:构造二维错切矩阵
Matrix Shear(float shx, float shy)
{
	Matrix m(3, 3);
	m[0][0] = 1;
	m[0][1] = shx;
	m[1][0] = shy;
	m[1][1] = 1;
	m[2][2] = 1;
	return m;
}

// 平移多边形,n为顶点数
void pingyi(Vector *juxing, int n, float tx, float ty)
{
	// 非齐次坐标
	if (juxing[0].size() == 2)
	{
		for (int i = 0; i < n; i++)
		{
			juxing[i][0] += tx;
			juxing[i][1] += ty;
		}
	}
	// 齐次坐标
	else if (juxing[0].size() == 3)
	{
		Matrix m = Translation(tx, ty);
		for (int i = 0; i < n; i++)
		{
			juxing[i] = m * juxing[i];
		}
	}
}

// TODO:旋转多边形
void xuanzhuan(Vector *juxing, int n, float jiaodu)
{
	if (juxing[0].size() == 2)
	{
		for (int i = 0; i < n; i++)
		{
			float x = juxing[i][0];
			float y = juxing[i][1];
			juxing[i][0] = x * cos(jiaodu * M_PI / 180) - y * sin(jiaodu * M_PI / 180);
			juxing[i][1] = x * sin(jiaodu * M_PI / 180) + y * cos(jiaodu * M_PI / 180);
		}
	}
	else if (juxing[0].size() == 3)
	{
		Matrix m = Rotation(jiaodu);
		for (int i = 0; i < n; i++)
		{
			juxing[i] = m * juxing[i];
		}
	}
}

// TODO:缩放多边形
void suofang(Vector *juxing, int n, float sx, float sy)
{
	if (juxing[0].size() == 2)
	{
		for (int i = 0; i < n; i++)
		{
			juxing[i][0] *= sx;
			juxing[i][1] *= sy;
		}
	}
	else if (juxing[0].size() == 3)
	{
		Matrix m = Scale(sx, sy);
		for (int i = 0; i < n; i++)
		{
			juxing[i] = m * juxing[i];
		}
	}
}
// TODO:错切多边形
void cuoqie(Vector *juxing, int n, float shx, float shy)
{
	if (juxing[0].size() == 2)
	{
		for (int i = 0; i < n; i++)
		{
			float x = juxing[i][0];
			float y = juxing[i][1];
			juxing[i][0] = x + shx * y;
			juxing[i][1] = shy * x + y;
		}
	}
	else if (juxing[0].size() == 3)
	{
		Matrix m = Shear(shx, shy);
		for (int i = 0; i < n; i++)
		{
			juxing[i] = m * juxing[i];
		}
	}
}

int main(){
	char arr[3] = { 'a','b','c' };
	Vector* triangle = new Vector[3]{ Vector(2,4), Vector(4,4), Vector(4,1) };
	pingyi(triangle,3,-2.0,-4.0);
	xuanzhuan(triangle,3,60.0);
	pingyi(triangle,3,2.0,4.0);
	for (int i = 0; i < 3 ; i++) {
		std::cout << arr[i] << ":(" << triangle[i][0] << "," << triangle[i][1] << ")" << std::endl;
	}

	
	delete[] triangle;
	getchar();
	getchar();
	return 0;
}