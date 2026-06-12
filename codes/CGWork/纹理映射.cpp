#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "geometry.h"
#include "SDL.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <cmath>
#include <map>
#include <filesystem>

// --- 新增: 纹理结构体 ---
struct Texture
{
    std::vector<Vec3f> pixels; // 存储像素数据，每个像素是 Vec3f (R, G, B)
    int width = 0;
    int height = 0;
};

// --- 新增: 材质结构体 ---
// 简化版，只包含漫反射贴图路径
struct Material
{
    std::string map_Kd_path; // 漫反射纹理贴图路径
    Texture texture;         // 实际加载的纹理数据
    bool has_texture = false;
};

struct Face
{
    std::vector<int> vertexIndices;
    std::vector<int> texcoordIndices;
    std::vector<int> normalIndices;
    // 新增: 记录面使用的材质索引 (或名称)
    std::string material_name;
};

struct Mesh
{
    std::vector<Vec3f> vertices;
    std::vector<Vec2f> texcoords;
    std::vector<Vec3f> normals;
    std::vector<Face> faces;
    // 新增: 存储材质信息
    std::map<std::string, Material> materials;
};

static bool fileExists(const std::string &p)
{
    try
    {
        return std::filesystem::exists(std::filesystem::u8path(p));
    }
    catch (...)
    {
        return false;
    }
}

static std::string joinPath(const std::string &a, const std::string &b)
{
    try
    {
        std::filesystem::path pa = std::filesystem::u8path(a);
        std::filesystem::path pb = std::filesystem::u8path(b);
        std::filesystem::path pr = pa / pb;
        return pr.u8string();
    }
    catch (...)
    {
        // fallback simple join
        if (a.empty())
            return b;
        char sep = (a.find('/') != std::string::npos) ? '/' : '\\';
        std::string s = a;
        if (s.back() != '/' && s.back() != '\\')
            s.push_back(sep);
        s += b;
        return s;
    }
}

static bool isAbsolutePath(const std::string &p)
{
    if (p.empty())
        return false;
#ifdef _WIN32
    // Windows: drive letter or UNC
    if (p.size() > 1 && p[1] == ':')
        return true;
    if (p.size() > 1 && p[0] == '\\' && p[1] == '\\')
        return true;
#else
    if (p.front() == '/')
        return true;
#endif
    return false;
}

// Try resolving texture path with several fallbacks.
// base_dir: directory where OBJ/MTL located (may be empty)
static std::string resolveTexturePath(const std::string &base_dir, const std::string &filename)
{
    if (filename.empty())
        return std::string();

    // if absolute, return if exists
    if (isAbsolutePath(filename))
    {
        if (fileExists(filename))
            return filename;
    }

    std::vector<std::string> candidates;

    // 1. If filename contains directory separators, try as relative to base_dir
    if (!base_dir.empty())
        candidates.push_back(joinPath(base_dir, filename));

    // 2. current working directory
    try
    {
        candidates.push_back((std::filesystem::current_path() / std::filesystem::u8path(filename)).u8string());
    }
    catch (...)
    {
        candidates.push_back(filename);
    }

    // 3. executable directory (SDL_GetBasePath)
#ifdef SDL_MAJOR_VERSION
    char *base = SDL_GetBasePath();
    if (base)
    {
        std::string exeDir(base);
        SDL_free(base);
        candidates.push_back(joinPath(exeDir, filename));
    }
#endif

    // 4. try filename as-is
    candidates.push_back(filename);

    // Check candidates and return first existing
    for (auto &c : candidates)
    {
        // normalize small things
        std::string tryPath = c;
        if (tryPath.size() >= 2 && tryPath[0] == '"' && tryPath.back() == '"')
        {
            tryPath = tryPath.substr(1, tryPath.size() - 2);
        }
        // trim whitespace
        auto l = tryPath.find_first_not_of(" \t\r\n");
        auto r = tryPath.find_last_not_of(" \t\r\n");
        if (l == std::string::npos)
            continue;
        tryPath = tryPath.substr(l, r - l + 1);

        std::cerr << "[texture] trying: " << tryPath << std::endl;
        if (fileExists(tryPath))
        {
            std::cerr << "[texture] found: " << tryPath << std::endl;
            return tryPath;
        }
    }

    // not found, return original (caller will try to load and fail)
    std::cerr << "[texture] not found, will attempt original name: " << filename << std::endl;
    return filename;
}

