// [compile]
// c++ -o normal_room -O3 -Wall normal_room.cpp `sdl2-config --cflags --libs`
// [/compile]

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include "SDL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define samplesPerPixel 4  //超采样4x

struct SamplePattern {
    float offsetX, offsetY;
};  //泊松采样

SamplePattern getPoissonSamples(int sampleCount) {
    // 这里可以预定义泊松磁盘采样模式
    // 示例：4个采样点的泊松模式
    static SamplePattern poisson4[] = {
        {-0.326212f, -0.40581f},
        {-0.840144f, -0.07358f},
        {-0.695914f, 0.457137f},
        {0.5f, -0.0865401f}
    };
    return poisson4[sampleCount % 4];
}

#if defined __linux__ || defined __APPLE__
#else

#endif

// --- 基础向量类 ---
template <typename T>
class Vec3
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Vec3 &normalize()
    {
        T nor2 = length2();
        if (nor2 > 0)
        {
            T invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
    Vec3<T> operator*(const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
    Vec3<T> operator*(const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    Vec3<T> operator/(const T &f) const { return Vec3<T>(x / f, y / f, z / f); }
    Vec3<T> operator/(const Vec3<T> &v) const { return Vec3<T>(x / v.x, y / v.y, z / v.z); }
    T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3<T> operator-(const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator+(const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    Vec3<T> &operator+=(const Vec3<T> &v)
    {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }
    Vec3<T> &operator*=(const Vec3<T> &v)
    {
        x *= v.x, y *= v.y, z *= v.z;
        return *this;
    }
    Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }
    T length2() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length2()); }
};

typedef Vec3<float> Vec3f;

//--- 纹理类 ---
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

// --- 几何体类 ---

class Sphere
{
public:
    Vec3f center;
    float radius, radius2;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection, shininess;

    Sphere(const Vec3f &c, const float &r, const Vec3f &sc, const float &refl = 0, const float &transp = 0, const Vec3f &ec = 0, const float &shiny = 0) : center(c), radius(r), radius2(r * r), surfaceColor(sc), emissionColor(ec),
                                                                                                                                                           transparency(transp), reflection(refl), shininess(shiny)
    {
    }

    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        Vec3f l = center - rayorig;
        float tca = l.dot(raydir);
        if (tca < 0)
            return false;
        float d2 = l.dot(l) - tca * tca;
        if (d2 > radius2)
            return false;
        float thc = sqrt(radius2 - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true;
    }
};

class Triangle
{
public:
    Vec3f v0, v1, v2;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection, shininess;
    float uv0[2], uv1[2], uv2[2]; // （新增）纹理坐标 (u,v)
    const Texture* texture;     // （新增）纹理指针
    bool hasTexture;            // （新增）是否有纹理

    Triangle(const Vec3f &a, const Vec3f &b, const Vec3f &c, 
             const Vec3f &sc, const float &refl = 0, const float &transp = 0, 
             const Vec3f &ec = 0, const float &shiny = 0,
             float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 0, 
             float u2 = 0, float v2 = 1, const Texture* tex = nullptr)
        : v0(a), v1(b), v2(c), surfaceColor(sc), emissionColor(ec), 
          transparency(transp), reflection(refl), shininess(shiny),
          texture(tex), hasTexture(tex != nullptr)
    {
        uv0[0] = u0; uv0[1] = v0;
        uv1[0] = u1; uv1[1] = v1;
        uv2[0] = u2; uv2[1] = v2;
    }

    // 在Triangle类中添加纹理采样方法
    Vec3f sampleTexture(float u, float v) const
    {
        if (!hasTexture || texture == nullptr) {
            return surfaceColor; // 如果没有纹理，返回表面颜色
        }
    
        // 使用重心坐标插值来计算纹理坐标
        // 在intersect函数中获取重心坐标后调用此方法
        return texture->sample(u, v);
    }

    // 根据重心坐标获取纹理颜色
    Vec3f getColorAtBarycentricCoords(float bary_u, float bary_v) const
    {
        if (!hasTexture || texture == nullptr) {
            return surfaceColor;
        }
    
        // 计算插值后的纹理坐标
        float u = uv0[0] * (1 - bary_u - bary_v) + uv1[0] * bary_u + uv2[0] * bary_v;
        float v = uv0[1] * (1 - bary_u - bary_v) + uv1[1] * bary_u + uv2[1] * bary_v;
    
        return texture->sample(u, v);
    }
    
    bool intersect(const Vec3f &orig, const Vec3f &dir, float &t, float &u, float &v) const
    {
        const float EPSILON = 1e-6f;
        Vec3f edge1 = v1 - v0;
        Vec3f edge2 = v2 - v0;
        Vec3f pvec = Vec3f(dir.y * edge2.z - dir.z * edge2.y, 
                           dir.z * edge2.x - dir.x * edge2.z, 
                           dir.x * edge2.y - dir.y * edge2.x);
        float det = edge1.dot(pvec);
        if (fabs(det) < EPSILON)
            return false;
        float invDet = 1.0f / det;
        Vec3f tvec = orig - v0;
        u = tvec.dot(pvec) * invDet;  
        if (u < 0.0f || u > 1.0f)
            return false;
        Vec3f qvec = Vec3f(tvec.y * edge1.z - tvec.z * edge1.y, 
                           tvec.z * edge1.x - tvec.x * edge1.z, 
                           tvec.x * edge1.y - tvec.y * edge1.x);
        v = dir.dot(qvec) * invDet;   
        if (v < 0.0f || u + v > 1.0f)
            return false;
        t = edge2.dot(qvec) * invDet;
        if (t <= EPSILON)
            return false;
        return true;
    }

    Vec3f normal() const
    {
        Vec3f e1 = v1 - v0;
        Vec3f e2 = v2 - v0;
        Vec3f n = Vec3f(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
        n.normalize();
        return n;
    }
};

#define MAX_RAY_DEPTH 5

float mix(const float &a, const float &b, const float &mix) { return b * mix + a * (1 - mix); }

Vec3f trace(const Vec3f &rayorig, const Vec3f &raydir, const std::vector<Sphere> &spheres, const std::vector<Triangle> &triangles, const int &depth)
{
    float tnear = INFINITY;
    float saved_u = 0, saved_v = 0;
    int hitType = 0;
    int hitIndex = -1;

    for (unsigned i = 0; i < spheres.size(); ++i)
    {
        float t0 = INFINITY, t1 = INFINITY;
        if (spheres[i].intersect(rayorig, raydir, t0, t1))
        {
            if (t0 < 0)
                t0 = t1;
            if (t0 < tnear)
            {
                tnear = t0;
                hitType = 1;
                hitIndex = int(i);
            }
        }
    }
    for (unsigned i = 0; i < triangles.size(); ++i)
    {
        float t = INFINITY, u = 0, v = 0;
        if (triangles[i].intersect(rayorig, raydir, t, u, v))
        {
            if (t < tnear)
            {
                tnear = t;
                hitType = 2;
                hitIndex = int(i);
                saved_u = u;
                saved_v = v;
            }
        }
    }

    if (hitType == 0)
        return Vec3f(0.01);

    Vec3f phit = rayorig + raydir * tnear;
    Vec3f nhit, surfaceColor, emission;
    float transparency = 0, reflection = 0, shininess = 0;

    if (hitType == 1)
    {
        const Sphere &s = spheres[hitIndex];
        nhit = phit - s.center;
        nhit.normalize();
        surfaceColor = s.surfaceColor;
        emission = s.emissionColor;
        transparency = s.transparency;
        reflection = s.reflection;
        shininess = s.shininess;
    }
    else
    {
        const Triangle &t = triangles[hitIndex];
        nhit = t.normal();
        if (raydir.dot(nhit) > 0)
            nhit = -nhit;

        // 如果有纹理，使用纹理颜色；否则使用表面颜色
        if (t.hasTexture && t.texture != nullptr) {
        // 这里需要根据交点位置计算对应的纹理坐标
        // 由于intersect函数返回的是重心坐标，我们需要在交点处插值纹理坐标
        // 这需要修改intersect函数或在trace中计算
            float dist = tnear;
            Vec3f hitPoint = rayorig + raydir * dist;           // 使用三角形的平均纹理坐标或根据射线参数计算重心坐标
            surfaceColor = t.getColorAtBarycentricCoords(saved_u, saved_v); // u,v需要正确计算
        }else {
            surfaceColor = t.surfaceColor;
        }

        emission = t.emissionColor;
        transparency = t.transparency;
        reflection = t.reflection;
        shininess = t.shininess;
    }

    if (emission.x > 0)
        return emission;

    Vec3f finalColor = 0;
    float bias = 1e-4f;

    if ((transparency > 0 || reflection > 0) && depth < MAX_RAY_DEPTH)
    {
        float facingratio = -raydir.dot(nhit);
        float fresnel = mix(pow(1 - facingratio, 3), 1, 0.1);
        Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
        refldir.normalize();
        Vec3f reflColor = trace(phit + nhit * bias, refldir, spheres, triangles, depth + 1);
        Vec3f refrColor = 0;

        if (transparency > 0)
        {
            float ior = 1.1f, eta = 1 / ior;
            float cosi = -nhit.dot(raydir);
            float k = 1 - eta * eta * (1 - cosi * cosi);
            if (k >= 0)
            {
                Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k));
                refrdir.normalize();
                refrColor = trace(phit - nhit * bias, refrdir, spheres, triangles, depth + 1);
            }
        }
        finalColor += (reflColor * fresnel + refrColor * (1 - fresnel) * transparency) * surfaceColor;
    }

    if (reflection < 1.0f)
    {
        Vec3f lightAmt = 0, specAmt = 0;
        Vec3f viewDir = -raydir;
        viewDir.normalize();

        for (const auto &light : spheres)
        {
            if (light.emissionColor.x > 0)
            {
                Vec3f lightDir = light.center - phit;
                float dist2 = lightDir.dot(lightDir);
                float dist = sqrt(dist2);
                lightDir.normalize();

                bool shadow = false;
                for (const auto &s : spheres)
                {
                    if (&s == &light)
                        continue;
                    float t0, t1;
                    if (s.intersect(phit + nhit * bias, lightDir, t0, t1))
                    {
                        if ((t0 < 0 ? t1 : t0) < dist)
                        {
                            shadow = true;
                            break;
                        }
                    }
                }
                if (!shadow)
                {
                    for (const auto &t : triangles)
                    {
                        float th, u, v;
                        if (t.intersect(phit + nhit * bias, lightDir, th, u, v))
                        {
                            if (th > 0 && th < dist)
                            {
                                shadow = true;
                                break;
                            }
                        }
                    }
                }

                if (!shadow)
                {
                    float att = 1.0f / dist2;
                    float diff = std::max(0.0f, nhit.dot(lightDir));
                    float spec = 0.0f;
                    if (diff > 0 && shininess > 0)
                    {
                        Vec3f half = (lightDir + viewDir);
                        half.normalize();
                        spec = pow(std::max(0.0f, nhit.dot(half)), shininess);
                    }
                    lightAmt += light.emissionColor * att * diff;
                    specAmt += light.emissionColor * att * spec;
                }
            }
        }
        if (transparency == 0)
            finalColor += (Vec3f(0.02) * surfaceColor) + lightAmt * surfaceColor + specAmt;
    }
    return finalColor;
}

inline float clamp(float v) { return std::max(0.0f, std::min(1.0f, v)); }
inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) { return (r << 24) | (g << 16) | (b << 8) | 255; }

