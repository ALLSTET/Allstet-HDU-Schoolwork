#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAXSIZE 10000
#define MAX_DEPTH 1000 // 限制递归深度

typedef struct
{
  int r[MAXSIZE + 1]; // r[0]用作哨兵或临时存储
  int length;
  int recursion_depth; // 跟踪递归深度
} SqList;

// 交换两个元素
void swap(int *a, int *b)
{
  int temp = *a;
  *a = *b;
  *b = temp;
}

// 初始化顺序表
void InitList(SqList *L, int arr[], int n)
{
  if (n > MAXSIZE)
  {
    printf("错误：数组大小超过最大值 %d\n", MAXSIZE);
    exit(1);
  }
  for (int i = 0; i < n; i++)
  {
    L->r[i + 1] = arr[i];
  }
  L->length = n;
  L->recursion_depth = 0;
}

// 打印顺序表
void PrintList(SqList L)
{
  printf("[");
  for (int i = 1; i <= L.length; i++)
  {
    printf("%d", L.r[i]);
    if (i < L.length)
      printf(", ");
  }
  printf("]\n");
}

int Partition(SqList *L, int low, int high)
{
  if (low < 1 || high > L->length || low > high)
  {
    printf("分区错误：low=%d, high=%d, length=%d\n", low, high, L->length);
    return low;
  }

  // 用子表的第一个记录做枢轴记录，暂存到r[0]
  L->r[0] = L->r[low];
  int pivotkey = L->r[low];

  // printf("Partition [%d..%d], pivot = %d\n", low, high, pivotkey);

  while (low < high)
  {
    // 从右向左找第一个小于枢轴的元素
    while (low < high && L->r[high] >= pivotkey)
    {
      high--;
    }
    if (low < high)
    {
      L->r[low] = L->r[high];
    }

    // 从左向右找第一个大于枢轴的元素
    while (low < high && L->r[low] <= pivotkey)
    {
      low++;
    }
    if (low < high)
    {
      L->r[high] = L->r[low];
    }
  }

  // 枢轴记录到位
  L->r[low] = L->r[0];

  return low;
}

void QSort(SqList *L, int low, int high)
{
  if (L->recursion_depth > MAX_DEPTH)
  {
    printf("警告：递归深度超过限制 %d\n", MAX_DEPTH);
    return;
  }

  L->recursion_depth++;

  if (low < high)
  {
    // 将L->r[low..high]一分为二，pivotloc是枢轴位置
    int pivotloc = Partition(L, low, high);

    if (pivotloc > low)
    {
      // 对左子表递归排序
      QSort(L, low, pivotloc - 1);
    }

    if (pivotloc < high)
    {
      // 对右子表递归排序
      QSort(L, pivotloc + 1, high);
    }
  }

  L->recursion_depth--;
}

void QuickSort_Iterative(SqList *L)
{
  if (L->length <= 1)
    return;

  // 使用栈存储子数组边界
  int *stack = (int *)malloc((L->length + 2) * sizeof(int));
  if (!stack)
  {
    printf("内存分配失败\n");
    return;
  }

  int top = -1;

  // 压入初始边界
  stack[++top] = 1;         // low
  stack[++top] = L->length; // high

  while (top >= 0)
  {
    // 弹出边界
    int high = stack[top--];
    int low = stack[top--];

    if (low < high)
    {
      // 分区
      int pivotloc = Partition(L, low, high);

      // 将左子数组边界压入栈
      if (pivotloc - 1 > low)
      {
        stack[++top] = low;
        stack[++top] = pivotloc - 1;
      }

      // 将右子数组边界压入栈
      if (pivotloc + 1 < high)
      {
        stack[++top] = pivotloc + 1;
        stack[++top] = high;
      }
    }
  }

  free(stack);
}

// 示例8.4的实现（简化的演示版本）
void Example8_4()
{
  printf("=============== 示例8.4 ===============\n");
  printf("已知待排序记录的关键字序列为{49,38,65,97,76,13,27,49}\n\n");

  int arr[] = {49, 38, 65, 97, 76, 13, 27, 49};
  int n = sizeof(arr) / sizeof(arr[0]);

  SqList L;
  InitList(&L, arr, n);

  printf("初始关键字: ");
  PrintList(L);
  printf("\n");

  printf("执行快速排序...\n");
  QuickSort_Iterative(&L);

  printf("排序结果: ");
  PrintList(L);
  printf("\n");

  // 验证排序结果
  bool sorted = true;
  for (int i = 1; i < L.length; i++)
  {
    if (L.r[i] > L.r[i + 1])
    {
      sorted = false;
      break;
    }
  }
  printf("排序验证: %s\n", sorted ? "正确" : "错误");
  printf("\n");
}

// 生成随机数组
void GenerateRandomArray(int arr[], int n, int max_value)
{
  srand(time(NULL));
  for (int i = 0; i < n; i++)
  {
    arr[i] = rand() % max_value;
  }
}

int main()
{
  printf("==================================================\n");
  printf("快速排序算法实现\n");
  printf("==================================================\n\n");

  int choice;

  do
  {
    printf("\n=============== 主菜单 ===============\n");
    printf("1. 运行示例(参考8.4)\n");
    printf("0. 退出程序\n");
    printf("====================================\n");
    printf("请选择 (0-1): ");

    if (scanf("%d", &choice) != 1)
    {
      printf("输入错误，请重新输入\n");
      while (getchar() != '\n')
        ; // 清空输入缓冲区
      continue;
    }

    switch (choice)
    {
    case 1:
      Example8_4();
      break;
    case 0:
      printf("感谢使用，再见！\n");
      break;
    default:
      printf("无效的选择，请重新输入\n");
    }

  } while (choice != 0);

  getchar(); // 防止程序立即退出
  return 0;
}