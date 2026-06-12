#define _USE_MATH_DEFINES
#include "geometry.h"
#include "SDL.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texcoordIndices;
    std::vector<int> normalIndices;
};

struct Mesh {
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> texcoords;
    std::vector<Vec3f> normals;
    std::vector<Face> faces;
};
bool loadOBJ(const std::string& filename, Mesh& mesh);
uint32_t* rasterization();

Mesh mesh;

static const float inchToMm = 25.4;
enum FitResolutionGate { kFill = 0, kOverscan };

const uint32_t imageWidth = 640;
const uint32_t imageHeight = 480;
const Matrix44f worldToCamera = { 0.707107, -0.331295, 0.624695, 0, 0, 0.883452, 0.468521, 0, -0.707107, -0.331295, 0.624695, 0, -1.63871, -5.747777, -40.400412, 1 };

const float nearClippingPLane = 1;
const float farClippingPLane = 1000;
float focalLength = 50; // in mm
// 35mm Full Aperture in inches
float filmApertureWidth = 0.980;
float filmApertureHeight = 0.735;


int main(int argc, char* argv[]) {
    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL初始化失败:" << SDL_GetError();
        return 1;
    }

    // 创建窗口    
    SDL_Window* window = SDL_CreateWindow(
        "三维变换",           // 窗口标题
        SDL_WINDOWPOS_CENTERED,  // 窗口x位置
        SDL_WINDOWPOS_CENTERED,  // 窗口y位置
        imageWidth,                     // 窗口宽度
        imageHeight,                     // 窗口高度
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

    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        imageWidth, imageHeight);

    //加载obj模型
    if (loadOBJ("./assets/sphere.obj", mesh)) {
        std::cout << "OBJ 解析成功！" << std::endl;
        std::cout << "顶点数: " << mesh.vertices.size() << std::endl;
        std::cout << "面数: " << mesh.faces.size() << std::endl;
    }
    else {
        std::cout << "解析失败！" << std::endl;
    }
    //绘制场景
    uint32_t* frameBuffer = rasterization();
    //输出图像到屏幕
    int pitch = imageWidth * 4;
    SDL_UpdateTexture(texture, nullptr, frameBuffer, pitch);
    delete[] frameBuffer;

    // 主循环标志
    bool running = true;
    SDL_Event event;

    // 主循环
    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        // 添加短暂延迟以减少CPU使用率
        SDL_Delay(16); // 约60FPS
    }

    //清理资源    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

bool loadOBJ(const std::string& filename, Mesh& mesh) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            Vec3f v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (type == "vt") {
            Vec2f vt;
            iss >> vt.x >> vt.y;
            mesh.texcoords.push_back(vt);
        }
        else if (type == "vn") {
            Vec3f vn;
            iss >> vn.x >> vn.y >> vn.z;
            mesh.normals.push_back(vn);
        }
        else if (type == "f") {
            Face face;
            std::string vertexData;
            while (iss >> vertexData) {
                int vi = -1, ti = -1, ni = -1;
                size_t pos1 = vertexData.find('/');
                size_t pos2 = vertexData.find('/', pos1 == std::string::npos ? pos1 : pos1 + 1);

                if (pos1 == std::string::npos) {
                    // 仅有顶点索引
                    vi = std::stoi(vertexData);
                }
                else if (pos2 == std::string::npos) {
                    // 形如 v/vt
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ti = std::stoi(vertexData.substr(pos1 + 1));
                }
                else if (pos2 == pos1 + 1) {
                    // 形如 v//vn
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ni = std::stoi(vertexData.substr(pos2 + 1));
                }
                else {
                    // 形如 v/vt/vn
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ti = std::stoi(vertexData.substr(pos1 + 1, pos2 - pos1 - 1));
                    ni = std::stoi(vertexData.substr(pos2 + 1));
                }
                face.vertexIndices.push_back(vi - 1); // OBJ 索引从1开始
                face.texcoordIndices.push_back(ti - 1);
                face.normalIndices.push_back(ni - 1);
            }
            mesh.faces.push_back(face);
        }
    }

    file.close();
    return true;
}

//[comment]
// Compute screen coordinates based on a physically-based camera model
// http://www.scratchapixel.com/lessons/3d-basic-rendering/3d-viewing-pinhole-camera
//[/comment]
void computeScreenCoordinates(
    const float& filmApertureWidth,
    const float& filmApertureHeight,
    const uint32_t& imageWidth,
    const uint32_t& imageHeight,
    const FitResolutionGate& fitFilm,
    const float& nearClippingPLane,
    const float& focalLength,
    float& top, float& bottom, float& left, float& right
)
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

    switch (fitFilm) {
    default:
    case kFill:
        if (filmAspectRatio > deviceAspectRatio) {
            xscale = deviceAspectRatio / filmAspectRatio;
        }
        else {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        break;
    case kOverscan:
        if (filmAspectRatio > deviceAspectRatio) {
            yscale = filmAspectRatio / deviceAspectRatio;
        }
        else {
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
    const Vec3f& vertexWorld,
    const Matrix44f& worldToCamera,
    const float& l,
    const float& r,
    const float& t,
    const float& b,
    const float& near,
    const uint32_t& imageWidth,
    const uint32_t& imageHeight,
    Vec3f& vertexRaster
)
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

float min3(const float& a, const float& b, const float& c)
{
    return std::min(a, std::min(b, c));
}

float max3(const float& a, const float& b, const float& c)
{
    return std::max(a, std::max(b, c));
}

float edgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}


inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t a = 255;
    return (r << 24) | (g << 16) | (b << 8) | a;
}