// 创建带纹理坐标的三角形
Triangle createTexturedTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c,
                               const Texture* tex,
                               float u0, float v0, float u1, float v1, float u2, float v2,
                               const float &refl = 0, const float &transp = 0, 
                               const Vec3f &ec = 0, const float &shiny = 0)
{
    return Triangle(a, b, c, Vec3f(1, 1, 1), refl, transp, ec, shiny,
                    u0, v0, u1, v1, u2, v2, tex);
}

Vec3f toneMapping(Vec3f c)
{
    Vec3f x = c * 0.6f;
    return (x * (x * 2.51f + 0.03f)) / (x * (x * 2.43f + 0.59f) + 0.14f);
}

uint32_t *render(const std::vector<Sphere> &spheres, const std::vector<Triangle> &triangles)
{
    unsigned w = 800, h = 600;
    uint32_t *buf = new uint32_t[w * h];
    float invW = 1 / float(w), invH = 1 / float(h);
    float angle = tan(M_PI * 0.5 * 40 / 180.);
    float aspect = w / float(h);

    for (unsigned y = 0; y < h; ++y)
    {
        for (unsigned x = 0; x < w; ++x)
        {   //应用开头定义的采样模式（泊松模式，4x超采样）
            Vec3f accumulatedColor(0, 0, 0);
            for (int s = 0; s < samplesPerPixel; ++s)
            {
                SamplePattern sp = getPoissonSamples(s);
                float xx = (2 * ((x + 0.5f + sp.offsetX * 0.5f) * invW) - 1) * angle * aspect;
                float yy = (1 - 2 * ((y + 0.5f + sp.offsetY * 0.5f) * invH)) * angle;
                Vec3f dir(xx, yy, -1);
                dir.normalize();

                Vec3f col = trace(Vec3f(0), dir, spheres, triangles, 0);
                accumulatedColor += col;
            }


            Vec3f col = accumulatedColor / samplesPerPixel;
            col = toneMapping(col);

            buf[y * w + x] = RGB(
                (char)(255 * clamp(pow(col.x, 1 / 2.2f))),
                (char)(255 * clamp(pow(col.y, 1 / 2.2f))),
                (char)(255 * clamp(pow(col.z, 1 / 2.2f))));
        }
    }
    return buf;
}