bool loadTextureFromFile(const std::string &filepath, Texture &texture)
{
    std::cout << "Attempting to load texture from: " << filepath << "..." << std::endl;

    int width = 0, height = 0, channels = 0;

    // try float loader first
    float *dataf = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
    if (dataf)
    {
        if (channels < 3)
            std::cerr << "[texture] warning: channels = " << channels << std::endl;
        texture.width = width;
        texture.height = height;
        texture.pixels.clear();
        texture.pixels.reserve((size_t)width * height);
        size_t npix = (size_t)width * height;
        for (size_t i = 0; i < npix; ++i)
        {
            size_t idx = i * channels;
            float r = dataf[idx];
            float g = (channels > 1) ? dataf[idx + 1] : r;
            float b = (channels > 2) ? dataf[idx + 2] : r;
            texture.pixels.emplace_back(r, g, b);
        }
        stbi_image_free(dataf);
        std::cout << "Texture loaded (float) W:" << width << " H:" << height << " C:" << channels << std::endl;
        return true;
    }

    // fallback to 8-bit loader then convert to float
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        std::cerr << "错误: 无法加载图片: " << filepath << "。原因: " << stbi_failure_reason() << std::endl;
        if (data)
            stbi_image_free(data);
        return false;
    }

    texture.width = width;
    texture.height = height;
    texture.pixels.clear();
    texture.pixels.reserve((size_t)width * height);
    size_t npix = (size_t)width * height;
    for (size_t i = 0; i < npix; ++i)
    {
        size_t idx = i * channels;
        float r = data[idx] / 255.0f;
        float g = (channels > 1) ? data[idx + 1] / 255.0f : r;
        float b = (channels > 2) ? data[idx + 2] / 255.0f : r;
        texture.pixels.emplace_back(r, g, b);
    }
    stbi_image_free(data);
    std::cout << "Texture loaded (8-bit) W:" << width << " H:" << height << " C:" << channels << std::endl;
    return true;
}

// -------------------------------------------------------------------------------------
// 新增: MTL 文件解析函数
// -------------------------------------------------------------------------------------
bool loadMTL(const std::string &mtl_filepath, std::map<std::string, Material> &materials, const std::string &base_dir)
{
    std::ifstream file(mtl_filepath);
    if (!file.is_open())
    {
        std::cerr << "无法打开 MTL 文件: " << mtl_filepath << std::endl;
        return false;
    }

    std::string line;
    std::string current_mtl_name;

    // base_dir may be provided by caller (directory of OBJ)
    std::string mtl_dir;
    size_t last_slash = mtl_filepath.find_last_of("/\\");
    if (last_slash == std::string::npos)
        mtl_dir = base_dir;
    else
        mtl_dir = mtl_filepath.substr(0, last_slash);

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "newmtl")
        {
            iss >> current_mtl_name;
            materials[current_mtl_name] = Material();
        }
        else if (type == "map_Kd" && !current_mtl_name.empty())
        {
            std::string texture_filename;
            iss >> texture_filename;
            // remove quotes if present
            if (!texture_filename.empty() && texture_filename.front() == '"' && texture_filename.back() == '"')
            {
                texture_filename = texture_filename.substr(1, texture_filename.size() - 2);
            }

            // Resolve with several fallbacks: MTL dir, OBJ base_dir, CWD, exe dir, original
            std::string resolved = resolveTexturePath(mtl_dir.empty() ? base_dir : mtl_dir, texture_filename);
            Material &current_mtl = materials[current_mtl_name];
            current_mtl.map_Kd_path = resolved;

            if (fileExists(resolved))
            {
                if (loadTextureFromFile(resolved, current_mtl.texture))
                {
                    current_mtl.has_texture = true;
                    std::cerr << "[mtl] loaded texture for " << current_mtl_name << " : " << resolved << std::endl;
                }
                else
                {
                    std::cerr << "[mtl] failed to load texture at " << resolved << std::endl;
                }
            }
            else
            {
                std::cerr << "[mtl] texture file not found: " << resolved << std::endl;
            }
        }
    }
    return true;
}

