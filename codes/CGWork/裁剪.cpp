#include "SDL.h"
#include "canvas.h"
#include <iostream>
#include <utility>
#include <algorithm>

typedef std::pair<int, int> Point;
typedef std::pair<Point, Point> Line;

// 定义裁剪区域
#define LEFT  1  // 1 000
#define RIGHT 2  // 0 010
#define BOTTOM 4 // 0 100
#define TOP 8    // 0 001

// 计算点的Outcode
int computeOutcode(int x, int y, int xmin, int xmax, int ymin, int ymax) {
    int code = 0;

    if (x < xmin)        // 点在矩形的左侧
        code |= LEFT;
    else if (x > xmax)   // 点在矩形的右侧
        code |= RIGHT;
    if (y < ymin)        // 点在矩形的下方
        code |= BOTTOM;
    else if (y > ymax)   // 点在矩形的上方
        code |= TOP;

    return code;
}

// Cohen-Sutherland 裁剪算法
Line clipLine(int xmin, int xmax, int ymin, int ymax, const Line& line) {
    int x1 = line.first.first;
    int y1 = line.first.second;
    int x2 = line.second.first;
    int y2 = line.second.second;

    int outcode1 = computeOutcode(x1, y1, xmin, xmax, ymin, ymax);
    int outcode2 = computeOutcode(x2, y2, xmin, xmax, ymin, ymax);

    bool accept = false;

    while (true) {
        if ((outcode1 | outcode2) == 0) { 
            // 两个端点都在矩形内，接受直线段
            accept = true;
            break;
        }
        else if ((outcode1 & outcode2) == 1) {
            // 两个端点在矩形外并且在相同方向，拒绝直线段
            break;
        }
        else {
            // 需要裁剪直线段
            int outcodeOut;
            int x, y;

            // 选择outcode为1的点进行裁剪
            if (outcode1 != 0) {
                outcodeOut = outcode1;
            }
            else {
                outcodeOut = outcode2;
            }

            // 计算裁剪点
            if (outcodeOut & TOP) {
                // 点在矩形的上方，裁剪到矩形的顶部
                x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
                y = ymax;
            }
            else if (outcodeOut & BOTTOM) {
                // 点在矩形的下方，裁剪到矩形的底部
                x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
                y = ymin;
            }
            else if (outcodeOut & RIGHT) {
                // 点在矩形的右侧，裁剪到矩形的右侧
                y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
                x = xmax;
            }
            else if (outcodeOut & LEFT) {
                // 点在矩形的左侧，裁剪到矩形的左侧
                y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
                x = xmin;
            }

            // 更新被裁剪的点
            if (outcodeOut == outcode1) {
                x1 = x;
                y1 = y;
                outcode1 = computeOutcode(x1, y1, xmin, xmax, ymin, ymax);
            }
            else {
                x2 = x;
                y2 = y;
                outcode2 = computeOutcode(x2, y2, xmin, xmax, ymin, ymax);
            }
        }
    }

    if (accept) {
        return Line(Point(x1, y1), Point(x2, y2));
    }
    else {
        return Line(Point(0, 0), Point(0, 0)); // 如果被裁剪掉，则返回无效线段
    }
}

int main(int argc, char* argv[]) {
    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL初始化失败:" << SDL_GetError();
        return 1;
    }
    
    // 创建窗口
    int w = 800, h = 600;
    SDL_Window* window = SDL_CreateWindow(
        "裁剪",           // 窗口标题
        SDL_WINDOWPOS_CENTERED,  // 窗口x位置
        SDL_WINDOWPOS_CENTERED,  // 窗口y位置
        w,                     // 窗口宽度
        h,                     // 窗口高度
        SDL_WINDOW_SHOWN         // 窗口标志
    );
    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 创建渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //创建画布
    Canvas canvas = Canvas(renderer, w, h);
    canvas.clear(100, 100, 100);
    
    //定义裁剪窗口
    int xmin = w/4, ymin = h/4, xmax = 3*w/4, ymax = 3*h/4;

    // 主循环标志
    bool running = true;
    SDL_Event event;
    
    bool drawing = false;
    std::vector<Line> lines;
    std::vector<Line> lines_accepted;
    int x1(0), y1(0), x2(0), y2(0);
    // 主循环
    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    drawing = true;
                    x1 = x2 = event.button.x;
                    y1 = y2 = event.button.y;
                }
                break;
            case SDL_MOUSEMOTION:
                if (drawing) {
                    x2 = event.button.x;
                    y2 = event.button.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (drawing && event.button.button == SDL_BUTTON_LEFT) {
                    drawing = false;
                    lines.push_back(Line(Point(x1,y1), Point(x2,y2)));
                }
                break;
                // 监听Enter键事件来清空线段
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_RETURN) {
                    lines_accepted.clear();
                    for (int i = 0; i < lines.size(); i++) {
                        lines_accepted.push_back(clipLine(xmin, xmax, ymin, ymax, lines[i]));
                    }
                }
                break;
            }            
        }
        canvas.clear(100, 100, 100);
        //窗口绘制为白色
        canvas.drawLine(xmin, ymin, xmin, ymax, 255, 255, 255);
        canvas.drawLine(xmin, ymin, xmax, ymin, 255, 255, 255);
        canvas.drawLine(xmax, ymax, xmax, ymin, 255, 255, 255);
        canvas.drawLine(xmax, ymax, xmin, ymax, 255, 255, 255);
        //绘制裁剪前线段
        for (int i = 0; i < lines.size(); i++) {
            canvas.drawLine(lines[i].first.first, lines[i].first.second,
                lines[i].second.first, lines[i].second.second, 255, 0, 0);
        }

        //绘制裁剪后线段
        for (int i = 0; i < lines_accepted.size(); i++) {
            Line line = lines_accepted[i];
            canvas.drawLine(line.first.first, line.first.second,
                line.second.first, line.second.second, 0, 255, 0);
        }
        
        if (drawing) {
            canvas.drawLine(x1, y1, x2, y2, 255, 0, 0);
        }        

        //绘制画布
        canvas.render(renderer);

        // 添加短暂延迟以减少CPU使用率
        SDL_Delay(16); // 约60FPS
    }        
    
    //清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
