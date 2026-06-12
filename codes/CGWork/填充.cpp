/*Step1:定义数据结构*/
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

struct Point
{
    int x, y;
};

struct Edge
{
    int ymax;        // 边的上端 y 值
    double x;        // 当前扫描线上的交点 x 值
    double invSlope; // 斜率的倒数
};

/*Step2:构建边表（ET）*/
#include <map>
map<int, vector<Edge>> buildEdgeTable(const vector<Point> &polygon)
{
    map<int, vector<Edge>> ET;
    int n = polygon.size();

    for (int i = 0; i < n; i++)
    {
        Point p1 = polygon[i];
        Point p2 = polygon[(i + 1) % n];

        // 忽略水平边
        if (p1.y == p2.y)
            continue;

        // 确保 p1.y < p2.y
        if (p1.y > p2.y)
            swap(p1, p2);

        Edge e;
        e.ymax = p2.y;
        e.x = p1.x;
        e.invSlope = double(p2.x - p1.x) / double(p2.y - p1.y);

        ET[p1.y].push_back(e);
    }
    return ET;
}

/*Step3:扫描线填充主逻辑*/
#include <set>
map<int, vector<int>> scanlineFill(const vector<Point> &polygon,
                                   int width, int height)
{
    auto ET = buildEdgeTable(polygon);

    map<int, vector<int>> result;
    int y_min = height, y_max = 0;
    for (auto &p : polygon)
    {
        if (p.y < y_min)
            y_min = p.y;
        if (p.y > y_max)
            y_max = p.y;
    }

    std::vector<Edge> AET;

    for (int y = y_min; y <= y_max; y++)
    {
        // 将当前 y 的边加入 AET
        if (ET.find(y) != ET.end())
        {
            for (auto &e : ET[y])
            {
                AET.push_back(e);
            }
        }

        // 删除已结束的边
        AET.erase(std::remove_if(AET.begin(), AET.end(),
                                 [y](Edge &e)
                                 { return e.ymax <= y; }),
                  AET.end());

        // 按 x 排序
        sort(AET.begin(), AET.end(),
             [](const Edge &a, const Edge &b)
             { return a.x < b.x; });

        // 成对填充
        for (size_t i = 0; i + 1 < AET.size(); i += 2)
        {
            int x_start = int(ceil(AET[i].x));
            int x_end = int(floor(AET[i + 1].x));
            for (int x = x_start; x <= x_end && x < width; x++)
            {
                if (x >= 0 && y >= 0 && y < height)
                    result[y].push_back(x);
            }
        }

        // 更新 AET 中每条边的 x
        for (auto &e : AET)
        {
            e.x += e.invSlope;
        }
    }

    return result;
}

#include <iomanip>
void printEdgeTable(const map<int, vector<Edge>> &ET)
{
    std::cout << "ET (ymin -> list of (ymax, x_start, invSlope))\n";
    for (auto &p : ET)
    {
        int ymin = p.first;
        std::cout << "y = " << ymin << " : ";
        for (const auto &e : p.second)
        {
            std::cout << "("
                      << "ymax=" << e.ymax << ", "
                      << "x=" << std::fixed << std::setprecision(4) << e.x << ", "
                      << "dx=" << std::fixed << std::setprecision(4) << e.invSlope
                      << ")  ";
        }
        std::cout << "\n";
    }
}

#include "SDL.h"
#include "canvas.h"
void test_edge_print();
int main(int argc, char *argv[])
{
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL初始化失败:" << SDL_GetError();
        return 1;
    }

    // 创建窗口
    int w = 800, h = 600;
    SDL_Window *window = SDL_CreateWindow(
        "Cutting",              // 窗口标题
        SDL_WINDOWPOS_CENTERED, // 窗口x位置
        SDL_WINDOWPOS_CENTERED, // 窗口y位置
        w,                      // 窗口宽度
        h,                      // 窗口高度
        SDL_WINDOW_SHOWN        // 窗口标志
    );
    if (!window)
    {
        std::cerr << "Window creating failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer creating failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 创建画布
    Canvas canvas = Canvas(renderer, w, h);
    canvas.clear(100, 100, 100);

    std::vector<Point> polygon = {
        {30, 10}, {150, 10},{80, 100}};
    map<int, vector<int>> result = scanlineFill(polygon, w, h);
    test_edge_print();

    for (const auto &pair : result)
    {
        int y = pair.first;
        for (const auto &x : pair.second)
        {
            canvas.setPixel(x, y, 0, 255, 0);
        }
    }

    // 主循环标志
    bool running = true;
    SDL_Event event;

    // 主循环
    while (running)
    {
        // 处理事件
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        // 绘制画布
        canvas.render(renderer);

        // 添加短暂延迟以减少CPU使用率
        SDL_Delay(16); // 约60FPS
    }

    // 清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void test_edge_print()
{
    std::vector<Point> polygon = {
      {3, 1},{15, 1},{8, 10}};
    auto ET = buildEdgeTable(polygon);
    printEdgeTable(ET);
}