uint32_t* rasterization()
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
    uint32_t* frameBuffer = new uint32_t[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i) frameBuffer[i] = RGB(255, 255, 255);
    float* depthBuffer = new float[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i) depthBuffer[i] = farClippingPLane;

    auto t_start = std::chrono::high_resolution_clock::now();

    // [comment]
    // Outer loop
    // [/comment]
    int ntris = mesh.faces.size();
    for (int fi = 0; fi < ntris; ++fi) {
        const Face& f = mesh.faces[fi];
        if (f.vertexIndices.size() < 3) continue;

        for (size_t k = 0; k + 2 < f.vertexIndices.size(); ++k) {
            int idx0 = f.vertexIndices[0];
            int idx1 = f.vertexIndices[k + 1];
            int idx2 = f.vertexIndices[k + 2];

            if (idx0 < 0 || idx1 < 0 || idx2 < 0) continue;
            if ((size_t)idx0 >= mesh.vertices.size() || (size_t)idx1 >= mesh.vertices.size() || (size_t)idx2 >= mesh.vertices.size()) continue;

            // 取出三角形三个顶点（世界坐标）
            const Vec3f& v0 = mesh.vertices[idx0];
            const Vec3f& v1 = mesh.vertices[idx1];
            const Vec3f& v2 = mesh.vertices[idx2];

            // 如果存在法线索引则取法线，否则用默认法线
            Vec3f n0 = {0,0,1}, n1 = {0,0,1}, n2 = {0,0,1};
            if (f.normalIndices.size() >= 1 && f.normalIndices[0] >= 0 && (size_t)f.normalIndices[0] < mesh.normals.size()) n0 = mesh.normals[f.normalIndices[0]];
            if (f.normalIndices.size() >= k+1 && f.normalIndices[k+1] >= 0 && (size_t)f.normalIndices[k+1] < mesh.normals.size()) n1 = mesh.normals[f.normalIndices[k+1]];
            if (f.normalIndices.size() >= k+2 && f.normalIndices[k+2] >= 0 && (size_t)f.normalIndices[k+2] < mesh.normals.size()) n2 = mesh.normals[f.normalIndices[k+2]];

            // 三角形转换与光栅化
            Vec3f v0Raster, v1Raster, v2Raster;
            convertToRaster(v0, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v0Raster);
            convertToRaster(v1, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v1Raster);
            convertToRaster(v2, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v2Raster);

            // 透视校正（1/z）
            v0Raster.z = 1.0f / v0Raster.z;
            v1Raster.z = 1.0f / v1Raster.z;
            v2Raster.z = 1.0f / v2Raster.z;

            // 法线随同 1/z 插值
            n0 *= v0Raster.z; n1 *= v1Raster.z; n2 *= v2Raster.z;
            
            // 计算三角形包围盒
            float xmin = min3(v0Raster.x, v1Raster.x, v2Raster.x);
            float ymin = min3(v0Raster.y, v1Raster.y, v2Raster.y);
            float xmax = max3(v0Raster.x, v1Raster.x, v2Raster.x);
            float ymax = max3(v0Raster.y, v1Raster.y, v2Raster.y);

            // 剔除视口外的三角形
            if (xmin > imageWidth - 1 || xmax < 0 || ymin > imageHeight - 1 || ymax < 0) continue;

            uint32_t x0 = std::max(int32_t(0), (int32_t)std::floor(xmin));
            uint32_t x1 = std::min(int32_t(imageWidth) - 1, (int32_t)std::floor(xmax));
            uint32_t y0 = std::max(int32_t(0), (int32_t)std::floor(ymin));
            uint32_t y1 = std::min(int32_t(imageHeight) - 1, (int32_t)std::floor(ymax));

            float area = edgeFunction(v0Raster, v1Raster, v2Raster);
            if (area == 0.0f) continue;

            for (uint32_t y = y0; y <= y1; ++y) {
                for (uint32_t x = x0; x <= x1; ++x) {
                    Vec3f pixelSample((float)x + 0.5f, (float)y + 0.5f, 0);
                    float w0 = edgeFunction(v1Raster, v2Raster, pixelSample);
                    float w1 = edgeFunction(v2Raster, v0Raster, pixelSample);
                    float w2 = edgeFunction(v0Raster, v1Raster, pixelSample);
                    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                        w0 /= area; w1 /= area; w2 /= area;
                        float oneOverZ = v0Raster.z * w0 + v1Raster.z * w1 + v2Raster.z * w2;
                        float z = 1.0f / oneOverZ;
                        if (z < depthBuffer[y * imageWidth + x]) {
                            depthBuffer[y * imageWidth + x] = z;
                            Vec3f n = n0 * w0 + n1 * w1 + n2 * w2;
                            n *= z; 
                            uint32_t color = RGB((uint8_t)(fabs(n.x) * 255), (uint8_t)(fabs(n.y) * 255), (uint8_t)(fabs(n.z) * 255));
                            frameBuffer[y * imageWidth + x] = color;
                        }
                    }
                }
            }
        } // end fan triangulation
    } // end face loop

    auto t_end = std::chrono::high_resolution_clock::now();
    auto passedTime = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    std::cerr << "Wall passed time: " << passedTime << "ms" << std::endl;

    delete[] depthBuffer;

    return frameBuffer;
}
