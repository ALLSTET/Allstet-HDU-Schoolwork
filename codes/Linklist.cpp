#include <iostream>

typedef int Elemtype;
struct LNode
{
  Elemtype data;
  struct LNode *next;
};
typedef LNode *Linklist;

void initLinklist(Linklist L, Elemtype *arr, int n);
Linklist MergeLinklist(Linklist &L1, Linklist &L2);
void PrintList(Linklist L);

int main()
{
  Elemtype arr1[] = {1, 2, 2, 5, 6};
  Elemtype arr2[] = {2, 2, 3, 4, 7};
  Linklist L1 = new LNode;
  Linklist L2 = new LNode;

  initLinklist(L1, arr1, 5);
  initLinklist(L2, arr2, 5);

  std::cout << "L1: ";
  PrintList(L1);
  std::cout << "L2: ";
  PrintList(L2);

  Linklist L3 = MergeLinklist(L1, L2); // 复用 L1 头结点作为 L3
  std::cout << "L3: ";
  PrintList(L3);

  getchar();
  getchar();

  return 0;
}

void initLinklist(Linklist L, Elemtype *arr, int n)
{
  L->data = 0; // 头结点数据可用 0 或其他占位符
  L->next = nullptr;
  Linklist tail = L;
  for (int i = 0; i < n; ++i)
  {
    Linklist p = new LNode;
    p->data = arr[i];
    p->next = nullptr;
    tail->next = p;
    tail = p;
  }
  return;
}

Linklist MergeLinklist(Linklist &L1, Linklist &L2)
{
  Linklist p1 = L1->next;
  Linklist p2 = L2->next;
  Linklist L3 = L1;   // 复用 L1 的头结点作为 L3
  L3->next = nullptr; // 结果表初始化为空（所有节点将被插到头后，形成倒序)

  bool hasLast = false;
  Elemtype lastVal = 0;

  while (p1 != nullptr && p2 != nullptr)
  {
    Linklist cur = nullptr;
    if (p1->data < p2->data)
    {
      cur = p1;
      p1 = p1->next;
    }
    else if (p1->data > p2->data)
    {
      cur = p2;
      p2 = p2->next;
    }
    else // 相等，取 p1 作为当前节点，p2 节点删去
    {
      cur = p1;
      p1 = p1->next;
      Linklist tmp = p2;
      p2 = p2->next;
      delete tmp;
    }

    if (!hasLast || cur->data != lastVal)
    {
      // 插入到结果表头部
      cur->next = L3->next;
      L3->next = cur;
      lastVal = cur->data;
      hasLast = true;
    }
    else
    {
      // 重复，丢弃该节点
      delete cur;
    }
  }

  // 处理剩余 p1
  while (p1 != nullptr)
  {
    Linklist cur = p1;
    p1 = p1->next;
    if (!hasLast || cur->data != lastVal)
    {
      cur->next = L3->next;
      L3->next = cur;
      lastVal = cur->data;
      hasLast = true;
    }
    else
    {
      delete cur;
    }
  }

  // 处理剩余 p2
  while (p2 != nullptr)
  {
    Linklist cur = p2;
    p2 = p2->next;
    if (!hasLast || cur->data != lastVal)
    {
      cur->next = L3->next;
      L3->next = cur;
      lastVal = cur->data;
      hasLast = true;
    }
    else
    {
      delete cur;
    }
  }

  // 清空 L2（保留头结点）
  L2->next = nullptr;

  return L3;
}

void PrintList(Linklist L)
{
  Linklist p = L->next; // 跳过头结点
  if (!p)
  {
    std::cout << "empty\n";
    return;
  }
  while (p != nullptr)
  {
    std::cout << p->data;
    if (p->next)
      std::cout << " -> ";
    p = p->next;
  }
  std::cout << std::endl;
}