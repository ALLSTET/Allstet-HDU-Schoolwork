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

            // 相机位置在 (0,0,15) 往回看
            Vec3f col = trace(Vec3f(0,0,15), dir, spheres, triangles, 0);
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

// 桌子函数
void addTable(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny) {
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float thickness = 0.2f; 
    float legWidth = 0.4f; 
    
    float topY_bottom = centerPos.y + height - thickness;
    float topY_top = centerPos.y + height;

    // 桌面
    addBox(tris, 
           Vec3f(centerPos.x - halfW, topY_bottom, centerPos.z - halfD), 
           Vec3f(centerPos.x + halfW, topY_top,    centerPos.z + halfD), 
           color, refl, shiny);

    // 四条腿
    addBox(tris, Vec3f(centerPos.x - halfW,          centerPos.y, centerPos.z + halfD - legWidth), Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z + halfD), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z + halfD - legWidth), Vec3f(centerPos.x + halfW,          topY_bottom, centerPos.z + halfD), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x - halfW,          centerPos.y, centerPos.z - halfD), Vec3f(centerPos.x - halfW + legWidth, topY_bottom, centerPos.z - halfD + legWidth), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x + halfW - legWidth, centerPos.y, centerPos.z - halfD), Vec3f(centerPos.x + halfW,          topY_bottom, centerPos.z - halfD + legWidth), color, 0, 0);
}

// 椅子函数
void addChair(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height, float depth, Vec3f color, float refl, float shiny) {
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float thickness = 0.2f; 
    float legWidth = 0.2f;  
    
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y, centerPos.z - halfD), Vec3f(centerPos.x + halfW, centerPos.y + thickness, centerPos.z + halfD), color, refl, shiny);
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y - thickness, centerPos.z - halfD), Vec3f(centerPos.x - halfW+0.1, centerPos.y + height*0.8, centerPos.z + halfD), color, refl, shiny);
    
    addBox(tris, Vec3f(centerPos.x - halfW,          centerPos.y - height, centerPos.z + halfD - legWidth), Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z + halfD), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z + halfD - legWidth), Vec3f(centerPos.x + halfW,          centerPos.y, centerPos.z + halfD), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x - halfW,          centerPos.y - height, centerPos.z - halfD), Vec3f(centerPos.x - halfW + legWidth, centerPos.y, centerPos.z - halfD + legWidth), color, 0, 0);
    addBox(tris, Vec3f(centerPos.x + halfW - legWidth, centerPos.y - height, centerPos.z - halfD), Vec3f(centerPos.x + halfW,          centerPos.y, centerPos.z - halfD + legWidth), color, 0, 0);
}

// 倾斜的杯子函数
void addTiltedCup(std::vector<Triangle>& tris, Vec3f center, float radius, float height, 
                  Vec3f color, float refl, float shiny, int segments, float tiltAngle) {
    
    float angleStep = 2 * M_PI / segments;
    float rad = tiltAngle * M_PI / 180.0f;
    float cosA = cos(rad);
    float sinA = sin(rad);

    float waterRatio = 0.8f;              
    float waterHeight = height * waterRatio;
    float waterRadius = radius * 0.90f;   
    Vec3f waterColor(0.2f, 0.6f, 1.0f);   
    float cupTransparency = 0.7f;         
    
    auto rotateX = [&](Vec3f v) -> Vec3f {
        return Vec3f(v.x, v.y * cosA - v.z * sinA, v.y * sinA + v.z * cosA);
    };

    Vec3f waterTopCenter = center + rotateX(Vec3f(0, waterHeight, 0));
    Vec3f cupBottomCenter = center;

    for (int i = 0; i < segments; ++i) {
        float theta1 = i * angleStep;
        float theta2 = (i + 1) * angleStep;

        float cx1 = radius * cos(theta1), cz1 = radius * sin(theta1);
        float cx2 = radius * cos(theta2), cz2 = radius * sin(theta2);
        
        float wx1 = waterRadius * cos(theta1), wz1 = waterRadius * sin(theta1);
        float wx2 = waterRadius * cos(theta2), wz2 = waterRadius * sin(theta2);

        Vec3f cB1 = center + rotateX(Vec3f(cx1, 0, cz1));
        Vec3f cB2 = center + rotateX(Vec3f(cx2, 0, cz2));
        Vec3f cT1 = center + rotateX(Vec3f(cx1, height, cz1));
        Vec3f cT2 = center + rotateX(Vec3f(cx2, height, cz2));

        Vec3f wB1 = center + rotateX(Vec3f(wx1, 0.02f, wz1));
        Vec3f wB2 = center + rotateX(Vec3f(wx2, 0.02f, wz2));
        Vec3f wT1 = center + rotateX(Vec3f(wx1, waterHeight, wz1));
        Vec3f wT2 = center + rotateX(Vec3f(wx2, waterHeight, wz2));

        tris.push_back(Triangle(cT1, cT2, cB2, color, refl, cupTransparency, 0, shiny));
        tris.push_back(Triangle(cT1, cB2, cB1, color, refl, cupTransparency, 0, shiny));
        tris.push_back(Triangle(cupBottomCenter, cB2, cB1, color, refl, cupTransparency, 0, shiny));

        addQuad(tris, wB1, wB2, wT2, wT1, waterColor, 0.1f, 20.f);
        tris.push_back(Triangle(waterTopCenter, wT2, wT1, waterColor, 0.3f, 0.5f, Vec3f(0.1), 80.f)); 
    }
}

