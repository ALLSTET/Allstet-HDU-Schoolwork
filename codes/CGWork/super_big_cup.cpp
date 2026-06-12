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

#if defined __linux__ || defined __APPLE__
#else
#define M_PI 3.141592653589793
#define INFINITY 1e8
#endif

// --- 基础向量类 ---
template<typename T>
class Vec3 {
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Vec3& normalize() {
        T nor2 = length2();
        if (nor2 > 0) {
            T invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
    Vec3<T> operator * (const T& f) const { return Vec3<T>(x * f, y * f, z * f); }
    Vec3<T> operator * (const Vec3<T>& v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    Vec3<T> operator / (const T& f) const { return Vec3<T>(x / f, y / f, z / f); }
    Vec3<T> operator / (const Vec3<T>& v) const { return Vec3<T>(x / v.x, y / v.y, z / v.z); }
    T dot(const Vec3<T>& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3<T> operator - (const Vec3<T>& v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator + (const Vec3<T>& v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    Vec3<T>& operator += (const Vec3<T>& v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vec3<T>& operator *= (const Vec3<T>& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
    T length2() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length2()); }
};

typedef Vec3<float> Vec3f;

// --- 几何体类 ---

class Sphere {
public:
    Vec3f center;
    float radius, radius2;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection, shininess;

    Sphere(const Vec3f& c, const float& r, const Vec3f& sc, const float& refl = 0, const float& transp = 0, const Vec3f& ec = 0, const float& shiny = 0) :
        center(c), radius(r), radius2(r* r), surfaceColor(sc), emissionColor(ec),
        transparency(transp), reflection(refl), shininess(shiny) {
    }

    bool intersect(const Vec3f& rayorig, const Vec3f& raydir, float& t0, float& t1) const {
        Vec3f l = center - rayorig;
        float tca = l.dot(raydir);
        if (tca < 0) return false;
        float d2 = l.dot(l) - tca * tca;
        if (d2 > radius2) return false;
        float thc = sqrt(radius2 - d2);
        t0 = tca - thc;
        t1 = tca + thc;
        return true;
    }
};

class Triangle {
public:
    Vec3f v0, v1, v2;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection, shininess;

    Triangle(const Vec3f& a, const Vec3f& b, const Vec3f& c, const Vec3f& sc, const float& refl = 0, const float& transp = 0, const Vec3f& ec = 0, const float& shiny = 0)
        : v0(a), v1(b), v2(c), surfaceColor(sc), emissionColor(ec), transparency(transp), reflection(refl), shininess(shiny) {
    }

    bool intersect(const Vec3f& orig, const Vec3f& dir, float& t, float& u, float& v) const {
        const float EPSILON = 1e-6f;
        Vec3f edge1 = v1 - v0;
        Vec3f edge2 = v2 - v0;
        Vec3f pvec = Vec3f(dir.y * edge2.z - dir.z * edge2.y, dir.z * edge2.x - dir.x * edge2.z, dir.x * edge2.y - dir.y * edge2.x);
        float det = edge1.dot(pvec);
        if (fabs(det) < EPSILON) return false;
        float invDet = 1.0f / det;
        Vec3f tvec = orig - v0;
        u = tvec.dot(pvec) * invDet;
        if (u < 0.0f || u > 1.0f) return false;
        Vec3f qvec = Vec3f(tvec.y * edge1.z - tvec.z * edge1.y, tvec.z * edge1.x - tvec.x * edge1.z, tvec.x * edge1.y - tvec.y * edge1.x);
        v = dir.dot(qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) return false;
        t = edge2.dot(qvec) * invDet;
        if (t <= EPSILON) return false;
        return true;
    }

    Vec3f normal() const {
        Vec3f e1 = v1 - v0;
        Vec3f e2 = v2 - v0;
        Vec3f n = Vec3f(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
        n.normalize();
        return n;
    }
};

#define MAX_RAY_DEPTH 5

float mix(const float& a, const float& b, const float& mix) { return b * mix + a * (1 - mix); }

Vec3f trace(const Vec3f& rayorig, const Vec3f& raydir, const std::vector<Sphere>& spheres, const std::vector<Triangle>& triangles, const int& depth) {
    float tnear = INFINITY;
    int hitType = 0; 
    int hitIndex = -1;

    for (unsigned i = 0; i < spheres.size(); ++i) {
        float t0 = INFINITY, t1 = INFINITY;
        if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < tnear) { tnear = t0; hitType = 1; hitIndex = int(i); }
        }
    }
    for (unsigned i = 0; i < triangles.size(); ++i) {
        float t = INFINITY, u = 0, v = 0;
        if (triangles[i].intersect(rayorig, raydir, t, u, v)) {
            if (t < tnear) { tnear = t; hitType = 2; hitIndex = int(i); }
        }
    }

    if (hitType == 0) return Vec3f(0.01); 

    Vec3f phit = rayorig + raydir * tnear;
    Vec3f nhit, surfaceColor, emission;
    float transparency = 0, reflection = 0, shininess = 0;

    if (hitType == 1) {
        const Sphere& s = spheres[hitIndex];
        nhit = phit - s.center; nhit.normalize();
        surfaceColor = s.surfaceColor; emission = s.emissionColor;
        transparency = s.transparency; reflection = s.reflection; shininess = s.shininess;
    }
    else {
        const Triangle& t = triangles[hitIndex];
        nhit = t.normal();
        if (raydir.dot(nhit) > 0) nhit = -nhit;
        surfaceColor = t.surfaceColor; emission = t.emissionColor;
        transparency = t.transparency; reflection = t.reflection; shininess = t.shininess;
    }

    if (emission.x > 0) return emission; 

    Vec3f finalColor = 0;
    float bias = 1e-4f;

    if ((transparency > 0 || reflection > 0) && depth < MAX_RAY_DEPTH) {
        float facingratio = -raydir.dot(nhit);
        float fresnel = mix(pow(1 - facingratio, 3), 1, 0.1);
        Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit); refldir.normalize();
        Vec3f reflColor = trace(phit + nhit * bias, refldir, spheres, triangles, depth + 1);
        Vec3f refrColor = 0;

        if (transparency > 0) {
            float ior = 1.1f, eta = 1 / ior;
            float cosi = -nhit.dot(raydir);
            float k = 1 - eta * eta * (1 - cosi * cosi);
            if (k >= 0) {
                Vec3f refrdir = raydir * eta + nhit * (eta * cosi - sqrt(k)); refrdir.normalize();
                refrColor = trace(phit - nhit * bias, refrdir, spheres, triangles, depth + 1);
            }
        }
        finalColor += (reflColor * fresnel + refrColor * (1 - fresnel) * transparency) * surfaceColor;
    }

    if (reflection < 1.0f) {
        Vec3f lightAmt = 0, specAmt = 0;
        Vec3f viewDir = -raydir; viewDir.normalize();

        for (const auto& light : spheres) {
            if (light.emissionColor.x > 0) {
                Vec3f lightDir = light.center - phit;
                float dist2 = lightDir.dot(lightDir);
                float dist = sqrt(dist2);
                lightDir.normalize();

                bool shadow = false;
                for (const auto& s : spheres) {
                    if (&s == &light) continue;
                    float t0, t1;
                    if (s.intersect(phit + nhit * bias, lightDir, t0, t1)) {
                        if ((t0 < 0 ? t1 : t0) < dist) { shadow = true; break; }
                    }
                }
                if (!shadow) {
                    for (const auto& t : triangles) {
                        float th, u, v;
                        if (t.intersect(phit + nhit * bias, lightDir, th, u, v)) {
                            if (th > 0 && th < dist) { shadow = true; break; }
                        }
                    }
                }

                if (!shadow) {
                    float att = 1.0f / dist2;
                    float diff = std::max(0.0f, nhit.dot(lightDir));
                    float spec = 0.0f;
                    if (diff > 0 && shininess > 0) {
                        Vec3f half = (lightDir + viewDir); half.normalize();
                        spec = pow(std::max(0.0f, nhit.dot(half)), shininess);
                    }
                    lightAmt += light.emissionColor * att * diff;
                    specAmt += light.emissionColor * att * spec;
                }
            }
        }
        if (transparency == 0) finalColor += (Vec3f(0.02) * surfaceColor) + lightAmt * surfaceColor + specAmt;
    }
    return finalColor;
}

inline float clamp(float v) { return std::max(0.0f, std::min(1.0f, v)); }
inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) { return (r << 24) | (g << 16) | (b << 8) | 255; }

Vec3f toneMapping(Vec3f c) {
    Vec3f x = c * 0.6f;
    return (x * (x * 2.51f + 0.03f)) / (x * (x * 2.43f + 0.59f) + 0.14f);
}

uint32_t* render(const std::vector<Sphere>& spheres, const std::vector<Triangle>& triangles) {
    unsigned w = 800, h = 600; 
    uint32_t* buf = new uint32_t[w * h];
    float invW = 1 / float(w), invH = 1 / float(h);
    float angle = tan(M_PI * 0.5 * 40 / 180.); 
    float aspect = w / float(h);

    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; ++x) {
            float xx = (2 * ((x + 0.5) * invW) - 1) * angle * aspect;
            float yy = (1 - 2 * ((y + 0.5) * invH)) * angle;
            Vec3f dir(xx, yy, -1); dir.normalize();

            Vec3f col = trace(Vec3f(0), dir, spheres, triangles, 0);
            col = toneMapping(col);

            buf[y * w + x] = RGB(
                (char)(255 * clamp(pow(col.x, 1 / 2.2f))),
                (char)(255 * clamp(pow(col.y, 1 / 2.2f))),
                (char)(255 * clamp(pow(col.z, 1 / 2.2f)))
            );
        }
    }
    return buf;
}

// 辅助：添加四边形
void addQuad(std::vector<Triangle>& tris, Vec3f v0, Vec3f v1, Vec3f v2, Vec3f v3, Vec3f c, float refl = 0, float shiny = 0) {
    tris.push_back(Triangle(v0, v1, v2, c, refl, 0, 0, shiny));
    tris.push_back(Triangle(v0, v2, v3, c, refl, 0, 0, shiny));
}

// 辅助：添加立方体 (AABB)
void addBox(std::vector<Triangle>& tris, Vec3f min, Vec3f max, Vec3f c, float refl = 0, float shiny = 0) {
    Vec3f v0(min.x, min.y, max.z); Vec3f v1(max.x, min.y, max.z);
    Vec3f v2(min.x, max.y, max.z); Vec3f v3(max.x, max.y, max.z);
    Vec3f v4(min.x, min.y, min.z); Vec3f v5(max.x, min.y, min.z);
    Vec3f v6(min.x, max.y, min.z); Vec3f v7(max.x, max.y, min.z);

    addQuad(tris, v4, v5, v7, v6, c, refl, shiny); // Front
    addQuad(tris, v1, v0, v2, v3, c, refl, shiny); // Back
    addQuad(tris, v6, v7, v3, v2, c, refl, shiny); // Top
    addQuad(tris, v4, v0, v1, v5, c, refl, shiny); // Bottom
    addQuad(tris, v0, v4, v6, v2, c, refl, shiny); // Left
    addQuad(tris, v5, v1, v3, v7, c, refl, shiny); // Right
}

// --- 新增：添加桌子函数 ---
// 参数：三角形数组，地面位置中心(x, y, z)，桌面尺寸(宽, 高, 深)，颜色，反射率，光泽度
void addTable(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny) {
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
           Vec3f(centerPos.x + halfW, topY_top,    centerPos.z + halfD), 
           color, refl, shiny);