// 辅助：添加四边形
// 修改addQuad函数以支持纹理坐标
void addQuad(std::vector<Triangle> &tris, Vec3f v0, Vec3f v1, Vec3f v2, Vec3f v3, 
             Vec3f c, float refl = 0, float shiny = 0,
             const Texture* tex = nullptr)
{
    // 为每个三角形指定合适的纹理坐标
    float uv0[2] = {0, 0}, uv1[2] = {1, 0}, uv2[2] = {1, 1}, uv3[2] = {0, 1};
    
    if (tex) {
        tris.push_back(createTexturedTriangle(v0, v1, v2, tex, 
                                             uv0[0], uv0[1], uv1[0], uv1[1], uv2[0], uv2[1], 
                                             refl, 0, 0, shiny));
        tris.push_back(createTexturedTriangle(v0, v2, v3, tex, 
                                             uv0[0], uv0[1], uv2[0], uv2[1], uv3[0], uv3[1], 
                                             refl, 0, 0, shiny));
    } else {
        tris.push_back(Triangle(v0, v1, v2, c, refl, 0, 0, shiny));
        tris.push_back(Triangle(v0, v2, v3, c, refl, 0, 0, shiny));
    }
}

// 辅助：添加立方体 (AABB)
// --- 已修改addBox函数以支持纹理坐标 ---
void addBox(std::vector<Triangle> &tris, Vec3f min, Vec3f max, Vec3f c, float refl = 0, float shiny = 0, const Texture* tex = nullptr)
{
    Vec3f v0(min.x, min.y, max.z);
    Vec3f v1(max.x, min.y, max.z);
    Vec3f v2(min.x, max.y, max.z);
    Vec3f v3(max.x, max.y, max.z);
    Vec3f v4(min.x, min.y, min.z);
    Vec3f v5(max.x, min.y, min.z);
    Vec3f v6(min.x, max.y, min.z);
    Vec3f v7(max.x, max.y, min.z);

    addQuad(tris, v4, v5, v7, v6, c, refl, shiny, tex); // Front
    addQuad(tris, v1, v0, v2, v3, c, refl, shiny, tex); // Back
    addQuad(tris, v6, v7, v3, v2, c, refl, shiny, tex); // Top
    addQuad(tris, v4, v0, v1, v5, c, refl, shiny, tex); // Bottom
    addQuad(tris, v0, v4, v6, v2, c, refl, shiny, tex); // Left
    addQuad(tris, v5, v1, v3, v7, c, refl, shiny, tex); // Right
}

