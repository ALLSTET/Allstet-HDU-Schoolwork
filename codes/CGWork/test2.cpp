#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

struct Point {
    int x, y;
};

struct Edge {
    int ymax;       // 边的上端 y 值
    double x;       // 当前扫描线上的交点 x 值
    double invSlope; // 斜率的倒数
};

/*Step2:构建边表（ET）*/
#include <map>