    // 2. 四条腿
    // 左前腿
    addBox(tris,
           Vec3f(centerPos.x - halfW,          centerPos.y, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z + halfD),
           color, 0, 0); // 桌腿通常不反光，或者反光较少
           
    // 右前腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x + halfW,          topY_bottom, centerPos.z + halfD),
           color, 0, 0);

    // 左后腿
    addBox(tris,
           Vec3f(centerPos.x - halfW,          centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z - halfD + legWidth),
           color, 0, 0);

    // 右后腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW,          topY_bottom, centerPos.z - halfD + legWidth),
           color, 0, 0);
}

void addChair(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny) {
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
           Vec3f(centerPos.x - halfW,          centerPos.y - height, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z + halfD),
           color, 0, 0); // 椅子腿通常不反光，或者反光较少
           
    // 右前腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z + halfD - legWidth),
           Vec3f(centerPos.x + halfW,          centerPos.y, centerPos.z + halfD),
           color, 0, 0);

    // 左后腿
    addBox(tris,
           Vec3f(centerPos.x - halfW,          centerPos.y - height, centerPos.z - halfD),
           Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z - halfD + legWidth),
           color, 0, 0);

    // 右后腿
    addBox(tris,
           Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z - halfD),
           Vec3f(centerPos.x + halfW,          centerPos.y, centerPos.z - halfD + legWidth),
           color, 0, 0);
}

