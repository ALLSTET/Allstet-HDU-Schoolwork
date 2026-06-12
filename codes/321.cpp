// 图的邻接矩阵与邻接表示例程序
// 支持：
// 1) 根据输入顶点数、边数、顶点信息、边信息建立邻接矩阵（有向/无向），并在邻接矩阵上执行DFS和BFS；
// 2) 根据输入建立有向图的邻接表，并在邻接表上执行DFS和BFS。
//
// 输入格式示例（邻接矩阵部分）：
// n m           // 顶点数 n，边数 m
// v1 v2 v3 ...  // n 个顶点标签（用字符串）
// a b           // m 条边，每条边用两个顶点标签或索引（标签需与前面一致）
// directed 0/1  // 可选：0 表示无向图，1 表示有向图
//
// 程序会先构建邻接矩阵并输出 DFS 与 BFS 遍历序列，随后构建邻接表（针对有向图）并再次输出。

#include <iostream>
#include <string>
#include <cstring>
#include <cctype>
#include <queue>
#include <functional>

using namespace std;

const int MAXN = 200;  // 最大顶点数
const int MAXM = 2000; // 最大边数（用于邻接表存储）

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

void buildAdjMatrixAndTraverse()
{
  cout << "Adjacency matrix build and traversal example\n";
  cout << "Enter: n m (number of vertices, number of edges)\n";
  int n, m;
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
  string verts[MAXN];
  cout << "Enter " << n << " vertex labels (space separated):\n";
  for (int i = 0; i < n; ++i)
    cin >> verts[i];

  cout << "Is the graph directed? (0=undirected,1=directed):\n";
  int directed = 0;
  cin >> directed;

  static int mat[MAXN][MAXN];
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      mat[i][j] = 0;

  cout << "Enter " << m << " edges, each as: u v (vertex label or index), e.g. a b or 1 2\n";
  for (int i = 0; i < m; ++i)
  {
    string a, b;
    cin >> a >> b;
    int u = parseVertexToken(verts, n, a);
    int v = parseVertexToken(verts, n, b);
    if (u < 0 || v < 0)
    {
      cerr << "Invalid vertex: " << a << " or " << b << "\n";
      --i;
      continue;
    }
    mat[u][v] = 1;
    if (!directed)
      mat[v][u] = 1;
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

// 采用邻接表的前向星表示法：head[u] -> index in to/next arrays
static int toArr[MAXM];
static int nextArr[MAXM];
static int headArr[MAXN];
static int edgeCnt;

void initEdges()
{
  edgeCnt = 0;
  for (int i = 0; i < MAXN; ++i)
    headArr[i] = -1;
}

void addEdge(int u, int v)
{
  if (edgeCnt >= MAXM)
    return;
  toArr[edgeCnt] = v;
  nextArr[edgeCnt] = headArr[u];
  headArr[u] = edgeCnt;
  ++edgeCnt;
}

void buildAdjListAndTraverse()
{
  cout << "\nAdjacency list (directed graph) build and traversal example\n";
  cout << "Enter: n m (number of vertices, number of arcs)\n";
  int n, m;
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
  string verts[MAXN];
  cout << "Enter " << n << " vertex labels (space separated):\n";
  for (int i = 0; i < n; ++i)
    cin >> verts[i];

  initEdges();
  cout << "Enter " << m << " arcs, each as: u v (denotes u->v), e.g. a b or 1 2\n";
  for (int i = 0; i < m; ++i)
  {
    string a, b;
    cin >> a >> b;
    int u = parseVertexToken(verts, n, a);
    int v = parseVertexToken(verts, n, b);
    if (u < 0 || v < 0)
    {
      cerr << "Invalid vertex: " << a << " or " << b << "\n";
      --i;
      continue;
    }
    addEdge(u, v);
  }

  cout << "Adjacency list:\n";
  for (int i = 0; i < n; ++i)
  {
    cout << verts[i] << ": ";
    for (int e = headArr[i]; e != -1; e = nextArr[e])
      cout << verts[toArr[e]] << ' ';
    cout << '\n';
  }

  // DFS (邻接表)
  static int visited[MAXN];
  for (int i = 0; i < n; ++i)
    visited[i] = 0;
  function<void(int)> dfs = [&](int u)
  {
    visited[u] = 1;
    cout << verts[u] << ' ';
    for (int e = headArr[u]; e != -1; e = nextArr[e])
    {
      int v = toArr[e];
      if (!visited[v])
        dfs(v);
    }
  };
  cout << "DFS traversal (adj list):\n";
  for (int i = 0; i < n; ++i)
    if (!visited[i])
      dfs(i);
  cout << '\n';

  // BFS (邻接表)
  for (int i = 0; i < n; ++i)
    visited[i] = 0;
  cout << "BFS traversal (adj list):\n";
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
      for (int e = headArr[u]; e != -1; e = nextArr[e])
      {
        int v = toArr[e];
        if (!visited[v])
        {
          visited[v] = 1;
          q.push(v);
        }
      }
    }
  }
  cout << '\n';
}

int main()
{
  cout << "Choose option: 1=Adjacency matrix (directed/undirected) with DFS/BFS, 2=Adjacency list (directed) with DFS/BFS:\n";
  int op = 1;
  if (!(cin >> op))
    return 0;
  if (op == 1)
    buildAdjMatrixAndTraverse();
  else
    buildAdjListAndTraverse();

  getchar();
  getchar();
  return 0;
}
