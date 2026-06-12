#include <iostream>

// 假设 L[1..n], R[1..n] 已定义，索引从 1 开始
bool isDescendant(int u, int v, int L[], int R[], int n) {
    if (v == 0) return false; // v 是空节点
    if (v == u) return true;  // 找到 u，说明 u 是 v 的后代（包括自身）
    
    // 递归检查左子树和右子树
    return isDescendant(u, L[v], L, R, n) || 
           isDescendant(u, R[v], L, R, n);
}

int main(){
  int L[6] = {0, 2, 4, 0, 0, 0}; // 示例左子树数组
  int R[6] = {0, 3, 5, 0, 0, 0}; // 示例右子树数组
  int u, v;
  std::cout << "Enter u and v: ";
  std::cin >> u >> v;
  if (isDescendant(u, v, L, R, 5)) {
    std::cout << u << " is " << v << "'s descendant." << std::endl;
  } else {
    std::cout << u << " is not " << v << "'s descendant." << std::endl;
  }
  getchar();
  getchar();
  return 0;
}