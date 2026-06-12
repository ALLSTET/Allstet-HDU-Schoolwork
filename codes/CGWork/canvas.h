#pragma once

#include <SDL.h>
#include <vector>
#include "geometry.h"

// 交换两个整数
void swap(int &a, int &b)
{
    a = a ^ b; // 第一步
    b = a ^ b; // 第二步
    a = a ^ b; // 第三步
}

class Canvas{
private:
    SDL_Texture *texture; // 用于保存图像信息的对象
    int width, height;    // 图像的宽度和高度
    Uint32 *pixelData;    // 像素数据，保存像素的rgba值

public:
    Canvas(SDL_Renderer *renderer, int w, int h) : width(w), height(h)
    {
        // 创建流式纹理
        texture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width, height);

        // 分配像素数据缓冲区
        pixelData = new Uint32[width * height];

        // 初始化为黑色
        clear(0, 0, 0);
    }

    ~Canvas()
    {
        SDL_DestroyTexture(texture);
        delete[] pixelData;
    }

    // 设置单个像素颜色
    void setPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            pixelData[y * width + x] = (r << 24) | (g << 16) | (b << 8) | a;
        }
    }

    // 清除纹理为指定颜色
    void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
    {
        Uint32 color = (r << 24) | (g << 16) | (b << 8) | a;
        for (int i = 0; i < width * height; i++)
        {
            pixelData[i] = color;
        }
    }

    // 更新纹理
    void updateTexture()
    {
        void *pixels;
        int pitch;

        // 锁定纹理以进行更新
        if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0)
        {
            // 复制像素数据到纹理
            memcpy(pixels, pixelData, width * height * sizeof(Uint32));
            SDL_UnlockTexture(texture);
        }
    }

    // 渲染纹理
    void render(SDL_Renderer *renderer)
    {
        updateTexture();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // 画线
    void drawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b)
    {
        int dx = x2 - x1, dy = y2 - y1;

        bool swapped = false;
        // 如果斜率绝对值大于1，交换x和y
        if (abs(dx) < abs(dy))
        {
            swap(x1, y1);
            swap(x2, y2);
            swapped = true;
        }

        // 如果起点在右，交换起点和终点
        if (x1 > x2)
        {
            swap(x1, x2);
            swap(y1, y2);
        }

        // 如果直线走向为从上到下，则y的增量为-1
        int ystep = 1;
        dx = x2 - x1, dy = y2 - y1;
        if (dy < 0)
        {
            ystep = -1;
            dy = -dy;
        }

        // 初始化
        int d = 2 * dy;
        int x = x1, y = y1;
        while (x <= x2)
        {
            // 填充像素
            if (swapped)
                setPixel(y, x, r, g, b);
            else
                setPixel(x, y, r, g, b);
            x++; // 更新x
            // 更新y和d
            if (d > dx)
            {
                y = y + ystep;
                d -= 2 * dx;
            }
            d += 2 * dy;
        }
    }

    // 绘制三角形
    void drawTriangles(const std::vector<Vec2i> &vertices, Uint8 r, Uint8 g, Uint8 b)
    {
        for (int i = 0; i < vertices.size(); i += 3)
        {
            drawLine(vertices[i].x, vertices[i].y, vertices[i + 1].x, vertices[i + 1].y, r, g, b);
            drawLine(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 2].x, vertices[i + 2].y, r, g, b);
            drawLine(vertices[i + 2].x, vertices[i + 2].y, vertices[i].x, vertices[i].y, r, g, b);
        }
    }
};