//辅助函数，在指定位置种一棵树

void addTree(std::vector<Triangle>& tris, std::vector<Sphere>& spheres, Vec3f rootPos) {
    Vec3f trunkColor(0.4f, 0.25f, 0.15f); // 棕色树干
    Vec3f leafColor(0.15f, 0.55f, 0.15f); // 绿色树叶

    // A. 树干 (细长方体)
    float trunkW = 0.3f; 
    float trunkH = 7.0f; 
    
    addBox(tris, 
           Vec3f(rootPos.x - trunkW, rootPos.y,          rootPos.z - trunkW), 
           Vec3f(rootPos.x + trunkW, rootPos.y + trunkH, rootPos.z + trunkW), 
           trunkColor, 0, 0);

    // B. 树冠 (单球体)
    spheres.push_back(Sphere(
        Vec3f(rootPos.x, rootPos.y + trunkH, rootPos.z), 
        2.2f,         // 半径
        leafColor,    // 颜色
        0, 0, 0, 10.f // 材质
    ));
}
// 独立功能：添加窗框和玻璃
// centerPos: 窗户中心点
// width, height: 尺寸
void addWindowFrame(std::vector<Triangle>& tris, Vec3f centerPos, float width, float height) {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float border = 0.4f; // 边框宽
    float gridThick = 0.15f; // 格子宽

    Vec3f frameColor(0.25f, 0.15f, 0.05f); // 深木色
    Vec3f glassColor(0.9f, 0.9f, 1.0f);    // 玻璃色
    
    float zBack = centerPos.z;
    float zFace = centerPos.z + 0.2f; // 稍微突出一点

    // 1. 四周外框
    // 上
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y + halfH - border, zBack), Vec3f(centerPos.x + halfW, centerPos.y + halfH, zFace), frameColor, 0, 0);
    // 下
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y - halfH, zBack), Vec3f(centerPos.x + halfW, centerPos.y - halfH + border, zFace), frameColor, 0, 0);
    // 左
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y - halfH, zBack), Vec3f(centerPos.x - halfW + border, centerPos.y + halfH, zFace), frameColor, 0, 0);
    // 右
    addBox(tris, Vec3f(centerPos.x + halfW - border, centerPos.y - halfH, zBack), Vec3f(centerPos.x + halfW, centerPos.y + halfH, zFace), frameColor, 0, 0);

    // 2. 十字格 (Grid)
    addBox(tris, Vec3f(centerPos.x - halfW, centerPos.y - gridThick/2, zBack), Vec3f(centerPos.x + halfW, centerPos.y + gridThick/2, zFace-0.05f), frameColor, 0, 0);
    addBox(tris, Vec3f(centerPos.x - gridThick/2, centerPos.y - halfH, zBack), Vec3f(centerPos.x + gridThick/2, centerPos.y + halfH, zFace-0.05f), frameColor, 0, 0);

    // 3. 玻璃 (透明度0.95)
    float glassZ = zFace - 0.1f;
    Vec3f bl(centerPos.x - halfW + border, centerPos.y - halfH + border, glassZ);
    Vec3f br(centerPos.x + halfW - border, centerPos.y - halfH + border, glassZ);
    Vec3f tr(centerPos.x + halfW - border, centerPos.y + halfH - border, glassZ);
    Vec3f tl(centerPos.x - halfW + border, centerPos.y + halfH - border, glassZ);

    tris.push_back(Triangle(bl, br, tr, glassColor, 0.2f, 0.95f, 0, 50.f));
    tris.push_back(Triangle(bl, tr, tl, glassColor, 0.2f, 0.95f, 0, 50.f));
}

