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

struct Face
{
    std::vector<int> vertexIndices;
    std::vector<int> texcoordIndices;
    std::vector<int> normalIndices;
};

struct Mesh
{
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> texcoords;
    std::vector<Vec3f> normals;
    std::vector<Face> faces;
};
bool loadOBJ(const std::string &filename, Mesh &mesh);
uint32_t *rasterization();

Mesh mesh;

static const float inchToMm = 25.4;
enum FitResolutionGate
{
    kFill = 0,
    kOverscan
};

const uint32_t imageWidth = 640;
const uint32_t imageHeight = 480;
const Matrix44f worldToCamera = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, -4.52, 1};

const float nearClippingPLane = 1;
const float farClippingPLane = 1000;
float focalLength = 20; // in mm
// 35mm Full Aperture in inches
float filmApertureWidth = 0.980;
float filmApertureHeight = 0.735;

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
        "3D Transformation",    // 窗口标题
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
        std::cerr << "Renderer creating failed!: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             imageWidth, imageHeight);

    // 加载obj模型
    if (loadOBJ("assets/sphere2.obj", mesh))
    {
        std::cout << "OBJ 解析成功！" << std::endl;
        std::cout << "顶点数: " << mesh.vertices.size() << std::endl;
        std::cout << "面数: " << mesh.faces.size() << std::endl;
    }
    else
    {
        std::cout << "解析失败！" << std::endl;
    }
    // 绘制场景
    uint32_t *frameBuffer = rasterization();
    // 输出图像到屏幕
    int pitch = imageWidth * 4;
    SDL_UpdateTexture(texture, nullptr, frameBuffer, pitch);
    delete[] frameBuffer;

    // 主循环标志
    bool running = true;
    SDL_Event event;

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

