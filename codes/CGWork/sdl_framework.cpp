﻿#include <SDL.h>
#include <cmath>
#include <iostream>
#include "hducg.h"

//1.创建画布，定义画布的大小和像素数组
//将画布宽度和高度定义为全局变量
const int CANVAS_WIDTH = 800;
const int CANVAS_HEIGHT = 600;
//用于保存输出图像像素的数组，像素按行排列
//每个像素的颜色由一个32位无符号整数表示，由高到低4个字节分别代表RGBA
unsigned int* pixels = new unsigned int[CANVAS_WIDTH * CANVAS_HEIGHT];

//设置背景色
void clearPixels(unsigned int color) {
    for (int i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; i++)
        pixels[i] = color;
}

//设置像素[x,y]的颜色为给定的rgba值
void setPixel(int x, int y, unsigned int color) {
    if (x < 0 || x >= CANVAS_WIDTH || y < 0 || y >= CANVAS_HEIGHT)
        return;
    pixels[y * CANVAS_WIDTH + x] = color;
}

//交换两个整数
void swap(int& a, int& b) {
    a = a ^ b; // 第一步
    b = a ^ b; // 第二步
    a = a ^ b; // 第三步
}

//Bresenham算法绘制直线
void drawLine(int x1, int y1, int x2, int y2, unsigned int color) {
    int dx = x2 - x1, dy = y2 - y1;
    
    bool swapped = false;
    //如果斜率绝对值大于1，交换x和y
    if (abs(dx) < abs(dy))
    {
        swap(x1, y1);
        swap(x2, y2);
        swapped = true;
    }

    //如果起点在右，交换起点和终点
    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }
    
    //如果直线走向为从上到下，则y的增量为-1
    int ystep = 1;
    dx = x2 - x1, dy = y2 - y1;
    if (dy < 0) {
        ystep = -1;
        dy = -dy;
    } 

    //初始化
    int d = 2 * dy;
    int x = x1, y = y1;
    while (x <= x2) {
        //填充像素
        if(swapped)
            setPixel(y, x, color);
        else
            setPixel(x, y, color);
        x++; //更新x
        //更新y和d
        if (d > dx) {
            y = y + ystep;
            d -= 2*dx;
        }
        d += 2*dy;
    }
}

//TODO：绘制圆
//cx,cy：圆心坐标
//r：半径
void drawCircle(int cx, int cy, int r, unsigned int color) {

}

//TODO：绘制椭圆
//cx,cy：圆心坐标
//a：x轴半径
//b：y轴半径
void drawEllipse(int cx, int cy, int a, int b, unsigned color) {

}

//绘制五角星，坐标计算参考这篇文章：
//Python绘制五角星！ - 程序员的文章 - 知乎
//https://zhuanlan.zhihu.com/p/81119430
//cx,cy:中心点坐标
//r：外接圆半径
void drawStar(int cx, int cy, int r, unsigned int color) {
    float pi = acos(-1.0);
    int angleSin18 = sin(pi * 0.1) * r + 0.5;
    int angleCos18 = cos(pi * 0.1) * r + 0.5;
    int angleSin54 = sin(pi * 0.3) * r + 0.5;
    int angleCos54 = cos(pi * 0.3) * r + 0.5;

    drawLine(cx - angleCos18, cy + angleSin18, cx + angleCos18, cy + angleSin18, color);
    drawLine(cx + angleCos18, cy + angleSin18, cx - angleCos54, cy - angleSin54, color);
    drawLine(cx - angleCos54, cy - angleSin54, cx, cy + r, color);
    drawLine(cx, cy + r, cx + angleCos54, cy - angleSin54, color);
    drawLine(cx + angleCos54, cy - angleSin54, cx - angleCos18, cy + angleSin18, color);
}

//2. 创建场景，场景包含一个矩形，定义为全局变量
//用三维向量表示多边形顶点的齐次坐标
//每条边都是直线
Vector jxqc[4] = { Vector(0,0,1), Vector(80,0,1), Vector(80,40,1), Vector(0,40,1)};
Vector jxfqc[4] = { Vector(0,0), Vector(80,0), Vector(80,40), Vector(0,40) };