// 综合函数：构建带洞的后墙 + 自动安窗户 + 自动种树 + 自动打光
void addWindowedBackWall(
    std::vector<Triangle>& tris,      
    std::vector<Sphere>& spheres,     
    Vec3f lbb, Vec3f rbb, Vec3f rtb, Vec3f ltb, // 后墙四个角
    Vec3f color,                      
    float winBottomY, float winTopY,  
    float winLeftX, float winRightX   
) {
    float wallZ = lbb.z;

    // --- 1. 拆分墙体 (挖出窗户洞) ---
    // A. 下墙
    addQuad(tris, lbb, rbb, Vec3f(rbb.x, winBottomY, wallZ), Vec3f(lbb.x, winBottomY, wallZ), color);
    // B. 上墙
    addQuad(tris, Vec3f(lbb.x, winTopY, wallZ), Vec3f(rbb.x, winTopY, wallZ), rtb, ltb, color);
    // C. 左墙柱
    addQuad(tris, Vec3f(lbb.x, winBottomY, wallZ), Vec3f(winLeftX, winBottomY, wallZ), Vec3f(winLeftX, winTopY, wallZ), Vec3f(lbb.x, winTopY, wallZ), color);
    // D. 右墙柱
    addQuad(tris, Vec3f(winRightX, winBottomY, wallZ), Vec3f(rbb.x, winBottomY, wallZ), Vec3f(rbb.x, winTopY, wallZ), Vec3f(winRightX, winTopY, wallZ), color);

    // --- 2. 自动安装窗框 (调用独立函数) ---
    float winW = winRightX - winLeftX;
    float winH = winTopY - winBottomY;
    float centerX = (winLeftX + winRightX) / 2.0f;
    float centerY = (winBottomY + winTopY) / 2.0f;
    
    // 调用之前提取的 addWindowFrame
    addWindowFrame(tris, Vec3f(centerX, centerY, wallZ), winW, winH);

    // --- 3. 自动种树 (调用独立函数) ---
    // 计算树的位置：在后墙外面 (wallZ - 8)，稍微偏右一点 (centerX + 4)
    Vec3f treePos(centerX + 4.0f, -6.0f, wallZ - 8.0f);
    
    // 调用之前提取的 addTree
    addTree(tris, spheres, treePos);

    // --- 4. 自动打光 ---
     // 光源位置：在树的前部 (treePos.z + 2.2f)
     Vec3f lightPos(treePos.x, treePos.y, treePos.z + 2.2f);
     spheres.push_back(Sphere(lightPos, 0.5f, Vec3f(1, 1, 1), 0, 0, 0, 10.f)); // 白色光源


    // 天空板
    float skyZ = wallZ - 50.0f;
    Vec3f skyBlue(0.6f, 0.8f, 1.0f);
    tris.push_back(Triangle(Vec3f(-50, -10, skyZ), Vec3f(50, -10, skyZ), Vec3f(50, 50, skyZ), Vec3f(0), 0, 0, skyBlue * 2.0f));
    tris.push_back(Triangle(Vec3f(-50, -10, skyZ), Vec3f(50, 50, skyZ), Vec3f(-50, 50, skyZ), Vec3f(0), 0, 0, skyBlue * 2.0f));
}