// --- 新增：添加桌子函数（此方法支持纹理） ---
// 参数：三角形数组，地面位置中心(x, y, z)，桌面尺寸(宽, 高, 深)，颜色，反射率，光泽度
void addTable(std::vector<Triangle> &tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny, const Texture* tex = nullptr)
{
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float thickness = 0.2f; // 桌面厚度
    float legWidth = 0.4f;  // 桌腿宽度

    // 桌面 Y 坐标范围
    float topY_bottom = centerPos.y + height - thickness;
    float topY_top = centerPos.y + height;

    // 1. 桌面
    addBox(tris,
           Vec3f(centerPos.x - halfW, topY_bottom, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW, topY_top, centerPos.z + halfD),
           color, refl, shiny, tex); // 添加纹理

    // 2. 四条腿
    // 左前腿
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z + halfD),
           color, 0, 0, tex); // 桌腿也可以使用相同纹理或传入特定纹理

    // 右前腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x + halfW, topY_bottom, centerPos.z + halfD),
           color, 0, 0, tex);

    // 左后腿
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z - halfD + legWidth),
           color, 0, 0, tex);

    // 右后腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW, topY_bottom, centerPos.z - halfD + legWidth),
           color, 0, 0, tex);
}
// --- 新增：添加椅子函数（此方法不支持纹理，因主函数未使用，暂未添加） ---
void addChair(std::vector<Triangle> &tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny)
{
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float thickness = 0.2f; // 椅子厚度
    float legWidth = 0.2f;  // 椅子腿宽度

    // 1. 椅子座位
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW, centerPos.y + thickness, centerPos.z + halfD),
           color, refl, shiny);

    // 2. 四条腿
    // 左前腿
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y - height, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z + halfD),
           color, 0, 0); // 椅子腿通常不反光，或者反光较少

    // 右前腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x + halfW, centerPos.y, centerPos.z + halfD),
           color, 0, 0);

    // 左后腿
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y - height, centerPos.z - halfD),
           Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z - halfD + legWidth),
           color, 0, 0);

    // 右后腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW, centerPos.y, centerPos.z - halfD + legWidth),
           color, 0, 0);
}
// --- 新增：添加柜子函数（此方法不支持纹理，因主函数未使用，暂未添加） ---
void addCabinet(std::vector<Triangle> &tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny)
{
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float thickness = 0.2f; // 柜子厚度
    float legWidth = 0.4f;  // 柜子腿宽度

    // 1. 柜子
    addBox(tris,
           Vec3f(centerPos.x - halfW, centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW, centerPos.y + height, centerPos.z + halfD),
           color, refl, shiny);
}
// 辅助：添加可倾斜的圆柱形杯子 --- 支持纹理 ---
// radius: 杯子半径
// center: 杯底中心点
// tiltAngle: 绕 X 轴倾斜的角度（度数），正值通常向屏幕外倾斜
// segments: 杯子侧壁分段数
void addTiltedCup(std::vector<Triangle> &tris, Vec3f center, float radius, float height, Vec3f color, float refl, float shiny, int segments, float tiltAngle, const Texture* tex = nullptr)
{
    float angleStep = 2 * M_PI / segments;

    // 预计算旋转矩阵的 cos 和 sin
    float rad = tiltAngle * M_PI / 180.0f; // 角度转弧度
    float cosA = cos(rad);
    float sinA = sin(rad);

    // 旋转辅助 Lambda：绕 X 轴旋转向量 v
    auto rotateX = [&](Vec3f v) -> Vec3f
    {
        // 绕 X 轴旋转公式:
        // y' = y*cos - z*sin
        // z' = y*sin + z*cos
        return Vec3f(
            v.x,
            v.y * cosA - v.z * sinA,
            v.y * sinA + v.z * cosA);
    };

    for (int i = 0; i < segments; ++i)
    {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;

        // 1. 生成局部坐标 (假设杯子在原点 (0,0,0) 直立)
        // 底部点
        Vec3f localB1(radius * cos(theta1), 0, radius * sin(theta1));
        Vec3f localB2(radius * cos(theta2), 0, radius * sin(theta1));
        // 顶部点 (y = height)
        Vec3f localT1(radius * cos(theta1), height, radius * sin(theta1));
        Vec3f localT2(radius * cos(theta2), height, radius * sin(theta2));

        // 2. 先应用旋转，再平移到 center 位置
        Vec3f b1 = center + rotateX(localB1);
        Vec3f b2 = center + rotateX(localB2);
        Vec3f t1 = center + rotateX(localT1);
        Vec3f t2 = center + rotateX(localT2);

        // 3. 构建三角形
        addQuad(tris, b1, b2, t2, t1, color, refl, shiny, tex);  // 侧壁，使用纹理
        
        // 创建杯底三角形，也可以使用纹理
        if (tex) {
            tris.push_back(createTexturedTriangle(center, b2, b1, tex, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, refl, 0, 0, shiny));
        } else {
            tris.push_back(Triangle(center, b2, b1, color, refl, 0, 0, shiny));
        }
    }
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 1;
    SDL_Window *window = SDL_CreateWindow("Raytracer - Normal Room", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 800, 600);

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;

    // --- 纹理加载 ---
    Texture walltexture;
    bool wallTexLoaded = loadTextureFromFile("textures/wall_texture.jpg", walltexture);
    Texture floortexture;
    bool floorTexLoaded = loadTextureFromFile("textures/floor_texture.jpg", floortexture);
    Texture ceilingtexture;
    bool ceilingTexLoaded = loadTextureFromFile("textures/ceiling_texture.jpg", ceilingtexture);
    Texture tabletexture;
    bool tableTexLoaded = loadTextureFromFile("textures/table_texture.jpg", tabletexture);
    Texture glasstexture;
    bool glassTexLoaded = loadTextureFromFile("textures/glass_texture.jpg", glasstexture);
    Texture mirrortexture;
    bool mirrorTexLoaded = loadTextureFromFile("textures/mirror_texture.jpg", mirrortexture);

    // --- 材质定义 ---
    Vec3f wallColor(0.9f, 0.88f, 0.82f);
    Vec3f floorColor(0.45f, 0.35f, 0.25f);
    Vec3f ceilingColor(0.95f);
    Vec3f tableColor(0.2f, 0.15f, 0.1f);
    Vec3f glassColor(0.9f, 0.95f, 1.0f);
    Vec3f mirrorColor(0.9f, 0.9f, 0.9f);

    // --- 房间几何体 ---
    float wX = 14.0f, fY = -6.0f, cY = 10.0f, bZ = -45.0f, fZ = 1.0f;
    Vec3f lbf(-wX, fY, fZ), lbb(-wX, fY, bZ), ltb(-wX, cY, bZ), ltf(-wX, cY, fZ);
    Vec3f rbf(wX, fY, fZ), rbb(wX, fY, bZ), rtb(wX, cY, bZ), rtf(wX, cY, fZ);
    // --- 房间六面体 ---
    if(floorTexLoaded){
        addQuad(triangles, lbf, lbb, rbb, rbf, floorColor, 0.25f, 30.f, &floortexture); // 地板
    }
    else{
        addQuad(triangles, lbf, lbb, rbb, rbf, floorColor, 0.25f, 30.f); // 地板
    }
    if(ceilingTexLoaded){
        addQuad(triangles, ltf, ltb, rtb, rtf, ceilingColor, 0.0f, 0.f, &ceilingtexture); // 天花板
    }
    else{
        addQuad(triangles, ltf, ltb, rtb, rtf, ceilingColor);            // 天花板
    }
    if(wallTexLoaded){
        addQuad(triangles, lbf, lbb, ltb, ltf, wallColor, 0.0f, 0.f, &walltexture);               // 左墙
        addQuad(triangles, rbf, rbb, rtb, rtf, wallColor, 0.0f, 0.f, &walltexture);               // 右墙
        addQuad(triangles, lbb, rbb, rtb, ltb, wallColor, 0.0f, 0.f, &walltexture);               // 后墙
    }
    else{
        addQuad(triangles, lbf, lbb, ltb, ltf, wallColor);               // 左墙
        addQuad(triangles, rbf, rbb, rtb, rtf, wallColor);               // 右墙
        addQuad(triangles, lbb, rbb, rtb, ltb, wallColor);               // 后墙
    }
    // --- 家具：调用 addTable 函数 ---
    // 位置(0, -6, -24)，尺寸 10x3x8
    if(tableTexLoaded){
        addTable(triangles, Vec3f(0, fY, -24), 10.0f, 3.0f, 8.0f, tableColor, 0.1f, 10.f, &tabletexture); // 桌子(10x3x8)
    }else{
        addTable(triangles, Vec3f(0, fY, -24), 10.0f, 3.0f, 8.0f, tableColor, 0.1f, 10.f); // 桌子(10x3x8)
    }
    // addCabinet(triangles, Vec3f(-14, fY, -24), 8.0f, 3.0f, 5.0f, tableColor, 0.1f, 10.f);
    addChair(triangles, Vec3f(-4, fY, -20), 4.0f, 3.0f, 2.0f, tableColor, 0.1f, 10.f);

    // --- 装饰品 ---
    float tableSurfaceY = fY + 3.0f;                                                                        // 桌面高度
    if(glassTexLoaded){
        addTiltedCup(triangles, Vec3f(-1.5f, tableSurfaceY, -24), 1.0f, 2.5f, glassColor, 0.1f, 100.f, 64, 10, &glasstexture); // 杯子 ()
    }else{
        addTiltedCup(triangles, Vec3f(-1.5f, tableSurfaceY, -24), 1.0f, 2.5f, glassColor, 0.1f, 100.f, 64, 10); // 杯子 ()
    }
    // --- 光源 ---
    spheres.push_back(Sphere(Vec3f(0, cY - 0.5f, -25), 2.0f, 0, 0, 0, Vec3f(120, 110, 100)));
    spheres.push_back(Sphere(Vec3f(-20, 8, -20), 1.0f, 0, 0, 0, Vec3f(150, 180, 255)));

    // 渲染
    uint32_t *buf = render(spheres, triangles);
    SDL_UpdateTexture(texture, nullptr, buf, 800 * 4);
    delete[] buf;

    bool run = true;
    SDL_Event e;
    while (run)
    {
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT)
                run = false;
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}