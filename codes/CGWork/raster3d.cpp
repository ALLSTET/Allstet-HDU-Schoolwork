//[header]
// A practical implementation of the rasterization algorithm.
//[/header]
//[compile]
// Download the raster3d.cpp, cow.h and geometry.h files to the same folder.
// Open a shell/terminal, and run the following command where the files are saved:
//
// c++ -o raster3d raster3d.cpp  -std=c++11 -O3
//
// Run with: ./raster3d. Open the file ./output.png in Photoshop or any program
// reading PPM files.
//[/compile]
//[ignore]
// Copyright (C) 2012  www.scratchapixel.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//[/ignore]
#define _USE_MATH_DEFINES
#include "geometry.h"
#include <fstream>
#include <chrono>

#include "cow.h"

static const float inchToMm = 25.4;
enum FitResolutionGate
{
    kFill = 0,
    kOverscan
};

//[comment]
// Compute screen coordinates based on a physically-based camera model
// http://www.scratchapixel.com/lessons/3d-basic-rendering/3d-viewing-pinhole-camera
//[/comment]
void computeScreenCoordinates(
    const float &filmApertureWidth,
    const float &filmApertureHeight,
    const uint32_t &imageWidth,
    const uint32_t &imageHeight,
    const FitResolutionGate &fitFilm,
    const float &nearClippingPLane,
    const float &focalLength,
    float &top, float &bottom, float &left, float &right)
{
    float filmAspectRatio = filmApertureWidth / filmApertureHeight;
    float deviceAspectRatio = imageWidth / (float)imageHeight;

    top = ((filmApertureHeight * inchToMm / 2) / focalLength) * nearClippingPLane;
    right = ((filmApertureWidth * inchToMm / 2) / focalLength) * nearClippingPLane;

    // field of view (horizontal)
    float fov = 2 * 180 / M_PI * atan((filmApertureWidth * inchToMm / 2) / focalLength);
    std::cerr << "Field of view " << fov << std::endl;

    float xscale = 1;
    float yscale = 1;

    switch (fitFilm)
    {
    default:
    case kFill:
        if (filmAspectRatio > deviceAspectRatio)
        {
            xscale = deviceAspectRatio / filmAspectRatio;
        }
        else
        {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        break;
    case kOverscan:
        if (filmAspectRatio > deviceAspectRatio)
        {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        else
        {
            xscale = deviceAspectRatio / filmAspectRatio;
        }
        break;
    }

    right *= xscale;
    top *= yscale;

    bottom = -top;
    left = -right;
}

//[comment]
// Compute vertex raster screen coordinates.
// Vertices are defined in world space. They are then converted to camera space,
// then to NDC space (in the range [-1,1]) and then to raster space.
// The z-coordinates of the vertex in raster space is set with the z-coordinate
// of the vertex in camera space.
//[/comment]
void convertToRaster(
    const Vec3f &vertexWorld,
    const Matrix44f &worldToCamera,
    const float &l,
    const float &r,
    const float &t,
    const float &b,
    const float &near,
    const uint32_t &imageWidth,
    const uint32_t &imageHeight,
    Vec3f &vertexRaster)
{
    Vec3f vertexCamera;

    worldToCamera.multVecMatrix(vertexWorld, vertexCamera);

    // convert to screen space
    Vec2f vertexScreen;
    vertexScreen.x = near * vertexCamera.x / -vertexCamera.z;
    vertexScreen.y = near * vertexCamera.y / -vertexCamera.z;

    // now convert point from screen space to NDC space (in range [-1,1])
    Vec2f vertexNDC;
    vertexNDC.x = 2 * vertexScreen.x / (r - l) - (r + l) / (r - l);
    vertexNDC.y = 2 * vertexScreen.y / (t - b) - (t + b) / (t - b);

    // convert to raster space
    vertexRaster.x = (vertexNDC.x + 1) / 2 * imageWidth;
    // in raster space y is down so invert direction
    vertexRaster.y = (1 - vertexNDC.y) / 2 * imageHeight;
    vertexRaster.z = -vertexCamera.z;
}

float min3(const float &a, const float &b, const float &c)
{
    return std::min(a, std::min(b, c));
}

float max3(const float &a, const float &b, const float &c)
{
    return std::max(a, std::max(b, c));
}

float edgeFunction(const Vec3f &a, const Vec3f &b, const Vec3f &c)
{
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

const uint32_t imageWidth = 640;
const uint32_t imageHeight = 480;
const Matrix44f worldToCamera = {0.707107, -0.331295, 0.624695, 0, 0, 0.883452, 0.468521, 0, -0.707107, -0.331295, 0.624695, 0, -1.63871, -5.747777, -40.400412, 1};

const uint32_t ntris = 3156;
const float nearClippingPLane = 1;
const float farClippingPLane = 1000;
float focalLength = 20; // in mm
// 35mm Full Aperture in inches
float filmApertureWidth = 0.980;
float filmApertureHeight = 0.735;

inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t a = 255;
    return (r << 24) | (g << 16) | (b << 8) | a;
}

bool isBackface(const Vec3f &v0, const Vec3f &v1, const Vec3f &v2, const Vec3f &viewDir)
{
    Vec3f edge1 = v1 - v0;
    Vec3f edge2 = v2 - v0;
    Vec3f normal = edge1.crossProduct(edge2);
    normal.normalize();
    return normal.dotProduct(viewDir) > 0;
}

// --- 新增：相机交互与裁剪参数（可调） ---
static Vec3f g_camPos = {0.0f, 0.0f, 0.0f}; // 相机在世界坐标系中的位置偏移（把场景当作相对于相机移动）
static float g_camYaw = 0.0f;               // 绕 Y 轴旋转（弧度）
static float g_camPitch = 0.0f;             // 绕 X 轴旋转（弧度）
static bool g_enableClipping = false;       // 是否开启像素裁剪窗口
static int g_clipX = 120;                   // 裁剪窗口左上角 x（像素）
static int g_clipY = 80;                    // 裁剪窗口左上角 y（像素）
static int g_clipW = 400;                   // 裁剪窗口宽度（像素）
static int g_clipH = 320;                   // 裁剪窗口高度（像素）

// 小型向量/旋转辅助（局部添加，直接在本文件使用）
static inline Vec3f rotateY(const Vec3f &v, float ang)
{
    float c = std::cos(ang), s = std::sin(ang);
    return Vec3f{v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
}
static inline Vec3f rotateX(const Vec3f &v, float ang)
{
    float c = std::cos(ang), s = std::sin(ang);
    return Vec3f{v.x, v.y * c - v.z * s, v.y * s + v.z * c};
}
// --- 新增 end ---

uint32_t *rasterization()
{
    Matrix44f cameraToWorld = worldToCamera.inverse();

    // compute screen coordinates
    float t, b, l, r;

    computeScreenCoordinates(
        filmApertureWidth, filmApertureHeight,
        imageWidth, imageHeight,
        kOverscan,
        nearClippingPLane,
        focalLength,
        t, b, l, r);

    // define the frame-buffer and the depth-buffer. Initialize depth buffer
    // to far clipping plane.
    uint32_t *frameBuffer = new uint32_t[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i)
        frameBuffer[i] = RGB(255, 255, 255);
    float *depthBuffer = new float[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i)
        depthBuffer[i] = farClippingPLane;

    auto t_start = std::chrono::high_resolution_clock::now();

    // [comment]
    // Outer loop
    // [/comment]
    for (uint32_t i = 0; i < ntris; ++i)
    {
        // 读取原始世界顶点
        Vec3f v0w = vertices[nvertices[i * 3]];
        Vec3f v1w = vertices[nvertices[i * 3 + 1]];
        Vec3f v2w = vertices[nvertices[i * 3 + 2]];

        // --- 在投影前应用用户控制的相机位移与旋转（把世界点转换到以相机为中心的坐标） ---
        // 平移（把世界点按相机位置做逆移）
        Vec3f v0t = Vec3f{v0w.x - g_camPos.x, v0w.y - g_camPos.y, v0w.z - g_camPos.z};
        Vec3f v1t = Vec3f{v1w.x - g_camPos.x, v1w.y - g_camPos.y, v1w.z - g_camPos.z};
        Vec3f v2t = Vec3f{v2w.x - g_camPos.x, v2w.y - g_camPos.y, v2w.z - g_camPos.z};
        // 先俯仰（绕X），再偏航（绕Y）——因为我们对世界点做逆相机旋转
        v0t = rotateX(v0t, -g_camPitch);
        v0t = rotateY(v0t, -g_camYaw);
        v1t = rotateX(v1t, -g_camPitch);
        v1t = rotateY(v1t, -g_camYaw);
        v2t = rotateX(v2t, -g_camPitch);
        v2t = rotateY(v2t, -g_camYaw);
        // 然后继续原来的 worldToCamera 变换（保留原有矩阵的视角/坐标系处理）
        Vec3f v0Cam, v1Cam, v2Cam;
        worldToCamera.multVecMatrix(v0t, v0Cam);
        worldToCamera.multVecMatrix(v1t, v1Cam);
        worldToCamera.multVecMatrix(v2t, v2Cam);
        // --- end 相机变换 ---

        Vec3f viewDirection(0, 0, -1); // camera looks down -z axis in camera space
        if (isBackface(v0Cam, v1Cam, v2Cam, viewDirection))
        {
            continue;
        }

        // [comment]
        // Convert the vertices of the triangle to raster space
        // [/comment]
        Vec3f v0Raster, v1Raster, v2Raster;
        convertToRaster(v0t, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v0Raster);
        convertToRaster(v1t, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v1Raster);
        convertToRaster(v2t, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v2Raster);

        // [comment]
        // Precompute reciprocal of vertex z-coordinate
        // [/comment]
        v0Raster.z = 1.0f / v0Raster.z;
        v1Raster.z = 1.0f / v1Raster.z;
        v2Raster.z = 1.0f / v2Raster.z;

        // [comment]
        // Prepare vertex attributes. Divde them by their vertex z-coordinate
        // (though we use a multiplication here because v.z = 1 / v.z)
        // [/comment]
        Vec2f st0 = st[stindices[i * 3]];
        Vec2f st1 = st[stindices[i * 3 + 1]];
        Vec2f st2 = st[stindices[i * 3 + 2]];

        st0 *= v0Raster.z, st1 *= v1Raster.z, st2 *= v2Raster.z;

        float xmin = min3(v0Raster.x, v1Raster.x, v2Raster.x);
        float ymin = min3(v0Raster.y, v1Raster.y, v2Raster.y);
        float xmax = max3(v0Raster.x, v1Raster.x, v2Raster.x);
        float ymax = max3(v0Raster.y, v1Raster.y, v2Raster.y);

        // the triangle is out of screen
        if (xmin > imageWidth - 1 || xmax < 0 || ymin > imageHeight - 1 || ymax < 0)
            continue;

        // be careful xmin/xmax/ymin/ymax can be negative. Don't cast to uint32_t
        uint32_t x0 = std::max(int32_t(0), (int32_t)(std::floor(xmin)));
        uint32_t x1 = std::min(int32_t(imageWidth) - 1, (int32_t)(std::floor(xmax)));
        uint32_t y0 = std::max(int32_t(0), (int32_t)(std::floor(ymin)));
        uint32_t y1 = std::min(int32_t(imageHeight) - 1, (int32_t)(std::floor(ymax)));

        float area = edgeFunction(v0Raster, v1Raster, v2Raster);
        if (area == 0.0f)
            continue;

        // [comment]
        // Inner loop
        // [/comment]
        for (uint32_t y = y0; y <= y1; ++y)
        {
            for (uint32_t x = x0; x <= x1; ++x)
            {
                // 如果开启裁剪窗口且像素不在裁剪区域内则跳过写入（执行简单的像素裁剪）
                if (g_enableClipping)
                {
                    if (x < g_clipX || x >= g_clipX + g_clipW || y < g_clipY || y >= g_clipY + g_clipH)
                        continue;
                }

                Vec3f pixelSample((float)x + 0.5f, (float)y + 0.5f, 0);
                float w0 = edgeFunction(v1Raster, v2Raster, pixelSample);
                float w1 = edgeFunction(v2Raster, v0Raster, pixelSample);
                float w2 = edgeFunction(v0Raster, v1Raster, pixelSample);
                if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                {
                    w0 /= area;
                    w1 /= area;
                    w2 /= area;
                    float oneOverZ = v0Raster.z * w0 + v1Raster.z * w1 + v2Raster.z * w2;
                    float z = 1.0f / oneOverZ;
                    // [comment]
                    // Depth-buffer test
                    // [/comment]
                    if (z < depthBuffer[y * imageWidth + x])
                    {
                        depthBuffer[y * imageWidth + x] = z;

                        Vec2f st = st0 * w0 + st1 * w1 + st2 * w2;

                        st *= z;

                        // [comment]
                        // If you need to compute the actual position of the shaded
                        // point in camera space. Proceed like with the other vertex attribute.
                        // Divide the point coordinates by the vertex z-coordinate then
                        // interpolate using barycentric coordinates and finally multiply
                        // by sample depth.
                        // [/comment]
                        Vec3f v0Cam, v1Cam, v2Cam;
                        worldToCamera.multVecMatrix(v0t, v0Cam);
                        worldToCamera.multVecMatrix(v1t, v1Cam);
                        worldToCamera.multVecMatrix(v2t, v2Cam);

                        float px = (v0Cam.x / -v0Cam.z) * w0 + (v1Cam.x / -v1Cam.z) * w1 + (v2Cam.x / -v2Cam.z) * w2;
                        float py = (v0Cam.y / -v0Cam.z) * w0 + (v1Cam.y / -v1Cam.z) * w1 + (v2Cam.y / -v2Cam.z) * w2;

                        Vec3f pt(px * z, py * z, -z); // pt is in camera space

                        // [comment]
                        // Compute the face normal which is used for a simple facing ratio.
                        // Keep in mind that we are doing all calculation in camera space.
                        // Thus the view direction can be computed as the point on the object
                        // in camera space minus Vec3f(0), the position of the camera in camera
                        // space.
                        // [/comment]
                        Vec3f n = (v1Cam - v0Cam).crossProduct(v2Cam - v0Cam);
                        n.normalize();
                        Vec3f viewDirection = -pt;
                        viewDirection.normalize();

                        float nDotView = std::max(0.f, n.dotProduct(viewDirection));

                        // [comment]
                        // The final color is the reuslt of the faction ration multiplied by the
                        // checkerboard pattern.
                        // [/comment]
                        const int M = 10;
                        float checker = (fmod(st.x * M, 1.0) > 0.5) ^ (fmod(st.y * M, 1.0) < 0.5);
                        float c = 0.3 * (1 - checker) + 0.7 * checker;
                        nDotView *= c;
                        // write pixel:
                        uint32_t color = RGB(nDotView * 255, nDotView * 255, nDotView * 255);
                        frameBuffer[y * imageWidth + x] = color;
                    }
                }
            }
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    auto passedTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    std::cerr << "Wall passed time:  " << passedTime << " ms" << std::endl;

    delete[] depthBuffer;

    return frameBuffer;
}

#include "SDL.h"
int main(int argc, char *argv[])
{
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cout << "SDL初始化失败:" << SDL_GetError();
        return 1;
    }

    // 创建窗口
    SDL_Window *window = SDL_CreateWindow(
        "三维变换",             // 窗口标题
        SDL_WINDOWPOS_CENTERED, // 窗口x位置
        SDL_WINDOWPOS_CENTERED, // 窗口y位置
        imageWidth,             // 窗口宽度
        imageHeight,            // 窗口高度
        SDL_WINDOW_SHOWN        // 窗口标志
    );
    if (!window)
    {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
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
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             imageWidth, imageHeight);

    // 首次渲染
    uint32_t *frameBuffer = rasterization();
    int pitch = imageWidth * 4;
    SDL_UpdateTexture(texture, nullptr, frameBuffer, pitch);
    delete[] frameBuffer;

    // 主循环标志
    bool running = true;
    SDL_Event event;

    bool needRedraw = false;
    const float moveStep = 0.2f;
    const float rotStep = 0.05f; // radians

    // 主循环
    while (running)
    {
        // 处理事件
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    g_camPos.z -= moveStep;
                    needRedraw = true;
                    break;
                case SDLK_s:
                    g_camPos.z += moveStep;
                    needRedraw = true;
                    break;
                case SDLK_a:
                    g_camPos.x -= moveStep;
                    needRedraw = true;
                    break;
                case SDLK_d:
                    g_camPos.x += moveStep;
                    needRedraw = true;
                    break;
                case SDLK_q:
                    g_camPos.y += moveStep;
                    needRedraw = true;
                    break;
                case SDLK_e:
                    g_camPos.y -= moveStep;
                    needRedraw = true;
                    break;
                case SDLK_LEFT:
                    g_camYaw -= rotStep;
                    needRedraw = true;
                    break;
                case SDLK_RIGHT:
                    g_camYaw += rotStep;
                    needRedraw = true;
                    break;
                case SDLK_UP:
                    g_camPitch += rotStep;
                    needRedraw = true;
                    break;
                case SDLK_DOWN:
                    g_camPitch -= rotStep;
                    needRedraw = true;
                    break;
                case SDLK_c:
                    g_enableClipping = !g_enableClipping;
                    needRedraw = true;
                    break;
                case SDLK_EQUALS:
                case SDLK_PLUS:
                    focalLength *= 1.1f;
                    needRedraw = true;
                    break;
                case SDLK_MINUS:
                    focalLength /= 1.1f;
                    needRedraw = true;
                    break;
                default:
                    break;
                }
            }
        }
        // 若相机参数或裁剪状态改变，重新 rasterize 并更新纹理
        if (needRedraw)
        {
            uint32_t *newBuffer = rasterization();
            SDL_UpdateTexture(texture, nullptr, newBuffer, pitch);
            delete[] newBuffer;
            needRedraw = false;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // 添加短暂延迟以减少CPU使用率
        SDL_Delay(16); // 约60FPS
    }

    // 清理资源
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}