// 辅助：添加方形吊灯罩 (Lampshade)
// pos: 光源球体的中心位置
// ceilingY: 天花板高度 (用于连接灯线)
void addLampShade(std::vector<Triangle>& tris, Vec3f pos, float ceilingY) {
    // 灯罩参数
    float topW = 0.4f;  // 顶部宽度 (较窄)
    float botW = 2.0f;  // 底部宽度 (较宽)
    float height = 1.5f; // 灯罩高度
    
    // 坐标计算
    // 灯罩顶部 Y (靠近天花板)
    float topY = pos.y + 0.8f; 
    // 灯罩底部 Y (光源下方)
    float botY = topY - height; 

    // 材质
    Vec3f shadeColor(0.15f, 0.15f, 0.15f); // 深灰色外壳 (不反光，遮光性好)
    Vec3f innerColor(0.9f, 0.8f, 0.6f);    // 内壁颜色 (亮米色)

    // --- 顶点计算 ---
    // 顶部四点
    Vec3f tlf(pos.x - topW, topY, pos.z + topW); // Top Left Front
    Vec3f trf(pos.x + topW, topY, pos.z + topW);
    Vec3f trb(pos.x + topW, topY, pos.z - topW);
    Vec3f tlb(pos.x - topW, topY, pos.z - topW);

    // 底部四点
    Vec3f blf(pos.x - botW, botY, pos.z + botW); // Bottom Left Front
    Vec3f brf(pos.x + botW, botY, pos.z + botW);
    Vec3f brb(pos.x + botW, botY, pos.z - botW);
    Vec3f blb(pos.x - botW, botY, pos.z - botW);

    // --- 绘制灯罩 (4个侧面) ---
    // 注意：addQuad 是双面可见的，但在光线追踪中，为了逻辑清晰，
    // 这里我们实际上只需要画一层，光线会被阻挡。
    
    // 前面
    addQuad(tris, blf, brf, trf, tlf, shadeColor);
    // 后面
    addQuad(tris, brb, blb, tlb, trb, shadeColor);
    // 左面
    addQuad(tris, blb, blf, tlf, tlb, shadeColor);
    // 右面
    addQuad(tris, brf, brb, trb, trf, shadeColor);

    // --- 绘制顶部封口 (防止光向上漏) ---
    addQuad(tris, tlf, trf, trb, tlb, shadeColor);

    // --- 绘制吊线 (Cord) ---
    // 从灯罩顶部画一根细线连到天花板
    float cordW = 0.05f;
    addBox(tris, 
           Vec3f(pos.x - cordW, topY, pos.z - cordW),
           Vec3f(pos.x + cordW, ceilingY, pos.z + cordW),
           Vec3f(0.1f) // 黑色电线
    );
}


