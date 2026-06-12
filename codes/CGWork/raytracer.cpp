#define SDL_MAIN_HANDLED
// ========== 纹理相关代码 ==========
#define STB_IMAGE_IMPLEMENTATION
extern "C" {
#include "stb_image.h"
}

#if defined __linux__ || defined __APPLE__
#else
#endif

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <limits>
#include <SDL2/SDL.h>

const float kInfinity = std::numeric_limits<float>::max();
const float kEpsilon = 1e-8;

class Vec3f
{
public:
    float x, y, z;
    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(float xx) : x(xx), y(xx), z(xx) {}
    Vec3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vec3f operator*(const float &f) const { return Vec3f(x * f, y * f, z * f); }
    Vec3f operator/(const float &f) const { return Vec3f(x / f, y / f, z / f); }
    Vec3f operator+(const Vec3f &v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }
    Vec3f operator-(const Vec3f &v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }
    Vec3f operator*(const Vec3f &v) const { return Vec3f(x * v.x, y * v.y, z * v.z); }
    Vec3f operator-() const { return Vec3f(-x, -y, -z); }
    float dot(const Vec3f &v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3f cross(const Vec3f &v) const { return Vec3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    Vec3f &normalize()
    {
        float nor2 = x * x + y * y + z * z;
        if (nor2 > 0)
        {
            float invNor = 1 / sqrtf(nor2);
            x *= invNor;
            y *= invNor;
            z *= invNor;
        }
        return *this;
    }
    float length2() const { return x * x + y * y + z * z; }
    float length() const { return sqrtf(length2()); }
};

struct Texture
{
    std::vector<Vec3f> pixels; // 存储像素数据，每个像素是 Vec3f (R, G, B)
    int width = 0;
    int height = 0;
    
    // 采样纹理
    Vec3f sample(float u, float v) const {
        if (pixels.empty()) return Vec3f(1, 1, 1);
        
        // 确保 UV 在 [0,1] 范围内
        u = fmod(u, 1.0f);
        v = fmod(v, 1.0f);
        if (u < 0) u += 1.0f;
        if (v < 0) v += 1.0f;
        
        // 转换为像素坐标 (注意Y轴翻转)
        int tx = (int)(u * (width - 1));
        int ty = (int)((1.0f - v) * (height - 1));
        
        // 边界检查
        tx = std::max(0, std::min(tx, width - 1));
        ty = std::max(0, std::min(ty, height - 1));
        
        int index = ty * width + tx;
        if (index >= 0 && index < (int)pixels.size()) {
            return pixels[index];
        }
        return Vec3f(1, 0, 1); // 品红色表示错误
    }
};

// 加载纹理
bool loadTextureFromFile(const std::string &filepath, Texture &texture) {
    std::cout << "Loading texture: " << filepath << "..." << std::endl;
    
    int width = 0, height = 0, channels = 0;
    
    // 尝试浮点加载器
    float *dataf = stbi_loadf(filepath.c_str(), &width, &height, &channels, 0);
    if (dataf) {
        if (channels < 3)
            std::cerr << "[texture] warning: channels = " << channels << std::endl;
        texture.width = width;
        texture.height = height;
        texture.pixels.clear();
        texture.pixels.reserve((size_t)width * height);
        size_t npix = (size_t)width * height;
        for (size_t i = 0; i < npix; ++i) {
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
    
    // 回退到8位加载器
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Error: Cannot load image: " << filepath << ". Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    texture.width = width;
    texture.height = height;
    texture.pixels.clear();
    texture.pixels.reserve((size_t)width * height);
    size_t npix = (size_t)width * height;
    for (size_t i = 0; i < npix; ++i) {
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

// --- Triangle structure ---
// --- 扩展 Triangle 结构 ---
struct Triangle
{
    Vec3f v0, v1, v2;           // 顶点
    float uv0[2], uv1[2], uv2[2]; // 纹理坐标 (u,v)
    Vec3f normal;               // 面法线
    Vec3f emission;             // 自发光
    Vec3f surfaceColor;         // 基础颜色
    float reflectivity;         // 反射率
    const Texture* texture;     // 纹理指针
    bool hasTexture;            // 是否有纹理

    // 构造函数（不带纹理）
    Triangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
             const Vec3f &e = Vec3f(0), float refl = 0.0f, 
             const Vec3f &sc = Vec3f(1))
        : v0(a), v1(b), v2(c), 
          emission(e), surfaceColor(sc), reflectivity(refl),
          texture(nullptr), hasTexture(false)
    {
        // 默认纹理坐标
        uv0[0] = 0; uv0[1] = 0;
        uv1[0] = 1; uv1[1] = 0;
        uv2[0] = 0; uv2[1] = 1;
        
        computeNormal();
    }

    // 构造函数（带纹理）
    Triangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
             const Vec3f &e, float refl, 
             const Vec3f &sc, const Texture* tex,
             float u0 = 0, float v0 = 0, 
             float u1 = 1, float v1 = 0, 
             float u2 = 0, float v2 = 1)
        : v0(a), v1(b), v2(c), 
          emission(e), surfaceColor(sc), reflectivity(refl),
          texture(tex), hasTexture(tex != nullptr)
    {
        uv0[0] = u0; uv0[1] = v0;
        uv1[0] = u1; uv1[1] = v1;
        uv2[0] = u2; uv2[1] = v2;
        
        computeNormal();
    }

private:
    void computeNormal() {
        Vec3f edge1 = v1 - v0;
        Vec3f edge2 = v2 - v0;
        normal = edge1.cross(edge2);
        normal.normalize();
    }

public:
    // 根据重心坐标获取颜色
    Vec3f getColorAt(float u, float v) const {
        if (hasTexture && texture) {
            // 根据重心坐标插值纹理坐标
            float w = 1.0f - u - v;
            float texU = uv0[0] * w + uv1[0] * u + uv2[0] * v;
            float texV = uv0[1] * w + uv1[1] * u + uv2[1] * v;
        
            // 直接返回纹理颜色，不要乘以surfaceColor
            return texture->sample(texU, texV);
        }   
        return surfaceColor;
    }

    // Moller-Trumbore 射线三角形相交测试（保持不变）
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t, float &u, float &v) const {
        Vec3f edge1 = v1 - v0;
        Vec3f edge2 = v2 - v0;
        Vec3f h = raydir.cross(edge2);
        float a = edge1.dot(h);

        if (fabs(a) < kEpsilon)
            return false;

        float f = 1.0f / a;
        Vec3f s = rayorig - v0;
        u = f * s.dot(h);

        if (u < 0.0f || u > 1.0f)
            return false;

        Vec3f q = s.cross(edge1);
        v = f * raydir.dot(q);

        if (v < 0.0f || u + v > 1.0f)
            return false;

        t = f * edge2.dot(q);
        return t > kEpsilon;
    }

    Vec3f getNormalAt(float u, float v) const {
        return normal;
    }
};

// --- Sphere (existing) ---
class Sphere
{
public:
    Vec3f center;
    float radius;
    Vec3f surfaceColor, emissionColor;
    float reflectivity;
    const Texture* texture; // 添加纹理指针
    bool hasTexture;
    
    Sphere(const Vec3f &c, float r, const Vec3f &sc, 
           const Vec3f &ec = Vec3f(0), float refl = 0.0f,
           const Texture* tex = nullptr)
        : center(c), radius(r), surfaceColor(sc), 
          emissionColor(ec), reflectivity(refl),
          texture(tex), hasTexture(tex != nullptr) {}
    
    // 获取球体上某点的纹理颜色
    Vec3f getColorAt(const Vec3f &point) const {
        if (!hasTexture) return surfaceColor;
        
        // 计算球坐标 (UV映射)
        Vec3f dir = (point - center).normalize();
        float u = 0.5f + atan2(dir.z, dir.x) / (2.0f * M_PI);
        float v = 0.5f - asin(dir.y) / M_PI;
        
        return texture->sample(u, v) * surfaceColor;
    }
    
    // 原有的 intersect 方法保持不变
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const {
        Vec3f l = center - rayorig;
        float tca = l.dot(raydir);
        if (tca < 0) return false;
        float d2 = l.dot(l) - tca * tca;
        float radius2 = radius * radius;
        if (d2 > radius2) return false;
        float thc = sqrtf(radius2 - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true;
    }
};

Vec3f reflect(const Vec3f &raydir, const Vec3f &normal)
{
    return raydir - normal * 2.0f * raydir.dot(normal);
}

// --- Tone mapping function ---
// Apply Reinhard tone mapping and gamma correction to HDR color
Vec3f toneMap(const Vec3f &hdrColor, float exposure = 1.0f, float gamma = 2.2f)
{
    // Apply exposure before tone mapping
    Vec3f color = hdrColor * exposure;
    
    // Reinhard tone mapping: color / (color + 1) - component-wise
    Vec3f mappedColor(
        color.x / (color.x + 1.0f),
        color.y / (color.y + 1.0f),
        color.z / (color.z + 1.0f)
    );
    
    // Gamma correction
    mappedColor.x = powf(std::max(0.0f, mappedColor.x), 1.0f / gamma);
    mappedColor.y = powf(std::max(0.0f, mappedColor.y), 1.0f / gamma);
    mappedColor.z = powf(std::max(0.0f, mappedColor.z), 1.0f / gamma);
    
    // Final clamp
    mappedColor.x = std::min(1.0f, std::max(0.0f, mappedColor.x));
    mappedColor.y = std::min(1.0f, std::max(0.0f, mappedColor.y));
    mappedColor.z = std::min(1.0f, std::max(0.0f, mappedColor.z));
    
    return mappedColor;
}

// --- Main ray tracing function ---
Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir,
            const std::vector<Sphere> &spheres,
            const std::vector<Triangle> &triangles,
            int depth)
{
    float tnear = kInfinity;
    int hitType = 0; // 0: none, 1: sphere, 2: triangle
    int sphereIdx = -1, triangleIdx = -1;
    float u_hit = 0, v_hit = 0; // 添加命中点的重心坐标

    // Check sphere intersections
    for (unsigned i = 0; i < spheres.size(); ++i)
    {
        float t0, t1;
        if (spheres[i].intersect(rayorig, raydir, t0, t1))
        {
            if (t0 < tnear)
            {
                tnear = t0;
                hitType = 1;
                sphereIdx = i;
            }
        }
    }

    // Check triangle intersections
    for (unsigned i = 0; i < triangles.size(); ++i)
    {
        float t, u, v;
        if (triangles[i].intersect(rayorig, raydir, t, u, v))
        {
            if (t < tnear)
            {
                tnear = t;
                hitType = 2;
                triangleIdx = i;
                u_hit = u;
                v_hit = v;
            }
        }
    }

    // No hit -> return background
    if (hitType == 0)
    {
        return Vec3f(0.1f); // background color (dark)
    }

    // Compute hit point and normal
    Vec3f phit = rayorig + raydir * tnear;
    Vec3f nhit;
    Vec3f surfaceColor, emissionColor;
    float reflectivity = 0.0f;

    if (hitType == 1) {
        // 球体命中 - 使用纹理颜色
        const Sphere &sphere = spheres[sphereIdx];
        nhit = (phit - sphere.center).normalize();
        surfaceColor = sphere.hasTexture ? sphere.getColorAt(phit) : sphere.surfaceColor;
        emissionColor = sphere.emissionColor;
        reflectivity = sphere.reflectivity;
    } else {
        // 三角形命中 - 使用纹理颜色
        const Triangle &triangle = triangles[triangleIdx];
        nhit = triangle.getNormalAt(u_hit, v_hit);
        surfaceColor = triangle.getColorAt(u_hit, v_hit); // 使用纹理插值
        emissionColor = triangle.emission;
        reflectivity = triangle.reflectivity;
    }

    // Flip normal if ray is inside (backfacing)
    if (raydir.dot(nhit) > 0)
        nhit = nhit * -1.0f;

    // Emission (if light source)
    Vec3f color = emissionColor;
    // Low-level ambient term to avoid fully black unlit areas
    Vec3f ambient(0.2f, 0.2f, 0.2f); // ambient level (increased for better visibility)
    color = color + surfaceColor * ambient;

    // Direct lighting from emissive objects
    float bias = 1e-3f;
    for (unsigned i = 0; i < spheres.size(); ++i)
    {
        if (spheres[i].emissionColor.length2() > kEpsilon)
        {
            Vec3f lightDir = (spheres[i].center - phit);
            float lightDist = lightDir.length();
            if (lightDist < 1e-4f)
                continue;
            lightDir = lightDir / lightDist;

            // Shadow ray test
            bool inShadow = false;
            for (unsigned j = 0; j < spheres.size(); ++j)
            {
                if (i != j)
                {
                    float t0, t1;
                    if (spheres[j].intersect(phit + nhit * bias, lightDir, t0, t1))
                    {
                        if (t0 > 0 && t0 < lightDist)
                        {
                            inShadow = true;
                            break;
                        }
                    }
                }
            }
            for (unsigned j = 0; j < triangles.size(); ++j)
            {
                float t, u, v;
                if (triangles[j].intersect(phit + nhit * bias, lightDir, t, u, v))
                {
                    if (t > 0 && t < lightDist)
                    {
                        inShadow = true;
                        break;
                    }
                }
            }

            if (!inShadow)
            {
                float attenuation = 20.0f / (lightDist * lightDist + 1.0f);
                Vec3f lightContrib = surfaceColor * spheres[i].emissionColor * attenuation *
                                     std::max(0.0f, nhit.dot(lightDir));
                color = color + lightContrib;
            }
        }
    }

    // Triangle emissive sources (approximate each emissive triangle as a point light at its centroid)
    for (unsigned i = 0; i < triangles.size(); ++i)
    {
        if (triangles[i].emission.length2() > kEpsilon)
        {
            Vec3f triCenter = (triangles[i].v0 + triangles[i].v1 + triangles[i].v2) / 3.0f;
            Vec3f lightDir = (triCenter - phit);
            float lightDist = lightDir.length();
            if (lightDist < 1e-4f)
                continue;
            lightDir = lightDir / lightDist;

            bool inShadow = false;
            // Shadow test against spheres
            for (unsigned j = 0; j < spheres.size(); ++j)
            {
                float t0, t1;
                if (spheres[j].intersect(phit + nhit * bias, lightDir, t0, t1))
                {
                    if (t0 > 0 && t0 < lightDist)
                    {
                        inShadow = true;
                        break;
                    }
                }
            }
            if (!inShadow)
            {
                // Shadow test against triangles
                for (unsigned j = 0; j < triangles.size(); ++j)
                {
                    if (j == i) // avoid self-intersection with the emitting triangle
                        continue;
                    float t, u, v;
                    if (triangles[j].intersect(phit + nhit * bias, lightDir, t, u, v))
                    {
                        if (t > 0 && t < lightDist)
                        {
                            inShadow = true;
                            break;
                        }
                    }
                }
            }

            if (!inShadow)
            {
                float attenuation = 25.0f / (lightDist * lightDist + 1.0f);
                Vec3f lightContrib = surfaceColor * triangles[i].emission * attenuation *
                                     std::max(0.0f, nhit.dot(lightDir));
                color = color + lightContrib;
            }
        }
    }

    // Reflection based on material reflectivity (mirror-like surfaces only)
    reflectivity = std::max(0.0f, std::min(1.0f, reflectivity));
    if (reflectivity > 0.0f && depth < 4)
    {
        Vec3f reflectDir = reflect(raydir, nhit);
        Vec3f reflectColor = trace(phit + nhit * bias, reflectDir, spheres, triangles, depth + 1);
        // simple energy balance: mix diffuse/emission with reflection
        color = color * (1.0f - reflectivity) + reflectColor * reflectivity;
    }

    // Simple indirect illumination (one bounce diffuse reflection for color bleeding)
    // Use cosine-weighted hemisphere sampling for more realistic diffuse bounce
    if (depth < 1) // Only do one indirect bounce to avoid too much computation
    {
        Vec3f indirectColor(0.0f);
        int numSamples = 8; // increase samples for better color bleeding

        // Create tangent basis
        Vec3f tangent = (fabs(nhit.x) > 0.9f) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
        Vec3f bitangent = nhit.cross(tangent);
        if (bitangent.length2() < 0.01f)
        {
            tangent = Vec3f(0, 0, 1);
            bitangent = nhit.cross(tangent);
        }
        bitangent.normalize();
        tangent = bitangent.cross(nhit);
        tangent.normalize();

        auto randf = []() { return (float)rand() / (float)RAND_MAX; };

        auto cosineSampleHemisphere = [&](float r1, float r2) {
            float phi = 2.0f * M_PI * r1;
            float cosTheta = sqrtf(1.0f - r2);
            float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
            float x = cosf(phi) * sinTheta;
            float y = sinf(phi) * sinTheta;
            float z = cosTheta;
            return Vec3f(x, y, z);
        };

        for (int s = 0; s < numSamples; ++s)
        {
            float r1 = randf();
            float r2 = randf();
            Vec3f sampleLocal = cosineSampleHemisphere(r1, r2);
            // Transform from local to world space
            Vec3f sampleDir = tangent * sampleLocal.x + bitangent * sampleLocal.y + nhit * sampleLocal.z;
            sampleDir.normalize();

            Vec3f sampleColor = trace(phit + nhit * bias, sampleDir, spheres, triangles, depth + 1);
            indirectColor = indirectColor + sampleColor;
        }
        indirectColor = indirectColor / (float)numSamples;

        // Add indirect contribution with low weight (simulating color bleeding)
        float indirectWeight = 0.12f * (1.0f - reflectivity); // Slightly increased weight
        color = color + surfaceColor * indirectColor * indirectWeight;
    }

    // NaN guard
    if (std::isnan(color.x) || std::isnan(color.y) || std::isnan(color.z))
    {
        color = Vec3f(0.5f);
    }
    
    // Return HDR color (tone mapping will be applied in main rendering loop)
    return color;
}

// --- Main function with scene setup ---
int main()
{
    unsigned width = 640, height = 480;
    std::vector<Vec3f> framebuffer(width * height);

    // 加载纹理
    Texture crateTexture;
    if (!loadTextureFromFile("crate_1.jpg", crateTexture)) {
        // 如果纹理加载失败，使用棋盘格作为后备
        std::cerr << "Texture not found, using checkerboard pattern" << std::endl;
        // 创建一个简单的棋盘格纹理
        crateTexture.width = 64;
        crateTexture.height = 64;
        crateTexture.pixels.resize(64 * 64);
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                bool black = ((x / 8) % 2) ^ ((y / 8) % 2);
                crateTexture.pixels[y * 64 + x] = black ? 
                    Vec3f(0.8f, 0.6f, 0.4f) : // 木箱颜色1
                    Vec3f(0.6f, 0.4f, 0.2f);   // 木箱颜色2
            }
        }
    }

    // 场景对象
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;

    // Cornell Box setup
    // Box dimensions: x in [-5.5, 5.5], y in [-5.5, 5.5], z in [-10, 0]
    // Camera at (0, 0, 5) looking into the box (towards -z)

    // Box corners
    Vec3f c000(-4.5f, -5.5f, -9.0f); // left, bottom, back
    Vec3f c001(-4.5f, -5.5f, 0.0f);   // left, bottom, front
    Vec3f c010(-4.5f, 3.5f, -9.0f);  // left, top, back
    Vec3f c011(-4.5f, 3.5f, 0.0f);    // left, top, front
    Vec3f c100(4.5f, -5.5f, -9.0f);  // right, bottom, back
    Vec3f c101(4.5f, -5.5f, 0.0f);    // right, bottom, front
    Vec3f c110(4.5f, 3.5f, -9.0f);   // right, top, back
    Vec3f c111(4.5f, 3.5f, 0.0f);     // right, top, front

    // Cornell Box colors
    Vec3f redWall(0.5f, 0.15f, 0.15f);   // Left wall (red)
    Vec3f greenWall(0.15f, 0.5f, 0.15f); // Right wall (green)
    Vec3f whiteWall(0.6f, 0.6f, 0.6f); // Back wall, floor, ceiling (white/light gray)

    // Left wall (x = -5.5) - RED
    triangles.push_back(Triangle(c000, c010, c011, Vec3f(0), 0.0f, redWall));
    triangles.push_back(Triangle(c000, c011, c001, Vec3f(0), 0.0f, redWall));

    // Right wall (x = 5.5) - GREEN
    triangles.push_back(Triangle(c100, c101, c111, Vec3f(0), 0.0f, greenWall));
    triangles.push_back(Triangle(c100, c111, c110, Vec3f(0), 0.0f, greenWall));

    // Back wall (z = -10) - WHITE
    triangles.push_back(Triangle(c000, c100, c110, Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(c000, c110, c010, Vec3f(0), 0.0f, whiteWall));

    // Floor (y = -5.5) - WHITE
    triangles.push_back(Triangle(c000, c001, c101, Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(c000, c101, c100, Vec3f(0), 0.0f, whiteWall));

    // Ceiling (y = 3.5) - WHITE (with light source opening)
    // Note: ceiling is at y = 3.5, light opening is around z = -5 to -7.5
    float ceilingY = 3.5f;
    float lightZ1 = -4.0f; // Light opening front
    float lightZ2 = -7.0f; // Light opening back
    
    // Left part of ceiling (z from -9 to lightZ1)
    triangles.push_back(Triangle(c010, Vec3f(-4.5f, ceilingY, lightZ1), Vec3f(-4.5f, ceilingY, 0.0f), Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(c010, Vec3f(-4.5f, ceilingY, 0.0f), c011, Vec3f(0), 0.0f, whiteWall));
    // Right part of ceiling (z from -9 to lightZ1)
    triangles.push_back(Triangle(Vec3f(4.5f, ceilingY, lightZ1), c110, c111, Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(Vec3f(4.5f, ceilingY, lightZ1), c111, Vec3f(4.5f, ceilingY, 0.0f), Vec3f(0), 0.0f, whiteWall));
    // Front part of ceiling (z > lightZ1)
    triangles.push_back(Triangle(Vec3f(-4.5f, ceilingY, lightZ1), Vec3f(4.5f, ceilingY, lightZ1), Vec3f(4.5f, ceilingY, 0.0f), Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(Vec3f(-4.5f, ceilingY, lightZ1), Vec3f(4.5f, ceilingY, 0.0f), Vec3f(-4.5f, ceilingY, 0.0f), Vec3f(0), 0.0f, whiteWall));
    // Back part of ceiling (z < lightZ2)
    triangles.push_back(Triangle(c010, c110, Vec3f(4.5f, ceilingY, lightZ2), Vec3f(0), 0.0f, whiteWall));
    triangles.push_back(Triangle(c010, Vec3f(4.5f, ceilingY, lightZ2), Vec3f(-4.5f, ceilingY, lightZ2), Vec3f(0), 0.0f, whiteWall));

    // Area light on ceiling (centered, size ~2.5x2.5)
    float lightY = 3.49f; // Slightly below ceiling to avoid z-fighting
    Vec3f l0(-1.25f, lightY, -4.0f);
    Vec3f l1(1.25f, lightY, -4.0f);
    Vec3f l2(1.25f, lightY, -7.0f);
    Vec3f l3(-1.25f, lightY, -7.0f);
    Vec3f lightEmission(0.8f, 0.8f, 0.8f); // Light intensity
    triangles.push_back(Triangle(l0, l1, l2, lightEmission, 0.0f, Vec3f(0.0f)));
    triangles.push_back(Triangle(l0, l2, l3, lightEmission, 0.0f, Vec3f(0.0f)));

    // Objects in Cornell Box
    // Blue sphere (right side, slightly forward) - reduced reflectivity to show more of its own color
    spheres.push_back(Sphere(Vec3f(1.5f, -4.0f, -6.0f), 1.2f, Vec3f(0.6f, 0.7f, 0.9f), Vec3f(0), 0.1f));

    // White cube (left side, slightly back)

    float cubeSize = 2.5f;
    Vec3f cubeCenter(-1.5f, -5.25f + cubeSize / 2, -5.5f);
    Vec3f cubeMin = cubeCenter - Vec3f(cubeSize / 2);
    Vec3f cubeMax = cubeCenter + Vec3f(cubeSize / 2);

    // Cube faces (6 faces, each with 2 triangles)
    Vec3f cubeColor(0.9f, 0.9f, 0.9f);
    // Front face (z = cubeMax.z)
    // 立方体顶点纹理坐标定义
    // 前后面使用不同的UV
    // 前脸
    // 带有纹理的立方体

    // 前脸
    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,  // u0, v0
    1, 0,  // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    0, 1)); // u2, v2

    // 后脸
    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMin.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    0, 1,   // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMin.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    1, 0)); // u2, v2

    // 左脸
    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMin.x, cubeMin.y, cubeMax.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 0,   // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMax.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMin.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    0, 1)); // u2, v2

    // 右脸
    triangles.push_back(Triangle(
    Vec3f(cubeMax.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    0, 1,   // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMax.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    1, 0)); // u2, v2

    // 上脸
    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMax.y, cubeMin.z), 
    Vec3f(cubeMin.x, cubeMax.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    0, 1,   // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMax.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMax.z), 
    Vec3f(cubeMax.x, cubeMax.y, cubeMin.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    1, 0)); // u2, v2

    // 下脸
    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 0,   // u1, v1
    1, 1)); // u2, v2

    triangles.push_back(Triangle(
    Vec3f(cubeMin.x, cubeMin.y, cubeMin.z), 
    Vec3f(cubeMax.x, cubeMin.y, cubeMax.z), 
    Vec3f(cubeMin.x, cubeMin.y, cubeMax.z),
    Vec3f(0), 0.0f, Vec3f(1,1,1), &crateTexture,
    0, 0,   // u0, v0
    1, 1,   // u1, v1
    0, 1)); // u2, v2
    Vec3f cameraPos(0.0f, -2.0f, 5.0f); // Camera at z=5, centered
    Vec3f lookAt(0.0f, -2.0f, -5.0f);   // Looking at center of box
    Vec3f forward = lookAt - cameraPos;
    forward.normalize();
    Vec3f up(0.0f, 1.0f, 0.0f);
    Vec3f right = forward.cross(up);
    right.normalize();
    Vec3f cameraUp = right.cross(forward);

    float fov = 50.0f;
    float invWidth = 1.0f / width, invHeight = 1.0f / height;
    float angle = tanf(M_PI * 0.5f * fov / 180.0);
    float aspectRatio = (float)width / height;

    // Render to framebuffer
    std::cout << "Rendering Cornell Box...\n";
    for (unsigned y = 0; y < height; ++y)
    {
        if (y % 50 == 0)
            std::cout << "Progress: " << (100 * y / height) << "%\n";
        for (unsigned x = 0; x < width; ++x)
        {
            // Calculate ray direction using proper camera basis
            float u = (2.0f * ((x + 0.5f) * invWidth) - 1.0f) * angle * aspectRatio;
            float v = (1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;
            Vec3f raydir = forward + right * u + cameraUp * v;
            raydir.normalize();
            Vec3f hdrColor = trace(cameraPos, raydir, spheres, triangles, 0);
            // Apply tone mapping to convert HDR to LDR
            Vec3f color = toneMap(hdrColor, 1.0f, 2.2f);
            framebuffer[y * width + x] = color;
        }
    }
    std::cout << "Rendering complete.\n";

    // --- SDL2 display ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Ray Tracer with Mirror",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             width, height);
    if (!texture)
    {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Convert framebuffer to SDL texture format (RGBA)
    uint32_t *pixels = new uint32_t[width * height];
    for (unsigned i = 0; i < width * height; ++i)
    {
        unsigned char r = (unsigned char)(std::min(1.0f, framebuffer[i].x) * 255);
        unsigned char g = (unsigned char)(std::min(1.0f, framebuffer[i].y) * 255);
        unsigned char b = (unsigned char)(std::min(1.0f, framebuffer[i].z) * 255);
        unsigned char a = 255;
        pixels[i] = (r << 24) | (g << 16) | (b << 8) | a; // RGBA format
    }

    SDL_UpdateTexture(texture, nullptr, pixels, width * sizeof(uint32_t));
    delete[] pixels;

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}