void drawPolygon(const Vector* polygon, int polygon_vertex_count, unsigned int color) {
    for (int vIndex = 0; vIndex < polygon_vertex_count; vIndex++)
    {
        int next_index = (vIndex + 1) % polygon_vertex_count;
        drawLine(polygon[vIndex][0], polygon[vIndex][1],
            polygon[next_index][0], polygon[next_index][1], color);
    }
}

//绘制坐标系，原点在左上角，水平x轴，竖直y轴
void drawCoordinate()
{
    unsigned int red = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 255, 0, 0, 255);
    for (int x = 0; x < CANVAS_WIDTH; x++) {
        pixels[x] = red;
        pixels[x + CANVAS_WIDTH] = red;
    }

    unsigned int green = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0, 255, 0, 255);
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        pixels[y * CANVAS_WIDTH] = green;
        pixels[y * CANVAS_WIDTH + 1] = green;
    }
}

//3. 光栅化场景，将图形转为像素
void drawScene() {
    //将背景色设置为灰色
    unsigned int background = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 128, 128, 128, 255);;
    clearPixels(background);

    drawCoordinate();

    //绘制五角星，位于窗口中心，外接圆半径为100
    unsigned int yellow = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 255, 255, 0, 255);
    drawStar(CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, 100, yellow);

    unsigned int purple = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 255, 0, 255, 255);
    //绘制圆
    drawCircle(CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2, 100, purple);
    //绘制椭圆
    drawEllipse(CANVAS_WIDTH / 4, CANVAS_HEIGHT / 4, 50, 75, purple);
    drawEllipse(CANVAS_WIDTH * 0.75, CANVAS_HEIGHT * 0.75, 75, 50, purple);

    //绘制多边形
    unsigned int blue = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 0, 0, 255, 255);
    drawPolygon(jxqc, 4, blue);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("create and display window", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    /*
    SDL_Renderer 是 SDL 中的一个用于在窗口上绘制图形的接口。
    它提供了一套函数用于绘制2D图形、处理纹理、进行渲染优化等功能。
    SDL_Renderer 支持硬件加速，通过它可以高效地在屏幕上渲染图像、形状和文本。
    */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    /*
    SDL_Texture 是 SDL 中用于存储和管理图像数据的对象。
    它代表了一个图像或纹理，可以被渲染器用来在窗口上绘制图像。
    SDL_Texture 支持硬件加速和不同的图像格式。
    */
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
        SDL_TEXTUREACCESS_STREAMING, CANVAS_WIDTH, CANVAS_HEIGHT);
    if (texture == nullptr) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //3. 光栅化场景
    drawScene();

    //4. 使用 SDL_UpdateTexture 更新纹理数据
    SDL_UpdateTexture(texture, NULL, pixels, CANVAS_WIDTH * sizeof(Uint32));

    bool running = true;
    SDL_Event event; 
    
    while (running) {
        //5. 处理交互事件
        while (SDL_PollEvent(&event)) {            
            //8. 关闭窗口
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            }
            // 鼠标事件
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    //6. 更新场景
                    //pingyi(jxfqc, 4, 1, 1); //非齐次平移
                    pingyi(jxqc, 4, 1, 1);     //齐次平移
                    //3. 光栅化场景
                    drawScene();
                    //4. 使用 SDL_UpdateTexture 更新纹理数据
                    SDL_UpdateTexture(texture, NULL, pixels, CANVAS_WIDTH * sizeof(Uint32));
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    //6. 更新场景
                    //pingyi(jxfqc, 4, 1, 1); //非齐次平移
                    pingyi(jxqc, 4, -1, -1);     //齐次平移
                    //3. 光栅化场景
                    drawScene();
                    //4. 使用 SDL_UpdateTexture 更新纹理数据
                    SDL_UpdateTexture(texture, NULL, pixels, CANVAS_WIDTH * sizeof(Uint32));
                }
            }
        }
        //4. 渲染纹理
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 延迟一小段时间，减少CPU占用
    }

    //8. 清理动态内存
    delete[] pixels;

    //清理渲染器、纹理和窗口，释放SDL资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
