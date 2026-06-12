#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

/*向量类
使用示例：
1.定义向量对象，
方法一：指定向量长度，元素值默认初始化为0
    Vector vec(3);
方法二：给定向量元素值，元素数量至少2个
    Vector vec(0,0,1);
2.为向量元素赋值
    vec[0] = 7; vec[1] = 8; vec[2] = 9;
3.以只读方式获取第i个分量的值
    cout<<vec[i];
4.获取向量长度
    cout<<vec.size();
*/
class Vector {
private:
    std::vector<float> elements;

public:
    Vector(int size) : elements(size, 0) {}
    Vector(float x, float y) :elements(2) {
        elements[0] = x;
        elements[1] = y;
    }
    Vector(float x, float y, float w) :elements(3) {
        elements[0] = x;
        elements[1] = y;
        elements[2] = w;
    }

    float& operator[](int index) {
        return elements[index];
    }

    float operator[](int index) const {
        return elements[index];
    }

    int size() const {
        return elements.size();
    }
};

/*
矩阵类
使用示例：
1.定义矩阵对象，矩阵大小为2行3列
//   Matrix mat(2, 3);
2.为矩阵元素赋值
//  mat[0][0] = 1; mat[0][1] = 2; mat[0][2] = 3;
//  mat[1][0] = 4; mat[1][1] = 5; mat[1][2] = 6;
3.以只读方式获取矩阵元素
//  cout<<m[i][j]
4.矩阵乘向量
    Vector resultVec = mat * vec;
5.矩阵乘矩阵
    Matrix mat2(3, 2);
    mat2[0][0] = 1; mat2[0][1] = 2;
    mat2[1][0] = 3; mat2[1][1] = 4;
    mat2[2][0] = 5; mat2[2][1] = 6;

    Matrix resultMat = mat * mat2;
6.获取矩阵行数和列数
    cout<<mat.getRows();
    cout<<mat.getCols();
*/
class Matrix {
private:
    std::vector<std::vector<float> > elements;
    int rows, cols;

public:
Matrix(int rows, int cols) : rows(rows), cols(cols), elements(rows, std::vector<float>(cols, 0)) {}

    std::vector<float>& operator[](int index) {
        return elements[index];
    }

    const std::vector<float>& operator[](int index) const {
        return elements[index];
    }

    int getRows() const {
        return rows;
    }

    int getCols() const {
        return cols;
    }

    // 重载乘法运算符计算矩阵与向量的乘积
    Vector operator*(const Vector& v) const {
        if (cols != v.size()) {
            throw std::invalid_argument("Matrix columns must match vector size.");
        }

        Vector result(rows);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                result[i] += elements[i][j] * v[j];
            }
        }
        return result;
    }

    // 重载乘法运算符计算矩阵与矩阵的乘积
    Matrix operator*(const Matrix& m) const {
        if (cols != m.getRows()) {
            throw std::invalid_argument("Matrix A's columns must match Matrix B's rows.");
        }

        Matrix result(rows, m.getCols());
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < m.getCols(); ++j) {
                for (int k = 0; k < cols; ++k) {
                    result[i][j] += elements[i][k] * m[k][j];
                }
            }
        }
        return result;
    }
};