bool loadOBJ(const std::string &filename, Mesh &mesh)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v")
        {
            Vec3f v;
            iss >> v.x >> v.y >> v.z;
            mesh.vertices.push_back(v);
        }
        else if (type == "vt")
        {
            Vec2f vt;
            iss >> vt.x >> vt.y;
            mesh.texcoords.push_back(vt);
        }
        else if (type == "vn")
        {
            Vec3f vn;
            iss >> vn.x >> vn.y >> vn.z;
            mesh.normals.push_back(vn);
        }
        else if (type == "f")
        {
            Face face;
            std::string vertexData;
            while (iss >> vertexData)
            {
                int vi = -1, ti = -1, ni = -1;
                size_t pos1 = vertexData.find('/');
                size_t pos2 = vertexData.find('/', pos1 == std::string::npos ? pos1 : pos1 + 1);

                if (pos1 == std::string::npos)
                {
                    // 仅有顶点索引
                    vi = std::stoi(vertexData);
                }
                else if (pos2 == std::string::npos)
                {
                    // 形如 v/vt
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ti = std::stoi(vertexData.substr(pos1 + 1));
                }
                else if (pos2 == pos1 + 1)
                {
                    // 形如 v//vn
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ni = std::stoi(vertexData.substr(pos2 + 1));
                }
                else
                {
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

inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t a = 255;
    return (r << 24) | (g << 16) | (b << 8) | a;
}

uint32_t *rasterization()
{
    // --- Lights & material ---
    // Ambient ambient intensity
    Vec3f Ia{0.1f, 0.1f, 0.1f};
    // Point light in world space (example). 调整位置以得到期望效果
    Vec3f pointLightWorld{0.0f, 5.0f, 0.0f};
    // Light color / intensity
    Vec3f Il{1.0f, 0.5f, 1.0f};
    // Attenuation coefficients (constant, linear, quadratic)
    float att_c = 1.0f, att_l = 0.09f, att_q = 0.032f;

    // Material
    Vec3f Ka{0.1f, 0.1f, 0.1f};
    Vec3f Kd{0.8f, 0.8f, 0.8f};
    Vec3f Ks{0.5f, 0.5f, 0.5f};
    float shininess = 64.0f;

    // Convert point light pos to camera space once
    Vec3f pointLightCam;
    worldToCamera.multVecMatrix(pointLightWorld, pointLightCam);

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

    // ---------------------------------------------------------
    // 计算每个顶点的法向（通过累加该顶点所属三角形的面法向）
    // 法向在 camera space 中累加/归一化
    // ---------------------------------------------------------
    std::vector<Vec3f> vertexNormalsCam(mesh.vertices.size(), Vec3f(0, 0, 0));
    std::vector<int> adjCount(mesh.vertices.size(), 0);

    for (size_t fi = 0; fi < mesh.faces.size(); ++fi)
    {
        const Face &f = mesh.faces[fi];
        if (f.vertexIndices.size() < 3)
            continue;
        int i0 = f.vertexIndices[0];
        int i1 = f.vertexIndices[1];
        int i2 = f.vertexIndices[2];
        Vec3f v0Cam, v1Cam, v2Cam;
        worldToCamera.multVecMatrix(mesh.vertices[i0], v0Cam);
        worldToCamera.multVecMatrix(mesh.vertices[i1], v1Cam);
        worldToCamera.multVecMatrix(mesh.vertices[i2], v2Cam);
        Vec3f faceN = (v1Cam - v0Cam).crossProduct(v2Cam - v0Cam);
        faceN.normalize();
        vertexNormalsCam[i0] = vertexNormalsCam[i0] + faceN;
        adjCount[i0]++;
        vertexNormalsCam[i1] = vertexNormalsCam[i1] + faceN;
        adjCount[i1]++;
        vertexNormalsCam[i2] = vertexNormalsCam[i2] + faceN;
        adjCount[i2]++;
    }
    for (size_t vi = 0; vi < vertexNormalsCam.size(); ++vi)
    {
        if (adjCount[vi] > 0)
            vertexNormalsCam[vi].normalize();
        else if (vi < mesh.normals.size())
        {
            vertexNormalsCam[vi] = mesh.normals[vi];
            vertexNormalsCam[vi].normalize();
        }
        else
            vertexNormalsCam[vi] = Vec3f(0, 0, 1);
    }

    // framebuffer / depthbuffer
    uint32_t *frameBuffer = new uint32_t[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i)
        frameBuffer[i] = RGB(255, 255, 255);
    float *depthBuffer = new float[imageWidth * imageHeight];
    for (uint32_t i = 0; i < imageWidth * imageHeight; ++i)
        depthBuffer[i] = farClippingPLane;

    auto t_start = std::chrono::high_resolution_clock::now();

    // 逐三角形光栅化（每像素插值法线并单独计算光照）
    int ntris = mesh.faces.size();
    for (int ti = 0; ti < ntris; ++ti)
    {
        const Face &face = mesh.faces[ti];
        if (face.vertexIndices.size() < 3)
            continue;
        int i0 = face.vertexIndices[0];
        int i1 = face.vertexIndices[1];
        int i2 = face.vertexIndices[2];

        const Vec3f &wv0 = mesh.vertices[i0];
        const Vec3f &wv1 = mesh.vertices[i1];
        const Vec3f &wv2 = mesh.vertices[i2];

        // raster space
        Vec3f v0Raster, v1Raster, v2Raster;
        convertToRaster(wv0, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v0Raster);
        convertToRaster(wv1, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v1Raster);
        convertToRaster(wv2, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v2Raster);

        // camera space positions (for backface)
        Vec3f v0Cam, v1Cam, v2Cam;
        worldToCamera.multVecMatrix(wv0, v0Cam);
        worldToCamera.multVecMatrix(wv1, v1Cam);
        worldToCamera.multVecMatrix(wv2, v2Cam);
        Vec3f faceN = (v1Cam - v0Cam).crossProduct(v2Cam - v0Cam);
        faceN.normalize();

        // Backface cull — compute view vector from triangle center to camera (camera at origin in camera space)
        Vec3f triCenterCam = (v0Cam + v1Cam + v2Cam) * (1.0f / 3.0f);
        Vec3f viewVec = Vec3f(0.0f, 0.0f, 0.0f) - triCenterCam; // from triangle toward camera
        float viewLen = viewVec.length();
        if (viewLen > 1e-6f)
            viewVec *= (1.0f / viewLen);
        // If face normal points away from view vector, cull
        if (faceN.dotProduct(viewVec) <= 0.0f)
            continue;

        // 顶点法线（camera space）
        Vec3f n0 = vertexNormalsCam[i0];
        Vec3f n1 = vertexNormalsCam[i1];
        Vec3f n2 = vertexNormalsCam[i2];

        // 透视修正准备：把 raster.z 设为 1/z
        v0Raster.z = 1.0f / v0Raster.z;
        v1Raster.z = 1.0f / v1Raster.z;
        v2Raster.z = 1.0f / v2Raster.z;

        // 顶点法线乘以 1/z（用于透视修正插值）
        Vec3f n0_p = n0 * v0Raster.z;
        Vec3f n1_p = n1 * v1Raster.z;
        Vec3f n2_p = n2 * v2Raster.z;

        // 顶点 camera-space 位置乘以 1/z（用于透视修正插值以重构片元空间位置）
        Vec3f p0_p = v0Cam * v0Raster.z;
        Vec3f p1_p = v1Cam * v1Raster.z;
        Vec3f p2_p = v2Cam * v2Raster.z;

        // 三角形 bbox
        float xmin = min3(v0Raster.x, v1Raster.x, v2Raster.x);
        float ymin = min3(v0Raster.y, v1Raster.y, v2Raster.y);
        float xmax = max3(v0Raster.x, v1Raster.x, v2Raster.x);
        float ymax = max3(v0Raster.y, v1Raster.y, v2Raster.y);
        if (xmin > imageWidth - 1 || xmax < 0 || ymin > imageHeight - 1 || ymax < 0)
            continue;

        int x0 = std::max(0, (int)std::floor(xmin));
        int x1 = std::min((int)imageWidth - 1, (int)std::floor(xmax));
        int y0 = std::max(0, (int)std::floor(ymin));
        int y1 = std::min((int)imageHeight - 1, (int)std::floor(ymax));

        float area = edgeFunction(v0Raster, v1Raster, v2Raster);
        if (area == 0.0f)
            continue;

        // per-pixel loop
        for (int y = y0; y <= y1; ++y)
        {
            for (int x = x0; x <= x1; ++x)
            {
                Vec3f pSample((float)x + 0.5f, (float)y + 0.5f, 0.0f);
                float w0 = edgeFunction(v1Raster, v2Raster, pSample);
                float w1 = edgeFunction(v2Raster, v0Raster, pSample);
                float w2 = edgeFunction(v0Raster, v1Raster, pSample);
                if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                {
                    w0 /= area;
                    w1 /= area;
                    w2 /= area;
                    // 透视修正 oneOverZ
                    float oneOverZ = v0Raster.z * w0 + v1Raster.z * w1 + v2Raster.z * w2;
                    float z = 1.0f / oneOverZ;
                    if (z < depthBuffer[y * imageWidth + x])
                    {
                        depthBuffer[y * imageWidth + x] = z;

                        // 透视修正法线插值： interpN = (n0_p*w0 + n1_p*w1 + n2_p*w2) * z
                        Vec3f interpN = (n0_p * w0) + (n1_p * w1) + (n2_p * w2);
                        interpN *= z;
                        // 归一化法线并防止零向量
                        float len = interpN.length();
                        if (len > 1e-6f)
                            interpN *= (1.0f / len);
                        else
                            interpN = Vec3f(0, 0, 1);

                        // Reconstruct fragment position in camera space (perspective-correct)
                        Vec3f fragPosCam = ((p0_p * w0) + (p1_p * w1) + (p2_p * w2)) * z;

                        // per-pixel lighting using point light (ambient + attenuated diffuse + specular)
                        Vec3f ambient = Ka * Ia;

                        // vector from fragment to light (camera space)
                        Vec3f Ld = pointLightCam - fragPosCam;
                        float dist = Ld.length();
                        if (dist > 1e-6f)
                            Ld *= (1.0f / dist);
                        else
                            Ld = Vec3f(0, 0, 1);

                        // attenuation
                        float attenuation = 1.0f / (att_c + att_l * dist + att_q * dist * dist);

                        // diffuse
                        float ndotl = interpN.dotProduct(Ld);
                        if (ndotl < 0)
                            ndotl = 0;
                        Vec3f diffuse = Il * Kd * ndotl * attenuation;

                        // specular (Blinn-Phong)
                        Vec3f Vcam = (Vec3f{0, 0, 0} - fragPosCam);
                        float vlen = Vcam.length();
                        if (vlen > 1e-6f)
                            Vcam *= (1.0f / vlen);
                        else
                            Vcam = Vec3f(0, 0, 1);
                        Vec3f H = Ld + Vcam;
                        H.normalize();
                        float ndoth = interpN.dotProduct(H);
                        if (ndoth < 0)
                            ndoth = 0;
                        Vec3f specular = Il * Ks * powf(ndoth, shininess) * attenuation;

                        Vec3f finalC = ambient + diffuse + specular;

                        // clamp and pack
                        auto clamp01 = [](float v)
                        { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); };
                        uint8_t rr = (uint8_t)(clamp01(finalC.x) * 255.0f);
                        uint8_t gg = (uint8_t)(clamp01(finalC.y) * 255.0f);
                        uint8_t bb = (uint8_t)(clamp01(finalC.z) * 255.0f);
                        frameBuffer[y * imageWidth + x] = RGB(rr, gg, bb);
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