// -------------------------------------------------------------------------------------
// 修改后的 loadOBJ 函数
// -------------------------------------------------------------------------------------
bool loadOBJ(const std::string &filename, Mesh &mesh)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::string current_mtl_name = ""; // 记录当前使用的材质名称

    // 提取 OBJ 文件的基础路径，用于加载 MTL 和纹理
    size_t last_slash = filename.find_last_of("/\\");
    std::string base_dir = (last_slash == std::string::npos) ? "." : filename.substr(0, last_slash);

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
        else if (type == "mtllib")
        {
            // 新增: 解析材质库文件
            std::string mtl_filename;
            iss >> mtl_filename;
            std::string mtl_full_path = base_dir + "/" + mtl_filename;
            loadMTL(mtl_full_path, mesh.materials, base_dir);
        }
        else if (type == "usemtl")
        {
            // 新增: 设置当前使用的材质
            iss >> current_mtl_name;
        }
        else if (type == "f")
        {
            Face face;
            face.material_name = current_mtl_name; // 将当前材质名赋予面

            std::string vertexData;
            while (iss >> vertexData)
            {
                // ... (OBJ 索引解析逻辑保持不变)
                int vi = -1, ti = -1, ni = -1;
                size_t pos1 = vertexData.find('/');
                size_t pos2 = vertexData.find('/', pos1 == std::string::npos ? pos1 : pos1 + 1);

                if (pos1 == std::string::npos)
                {
                    vi = std::stoi(vertexData);
                }
                else if (pos2 == std::string::npos)
                {
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ti = std::stoi(vertexData.substr(pos1 + 1));
                }
                else if (pos2 == pos1 + 1)
                {
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ni = std::stoi(vertexData.substr(pos2 + 1));
                }
                else
                {
                    vi = std::stoi(vertexData.substr(0, pos1));
                    ti = std::stoi(vertexData.substr(pos1 + 1, pos2 - pos1 - 1));
                    ni = std::stoi(vertexData.substr(pos2 + 1));
                }
                if (vi != 0)
                    vi -= 1;
                if (ti != 0)
                    ti -= 1;
                if (ni != 0)
                    ni -= 1;
                face.vertexIndices.push_back(vi);
                face.texcoordIndices.push_back(ti);
                face.normalIndices.push_back(ni);
            }
            mesh.faces.push_back(face);
        }
    }

    file.close();
    return true;
}
//

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
    // X (Right) 轴
    0.707107, -0.408248, 0.577350, 0,
    // Y (Up) 轴
    0.000000, 0.816497, 0.577350, 0,
    // Z (Look) 轴
    -0.707107, -0.408248, 0.577350, 0,
    // 平移项 (-e.x, -e.y, -e.z)
    0.000000, 0.000000, -5.196152, 1};

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

    // 加载obj模型
    if (loadOBJ("Crate1.obj", mesh))
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
    // 定义光源
    Vec3f Ia{0.1, 0.1, 0.1}; // 环境光
    Vec3f Ii{0.5, 0.5, 0.5}; // 平行光
    Vec3f L{1, 1, 1};        // 平行光的方向
    L.normalize();

    // 定义材质
    Vec3f Ka{1, 1, 1};          // 环境反射系数
    Vec3f Kd{0.8, 0.002, 0.01}; // 漫反射系数
    Vec3f Ks{0.5, 0.5, 0.5};    // 镜面反射系数
    float nb = 100;             // 高光系数

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
    // 1. 遍历所有三角形
    int ntris = mesh.faces.size();
    for (uint32_t i = 0; i < ntris; ++i)
    {
        // 检查面的索引是否有效
        if (mesh.faces[i].vertexIndices.size() < 3)
            continue;

        // 安全获取顶点
        int vIdx0 = mesh.faces[i].vertexIndices[0];
        int vIdx1 = mesh.faces[i].vertexIndices[1];
        int vIdx2 = mesh.faces[i].vertexIndices[2];

        if (vIdx0 < 0 || vIdx0 >= (int)mesh.vertices.size() ||
            vIdx1 < 0 || vIdx1 >= (int)mesh.vertices.size() ||
            vIdx2 < 0 || vIdx2 >= (int)mesh.vertices.size())
        {
            continue;
        }

        const Vec3f &v0 = mesh.vertices[vIdx0];
        const Vec3f &v1 = mesh.vertices[vIdx1];
        const Vec3f &v2 = mesh.vertices[vIdx2];

        // 安全获取纹理坐标
        Vec2f uv0(0, 0), uv1(0, 0), uv2(0, 0);
        if (mesh.faces[i].texcoordIndices.size() >= 3)
        {
            int tIdx0 = mesh.faces[i].texcoordIndices[0];
            int tIdx1 = mesh.faces[i].texcoordIndices[1];
            int tIdx2 = mesh.faces[i].texcoordIndices[2];

            if (tIdx0 >= 0 && tIdx0 < (int)mesh.texcoords.size())
                uv0 = mesh.texcoords[tIdx0];
            if (tIdx1 >= 0 && tIdx1 < (int)mesh.texcoords.size())
                uv1 = mesh.texcoords[tIdx1];
            if (tIdx2 >= 0 && tIdx2 < (int)mesh.texcoords.size())
                uv2 = mesh.texcoords[tIdx2];
        }
        // [comment]
        // Convert the vertices of the triangle to raster space
        // [/comment]
        Vec3f v0Raster, v1Raster, v2Raster;
        convertToRaster(v0, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v0Raster);
        convertToRaster(v1, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v1Raster);
        convertToRaster(v2, worldToCamera, l, r, t, b, nearClippingPLane, imageWidth, imageHeight, v2Raster);

        // 计算三角形法向
        Vec3f v0Cam, v1Cam, v2Cam;
        worldToCamera.multVecMatrix(v0, v0Cam);
        worldToCamera.multVecMatrix(v1, v1Cam);
        worldToCamera.multVecMatrix(v2, v2Cam);
        Vec3f n = (v1Cam - v0Cam).crossProduct(v2Cam - v0Cam);
        n.normalize();

        // 背面剔除
        Vec3f V = (v0Cam + v1Cam + v2Cam) * (-1.f / 3.f); // 视线方向
        V.normalize();
        if (V.dotProduct(n) <= 0)
            continue;

        // Flat shading实现
        // 1. 环境反射：Ka * Ia
        Vec3f ca = Ka * Ia;
        // 2. 漫反射：Ii * Kd * (n * l)
        float ndotL = n.dotProduct(L);
        ndotL = ndotL > 0 ? ndotL : 0; // 光线方向与法向夹角大于90°时，无法到达该面
        Vec3f cd = Ii * Kd * ndotL;
        // 3. 镜面反射：Ii * Ks * (n * h)^nb
        Vec3f h = L + V; // 半程向量
        h.normalize();
        float ndoth = n.dotProduct(h);
        ndoth = ndoth > 0 ? ndoth : 0;
        Vec3f cs = Ii * Ks * powf(ndoth, nb);
        Vec3f color = ca + cd + cs; // 三种反射光叠加得到最终颜色

        // [comment]
        // Precompute reciprocal of vertex z-coordinate
        // [/comment]
        v0Raster.z = 1 / v0Raster.z,
        v1Raster.z = 1 / v1Raster.z,
        v2Raster.z = 1 / v2Raster.z;
        // 3. 获取三角形的三个法向
        Vec3f n0(0, 0, 1), n1(0, 0, 1), n2(0, 0, 1);
        if (mesh.faces[i].normalIndices.size() >= 3)
        {
            int nIdx0 = mesh.faces[i].normalIndices[0];
            int nIdx1 = mesh.faces[i].normalIndices[1];
            int nIdx2 = mesh.faces[i].normalIndices[2];

            if (nIdx0 >= 0 && nIdx0 < (int)mesh.normals.size())
                n0 = mesh.normals[nIdx0];
            if (nIdx1 >= 0 && nIdx1 < (int)mesh.normals.size())
                n1 = mesh.normals[nIdx1];
            if (nIdx2 >= 0 && nIdx2 < (int)mesh.normals.size())
                n2 = mesh.normals[nIdx2];
        }
        // 4. 乘以1/z进行透视校正
        n0 *= v0Raster.z, n1 *= v1Raster.z, n2 *= v2Raster.z;

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

        // [comment]
        // Inner loop
        // [/comment]
        for (uint32_t y = y0; y <= y1; ++y)
        {
            for (uint32_t x = x0; x <= x1; ++x)
            {
                Vec3f pixelSample(x + 0.5, y + 0.5, 0);
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
                    Vec2f uv = uv0 * w0 + uv1 * w1 + uv2 * w2;
                    Vec3f texColor = Vec3f(1, 1, 1); // 默认颜色
                    if (mesh.materials.find(mesh.faces[i].material_name) != mesh.materials.end() &&
                        mesh.materials[mesh.faces[i].material_name].has_texture)
                    {

                        const Texture &texture = mesh.materials[mesh.faces[i].material_name].texture;

                        // 计算纹理坐标（注意UV坐标系转换）
                        float u = fmod(uv.x, 1.0f);
                        float v = fmod(uv.y, 1.0f);
                        if (u < 0)
                            u += 1.0f;
                        if (v < 0)
                            v += 1.0f;

                        // 转换为像素坐标
                        int tx = (int)(u * (texture.width - 1));
                        int ty = (int)((1.0f - v) * (texture.height - 1)); // 注意Y轴翻转

                        // 边界检查
                        tx = std::max(0, std::min(tx, texture.width - 1));
                        ty = std::max(0, std::min(ty, texture.height - 1));

                        int index = ty * texture.width + tx;
                        if (index >= 0 && index < (int)texture.pixels.size())
                        {
                            texColor = texture.pixels[index];
                        }
                    }

                    if (z < depthBuffer[y * imageWidth + x])
                    {
                        depthBuffer[y * imageWidth + x] = z;
                        uint32_t c = RGB(texColor.x * 255, texColor.y * 255, texColor.z * 255);
                        frameBuffer[y * imageWidth + x] = c;
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