void addLeaningPictureFrame(std::vector<Triangle>& tris, Vec3f bottomPos, float width, float height, float tiltOffset) {
    float halfW = width * 0.5f;
    float border = 0.3f; // 边框宽度
    float thick = 0.1f;  // 画框厚度

    // 材质
    Vec3f frameColor(0.25f, 0.15f, 0.05f); // 深木色边框
    Vec3f canvasColor(1.0f, 1.0f, 1.0f);   // 纯白画布 (用于贴图)

    // --- 1. 计算关键坐标 ---
    // 底部 Z 和 顶部 Z (倾斜逻辑)
    float baseZ = bottomPos.z;
    float topZ  = bottomPos.z - tiltOffset; // 向后倾斜 (Z变小)

    // 底部 Y 和 顶部 Y
    float baseY = bottomPos.y;
    float topY  = bottomPos.y + height;

    // 四个外角 (Outer Corners) - 用于画边框背景板
    Vec3f bl(bottomPos.x - halfW, baseY, baseZ); // Bottom Left
    Vec3f br(bottomPos.x + halfW, baseY, baseZ); // Bottom Right
    Vec3f tl(bottomPos.x - halfW, topY,  topZ);  // Top Left
    Vec3f tr(bottomPos.x + halfW, topY,  topZ);  // Top Right

    // --- 2. 绘制边框 (Backing Board) ---
    // 为了简单，我们画一个大的木板作为底座
    addQuad(tris, bl, br, tr, tl, frameColor, 0.1f, 10.f);
    
    // 如果想要侧面厚度，可以再加 box 逻辑，这里暂略，只做一个有厚度的板
    Vec3f offset(0, 0, thick); // 简单的厚度偏移 (不严谨但够用)
    addQuad(tris, bl+offset, br+offset, tr+offset, tl+offset, frameColor); 

    // --- 3. 绘制画布 (Canvas) --- 
    // 这是核心部分！后续纹理映射主要针对这个四边形
    
    // 画布比边框稍微小一点 (inset)
    // 同时也稍微向前凸出一点 (interp)，防止重叠闪烁
    float t = 0.2f; // 向前突出的厚度
    
    // 简单的线性插值计算内框坐标，让它贴合倾斜面
    // 我们手动缩小左右和上下
    Vec3f c_bl(bottomPos.x - halfW + border, baseY + border, baseZ + t);
    Vec3f c_br(bottomPos.x + halfW - border, baseY + border, baseZ + t);
    Vec3f c_tl(bottomPos.x - halfW + border, topY  - border, topZ  + t);
    Vec3f c_tr(bottomPos.x + halfW - border, topY  - border, topZ  + t);

    // [纹理映射区域] 
    // 目前是纯白色。之后你可以在 Triangle 类中加入 UV，并在这里传入 uv0(0,0), uv1(1,0), uv2(1,1)...
    addQuad(tris, c_bl, c_br, c_tr, c_tl, canvasColor, 0.0f, 0.0f); 
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    SDL_Window* window = SDL_CreateWindow("Raytracer - Back Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 800, 600);

    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;

    // --- 材质定义 ---
    Vec3f wallColor(0.9f, 0.88f, 0.82f); 
    Vec3f floorColor(0.45f, 0.35f, 0.25f); 
    Vec3f ceilingColor(0.95f);
    Vec3f tableColor(0.2f, 0.15f, 0.1f);
    Vec3f vividRedCup(0.9f, 0.1f, 0.1f); 
    
    // --- 房间几何体 ---
    float wX = 14.0f, fY = -6.0f, cY = 10.0f, bZ = -45.0f, fZ = 1.0f;
    Vec3f lbf(-wX, fY, fZ), lbb(-wX, fY, bZ), ltb(-wX, cY, bZ), ltf(-wX, cY, fZ);
    Vec3f rbf(wX, fY, fZ), rbb(wX, fY, bZ), rtb(wX, cY, bZ), rtf(wX, cY, fZ);

    addQuad(triangles, lbf, lbb, rbb, rbf, floorColor, 0.25f, 30.f); // 地板
    addQuad(triangles, ltf, ltb, rtb, rtf, ceilingColor); // 天花板
    addQuad(triangles, lbf, lbb, ltb, ltf, wallColor); // 左墙
    addQuad(triangles, rbf, rbb, rtb, rtf, wallColor); // 右墙 (恢复实心墙)
    
    // --- 修改：后墙开窗 ---
    // 调用新函数替代原来的 addQuad 后墙
    // 窗户范围：高度 0 到 6，X轴范围 -6 到 6 (居中大窗)
    addWindowedBackWall(triangles, spheres, lbb, rbb, rtb, ltb, wallColor, 
                        0.0f, 6.0f, -6.0f, 6.0f); 

    // --- 家具 ---
    addTable(triangles, Vec3f(0, fY, -24), 10.0f, 3.0f, 8.0f, tableColor, 0.1f, 10.f); 
    addChair(triangles, Vec3f(-7, fY+1.5f, -23), 2.5f, 3.0f, 2.0f, tableColor, 0.1f, 10.f);

    float tableSurfaceY = fY + 3.0f; 
    addTiltedCup(triangles, Vec3f(-1.5f, tableSurfaceY, -24), 0.5f, 1.5f, vividRedCup, 0.1f, 100.f, 64, 10); 

    // --- 室内光源 ---
    spheres.push_back(Sphere(Vec3f(-20, 8, -20), 1.0f, 0, 0, 0, Vec3f(150, 180, 255)));
    spheres.push_back(Sphere(Vec3f(0, 5, 25), 1.0f, 0, 0, 0, Vec3f(160, 160, 160)));

    // --- 倾斜相框 ---
    addLeaningPictureFrame(triangles, Vec3f(10.0f, fY, -42.0f), 4.0f, 5.0f, 2.5f);
   

    Vec3f lightPos(0, cY - 1.5f, -25); 

    // 添加光源球体 (稍微调小一点，确保在灯罩内部)
    spheres.push_back(Sphere(lightPos, 0.8f, 0, 0, 0, Vec3f(50, 44, 33))); 
    
    // 添加灯罩
    // 参数: 三角形列表, 光源位置, 天花板高度(cY)
    addLampShade(triangles, lightPos+Vec3f(0, 0.5f, 0), cY);

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