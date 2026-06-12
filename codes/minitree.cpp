#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <queue>
#include <functional>
#include <algorithm>

using namespace std;

const int MAXN = 200;  // 最大顶点数

// 并查集结构用于Kruskal算法
class UnionFind {
private:
    int *parent;
    int n;
public:
    UnionFind(int size) {
        n = size;
        parent = new int[n];
        for(int i = 0; i < n; i++) parent[i] = i;
    }
    
    int find(int x) {
        if(parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    
    void unite(int x, int y) {
        parent[find(x)] = find(y);
    }
    
    ~UnionFind() { delete[] parent; }
};

int findIndex(const string verts[], int n, const string &s)
{
  for (int i = 0; i < n; ++i)
    if (verts[i] == s)
      return i;
  return -1;
}

int parseVertexToken(const string verts[], int n, const string &token)
{
  bool allDigits = true;
  for (size_t i = 0; i < token.size(); ++i)
    if (!isdigit((unsigned char)token[i]))
    {
      allDigits = false;
      break;
    }
  if (allDigits && !token.empty())
  {
    int val = atoi(token.c_str());
    if (val >= 1 && val <= n)
      return val - 1;
    if (val >= 0 && val < n)
      return val;
  }
  return findIndex(verts, n, token);
}

void buildMinitreePrim(const string verts[], int n, int adjMatrix[MAXN][MAXN]){
    bool inMST[MAXN];
    memset(inMST, 0, sizeof(inMST));
    
    string a;
    cout << "Pls enter the first vertex:" << endl;
    cin >> a;
    int start = parseVertexToken(verts, n, a);
    
    if(start < 0 || start >= n) {
        cout << "Invalid starting vertex!" << endl;
        return;
    }
    
    inMST[start] = true; // Start from the selected vertex
    cout << "Edges in the Minimum Spanning Tree:\n";
    
    for (int count = 0; count < n - 1; ++count) {
        int minWeight = INT_MAX;
        int u = -1, v = -1;
        
        // 遍历所有已经在 MST 中的顶点
        for (int x = 0; x < n; ++x) {
            if (inMST[x]) {
                // 遍历所有不在 MST 中的顶点
                for (int y = 0; y < n; ++y) {
                    if (!inMST[y] && adjMatrix[x][y] > 0 && adjMatrix[x][y] < minWeight) {
                        minWeight = adjMatrix[x][y];
                        u = x;
                        v = y;
                    }
                }
            }
        }
        
        if (u != -1 && v != -1) {
            inMST[v] = true;
            cout << verts[u] << " - " << verts[v] << " : " << minWeight << "\n";
        } else {
            cout << "The graph is not connected!" << endl;
            break;
        }
    }
}

void buildAdjMatrixAndTraverse(string verts[MAXN], int& n, int mat[MAXN][MAXN])
{
  cout << "Adjacency matrix build and traversal example\n";
  cout << "Enter: n m (number of vertices, number of edges)\n";
  int m;
  if (!(cin >> n >> m))
  {
    cerr << "Input error: expected n m\n";
    return;
  }
  if (n <= 0 || n > MAXN)
  {
    cerr << "n out of allowed range\n";
    return;
  }
  
  cout << "Enter " << n << " vertex labels (space separated):\n";
  for (int i = 0; i < n; ++i)
    cin >> verts[i];

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      mat[i][j] = 0;

  cout << "Enter " << m << " edges, each as: u v weight (vertex label or index), e.g. a b 5 or 1 2 3\n";
  for (int i = 0; i < m; ++i)
  {
    string a, b;
    int weight;
    cin >> a >> b >> weight;
    int u = parseVertexToken(verts, n, a);
    int v = parseVertexToken(verts, n, b);
    if (u < 0 || v < 0)
    {
      cerr << "Invalid vertex: " << a << " or " << b << "\n";
      --i;
      continue;
    }
    mat[u][v] = weight;
    mat[v][u] = weight;
  }

  cout << "Adjacency matrix:\n";
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
      cout << mat[i][j] << ' ';
    cout << '\n';
  }

  // DFS (矩阵)
  static int visited[MAXN];
  for (int i = 0; i < n; ++i)
    visited[i] = 0;
  function<void(int)> dfs = [&](int u)
  {
    visited[u] = 1;
    cout << verts[u] << ' ';
    for (int v = 0; v < n; ++v)
      if (mat[u][v] && !visited[v])
        dfs(v);
  };

  cout << "DFS traversal (matrix):\n";
  for (int i = 0; i < n; ++i)
    if (!visited[i])
      dfs(i);
  cout << '\n';

  // BFS (矩阵)
  for (int i = 0; i < n; ++i)
    visited[i] = 0;
  cout << "BFS traversal (matrix):\n";
  for (int i = 0; i < n; ++i)
  {
    if (visited[i])
      continue;
    queue<int> q;
    q.push(i);
    visited[i] = 1;
    while (!q.empty())
    {
      int u = q.front();
      q.pop();
      cout << verts[u] << ' ';
      for (int v = 0; v < n; ++v)
        if (mat[u][v] && !visited[v])
        {
          visited[v] = 1;
          q.push(v);
        }
    }
  }
  cout << '\n';
}

void buildMinitreeKruskal(const string verts[], int n, int adjMatrix[MAXN][MAXN]) {
    // 创建边的列表 - 使用数组代替vector
    int edges[MAXN * MAXN][3]; // [边数][权重, u, v]
    int edgeCount = 0;
    
    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            if(adjMatrix[i][j] > 0) {
                edges[edgeCount][0] = adjMatrix[i][j]; // 权重
                edges[edgeCount][1] = i; // 起点
                edges[edgeCount][2] = j; // 终点
                edgeCount++;
            }
        }
    }
    
    // 按权重排序
    for(int i = 0; i < edgeCount - 1; i++) {
        for(int j = 0; j < edgeCount - 1 - i; j++) {
            if(edges[j][0] > edges[j + 1][0]) {
                // 交换边
                for(int k = 0; k < 3; k++) {
                    int temp = edges[j][k];
                    edges[j][k] = edges[j + 1][k];
                    edges[j + 1][k] = temp;
                }
            }
        }
    }
    
    UnionFind uf(n);
    cout << "Edges in the Minimum Spanning Tree (Kruskal):\n";
    
    int mstEdgeCount = 0; // MST边计数器
    for(int i = 0; i < edgeCount; i++) {
        int weight = edges[i][0];
        int u = edges[i][1];
        int v = edges[i][2];
        
        if(uf.find(u) != uf.find(v)) {
            uf.unite(u, v);
            cout << verts[u] << " - " << verts[v] << " : " << weight << "\n";
            mstEdgeCount++;
            if(mstEdgeCount == n - 1) break; // MST已包含n-1条边时退出
        }
    }
}

int main()
{
    string verts[MAXN];
    int n;
    int mat[MAXN][MAXN];
    
    buildAdjMatrixAndTraverse(verts, n, mat);
    
    cout << "\nBuilding Minimum Spanning Tree using Prim's Algorithm:\n";
    buildMinitreePrim(verts, n, mat);
    
    cout << "\nBuilding Minimum Spanning Tree using Kruskal's Algorithm:\n";
    buildMinitreeKruskal(verts, n, mat);
    
    getchar();
    getchar();
    return 0;
}