void addCabinet(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny) {
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
// 辅助：添加可倾斜的圆柱形杯子
// center: 杯底中心点
// tiltAngle: 绕 X 轴倾斜的角度（度数），正值通常向屏幕外倾斜
void addTiltedCup(std::vector<Triangle>& tris, Vec3f center, float radius, float height, Vec3f color, float refl, float shiny, int segments, float tiltAngle) {
    float angleStep = 2 * M_PI / segments;
    
    // 预计算旋转矩阵的 cos 和 sin
    float rad = tiltAngle * M_PI / 180.0f; // 角度转弧度
    float cosA = cos(rad);
    float sinA = sin(rad);

    // 旋转辅助 Lambda：绕 X 轴旋转向量 v
    auto rotateX = [&](Vec3f v) -> Vec3f {
        // 绕 X 轴旋转公式:
        // y' = y*cos - z*sin
        // z' = y*sin + z*cos
        return Vec3f(
            v.x, 
            v.y * cosA - v.z * sinA, 
            v.y * sinA + v.z * cosA
        );
    };

    for (int i = 0; i < segments; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;

        // 1. 生成局部坐标 (假设杯子在原点 (0,0,0) 直立)
        // 底部点
        Vec3f localB1(radius * cos(theta1), 0, radius * sin(theta1));
        Vec3f localB2(radius * cos(theta2), 0, radius * sin(theta2));
        // 顶部点 (y = height)
        Vec3f localT1(radius * cos(theta1), height, radius * sin(theta1));
        Vec3f localT2(radius * cos(theta2), height, radius * sin(theta2));

        // 2. 先应用旋转，再平移到 center 位置
        Vec3f b1 = center + rotateX(localB1);
        Vec3f b2 = center + rotateX(localB2);
        Vec3f t1 = center + rotateX(localT1);
        Vec3f t2 = center + rotateX(localT2);

        // 3. 构建三角形 (和之前一样)
        addQuad(tris, b1, b2, t2, t1, color, refl, shiny); // 侧壁
        tris.push_back(Triangle(center, b2, b1, color, refl, 0, 0, shiny)); // 底面 (注意中心点其实也应该考虑旋转，如果底面不是平的)
        
        // 修正：底面中心点 center 其实也代表了旋转轴心。
        // 如果你需要底面中心也跟着倾斜（比如杯子悬空旋转），逻辑会更复杂。
        // 但如果杯子是放在桌上倾斜（支点在中心），上面的代码是正确的：底面中心不动。
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_Window* window = SDL_CreateWindow("Raytracer - Normal Room", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 800, 600);

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;

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

    addQuad(triangles, lbf, lbb, rbb, rbf, floorColor, 0.25f, 30.f); // 地板
    addQuad(triangles, ltf, ltb, rtb, rtf, ceilingColor); // 天花板
    addQuad(triangles, lbf, lbb, ltb, ltf, wallColor); // 左墙
    addQuad(triangles, rbf, rbb, rtb, rtf, wallColor); // 右墙
    addQuad(triangles, lbb, rbb, rtb, ltb, wallColor); // 后墙

    // --- 家具：调用 addTable 函数 ---
    // 位置(0, -6, -24)，尺寸 10x3x8
    addTable(triangles, Vec3f(0, fY, -24), 10.0f, 3.0f, 8.0f, tableColor, 0.1f, 10.f); // 桌子(10x3x8)
    //addCabinet(triangles, Vec3f(-14, fY, -24), 8.0f, 3.0f, 5.0f, tableColor, 0.1f, 10.f);
    addChair(triangles, Vec3f(-4, fY, -20), 4.0f, 3.0f, 2.0f, tableColor, 0.1f, 10.f);

    // --- 装饰品 ---
    float tableSurfaceY = fY + 3.0f; // 桌面高度
    addTiltedCup(triangles, Vec3f(-1.5f, tableSurfaceY, -24), 1.0f, 2.5f, glassColor, 0.1f, 100.f, 64, 10); // 杯子 ()
    

    // --- 光源 ---
    spheres.push_back(Sphere(Vec3f(0, cY - 0.5f, -25), 2.0f, 0, 0, 0, Vec3f(120, 110, 100))); 
    spheres.push_back(Sphere(Vec3f(-20, 8, -20), 1.0f, 0, 0, 0, Vec3f(150, 180, 255)));     

    // 渲染
    uint32_t* buf = render(spheres, triangles);
    SDL_UpdateTexture(texture, nullptr, buf, 800 * 4);
    delete[] buf;

    bool run = true;
    SDL_Event e;
    while (run) {
        while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) run = false;
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture); SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
    return 0;
}