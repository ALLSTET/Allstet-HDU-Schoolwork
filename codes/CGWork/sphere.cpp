#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>

// ========== 向量与网格结构 ==========
struct Vec3
{
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    Vec3 operator+(const Vec3 &b) const { return Vec3(x + b.x, y + b.y, z + b.z); }
    Vec3 operator-(const Vec3 &b) const { return Vec3(x - b.x, y - b.y, z - b.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }

    Vec3 normalized() const
    {
        double len = std::sqrt(x * x + y * y + z * z);
        return (len > 1e-9) ? Vec3(x / len, y / len, z / len) : Vec3(0, 0, 0);
    }
};

struct Face
{
    int v1, v2, v3;
};

struct Mesh
{
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Face> faces;
};

// ========== 工具函数：边中点缓存 ==========
int getMiddlePoint(int p1, int p2, std::map<long long, int> &cache, std::vector<Vec3> &vertices)
{
    long long smaller = std::min(p1, p2);
    long long greater = std::max(p1, p2);
    long long key = (smaller << 32) + greater;

    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;

    Vec3 v1 = vertices[p1];
    Vec3 v2 = vertices[p2];
    Vec3 middle = Vec3(
                      (v1.x + v2.x) / 2.0,
                      (v1.y + v2.y) / 2.0,
                      (v1.z + v2.z) / 2.0)
                      .normalized();

    int index = (int)vertices.size();
    vertices.push_back(middle);
    cache[key] = index;
    return index;
}

// ========== 生成正二十面体球 ==========
Mesh createIcocylinder(int recursionLevel)
{
    Mesh mesh;

    int slices = (recursionLevel > 0) ? std::max(3, recursionLevel * 8) : 16;
    double radius = 1.0;
    double height = 2.0;
    double halfH = height * 0.5;

    mesh.vertices.clear();
    mesh.faces.clear();
    mesh.normals.clear();

    // 侧面顶点（按 pair 存放：2*i 为底圈点，2*i+1 为顶圈点）
    for (int i = 0; i < slices; ++i)
    {
        double ang = 2.0 * M_PI * i / slices;
        double cx = std::cos(ang) * radius;
        double cz = std::sin(ang) * radius;
        mesh.vertices.push_back(Vec3(cx, -halfH, cz)); // bottom
        mesh.vertices.push_back(Vec3(cx, halfH, cz));  // top
    }

    // 底顶中心点
    int bottomCenter = (int)mesh.vertices.size();
    mesh.vertices.push_back(Vec3(0.0, -halfH, 0.0));
    int topCenter = (int)mesh.vertices.size();
    mesh.vertices.push_back(Vec3(0.0, halfH, 0.0));

    // 侧面：每个切片两三角形
    for (int i = 0; i < slices; ++i)
    {
        int b0 = 2 * i;
        int t0 = 2 * i + 1;
        int b1 = 2 * ((i + 1) % slices);
        int t1 = 2 * ((i + 1) % slices) + 1;

        mesh.faces.push_back({b0, b1, t1});
        mesh.faces.push_back({b0, t1, t0});
    }

    // 底盖与顶盖（扇形）
    for (int i = 0; i < slices; ++i)
    {
        int b0 = 2 * i;
        int b1 = 2 * ((i + 1) % slices);
        mesh.faces.push_back({bottomCenter, b1, b0});

        int t0 = 2 * i + 1;
        int t1 = 2 * ((i + 1) % slices) + 1;
        mesh.faces.push_back({topCenter, t0, t1});
    }

    // 法线：侧面为 (x,0,z) 单位向量，中心点为上下向量
    mesh.normals.resize(mesh.vertices.size());
    for (size_t i = 0; i < mesh.vertices.size(); ++i)
    {
        if ((int)i == bottomCenter)
        {
            mesh.normals[i] = Vec3(0.0, -1.0, 0.0);
        }
        else if ((int)i == topCenter)
        {
            mesh.normals[i] = Vec3(0.0, 1.0, 0.0);
        }
        else
        {
            Vec3 v = mesh.vertices[i];
            mesh.normals[i] = Vec3(v.x, 0.0, v.z).normalized();
        }
    }

    return mesh;
}

Mesh createIcosphere(int recursionLevel)
{
    Mesh mesh;

    const double t = (1.0 + std::sqrt(5.0)) / 2.0;

    // 12 顶点
    mesh.vertices = {
        Vec3(-1, t, 0).normalized(),
        Vec3(1, t, 0).normalized(),
        Vec3(-1, -t, 0).normalized(),
        Vec3(1, -t, 0).normalized(),

        Vec3(0, -1, t).normalized(),
        Vec3(0, 1, t).normalized(),
        Vec3(0, -1, -t).normalized(),
        Vec3(0, 1, -t).normalized(),

        Vec3(t, 0, -1).normalized(),
        Vec3(t, 0, 1).normalized(),
        Vec3(-t, 0, -1).normalized(),
        Vec3(-t, 0, 1).normalized(),
    };

    // 20 面
    mesh.faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11}, {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8}, {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9}, {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}};

    // 细分
    for (int i = 0; i < recursionLevel; ++i)
    {
        std::map<long long, int> cache;
        std::vector<Face> newFaces;
        for (auto &tri : mesh.faces)
        {
            int a = getMiddlePoint(tri.v1, tri.v2, cache, mesh.vertices);
            int b = getMiddlePoint(tri.v2, tri.v3, cache, mesh.vertices);
            int c = getMiddlePoint(tri.v3, tri.v1, cache, mesh.vertices);

            newFaces.push_back({tri.v1, a, c});
            newFaces.push_back({tri.v2, b, a});
            newFaces.push_back({tri.v3, c, b});
            newFaces.push_back({a, b, c});
        }
        mesh.faces.swap(newFaces);
    }

    // 法线与顶点相同（单位球）
    mesh.normals = mesh.vertices;

    return mesh;
}

// ========== 输出 OBJ 文件 ==========
void exportOBJ(const Mesh &mesh, const std::string &filename)
{
    std::ofstream out(filename);
    if (!out.is_open())
    {
        std::cerr << "File creating failed!:" << filename << std::endl;
        return;
    }

    // 顶点
    for (const auto &v : mesh.vertices)
        out << "v " << v.x << " " << v.y << " " << v.z << "\n";

    // 顶点法线
    for (const auto &n : mesh.normals)
        out << "vn " << n.x << " " << n.y << " " << n.z << "\n";

    // 面（三角形）
    for (const auto &f : mesh.faces)
        out << "f "
            << f.v1 + 1 << "//" << f.v1 + 1 << " "
            << f.v2 + 1 << "//" << f.v2 + 1 << " "
            << f.v3 + 1 << "//" << f.v3 + 1 << "\n";

    out.close();
    std::cout << "OBJ File exporting successfully!: " << filename << std::endl;
}

// ========== 主函数 ==========
int main()
{
    int level = 4; // 细分层数（0~5推荐）
    Mesh Cylinder = createIcocylinder(level);

    std::cout << "RecursionLevel: " << level << std::endl;
    std::cout << "Vertices: " << Cylinder.vertices.size() << std::endl;
    std::cout << "Faces: " << Cylinder.faces.size() << std::endl;

    exportOBJ(Cylinder, "./assets/sphere.obj");
    getchar();
    return 0;
}
