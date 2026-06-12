#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAXSIZE 100

// 顺序表结构
typedef struct {
    int r[MAXSIZE + 1];  // r[0]可用作暂存单元
    int length;
} SqList;

// 交换两个元素
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// 初始化顺序表
void InitList(SqList *L, int arr[], int n) {
    for (int i = 0; i < n; i++) {
        L->r[i + 1] = arr[i];
    }
    L->length = n;
}

// 打印顺序表
void PrintList(SqList L) {
    printf("[");
    for (int i = 1; i <= L.length; i++) {
        printf("%d", L.r[i]);
        if (i < L.length) printf(", ");
    }
    printf("]");
}

// 打印数组
void PrintArray(int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

// 打印堆的树状结构
void PrintHeapTree(SqList L, int s, int depth) {
    if (s > L.length) return;
    
    // 打印右子树
    if (2 * s + 1 <= L.length) {
        PrintHeapTree(L, 2 * s + 1, depth + 4);
    }
    
    // 打印当前节点
    for (int i = 0; i < depth; i++) printf(" ");
    printf("%d\n", L.r[s]);
    
    // 打印左子树
    if (2 * s <= L.length) {
        PrintHeapTree(L, 2 * s, depth + 4);
    }
}

// 堆调整函数
void HeapAdjust(SqList *L, int s, int m) {
    // 假设r[s+1..m]已经是堆，将r[s..m]调整为以r[s]为根的大根堆
    int rc = L->r[s];
    int j;
    
    printf("  HeapAdjust(L, %d, %d): rc = %d\n", s, m, rc);
    
    for (j = 2 * s; j <= m; j *= 2) {
        if (j < m && L->r[j] < L->r[j + 1]) {
            ++j;  // j为key较大的记录的下标
        }
        
        if (rc >= L->r[j]) {
            break;  // rc应插入在位置s上
        }
        
        L->r[s] = L->r[j];
        s = j;
    }
    
    L->r[s] = rc;  // 插入
}

// 创建初始堆
void CreatHeap(SqList *L) {
    // 把无序序列L.r[1..n]建成大根堆
    int n = L->length;
    printf("\nBuilding initial heap:\n");
    printf("=======================\n");
    
    for (int i = n / 2; i > 0; --i) {
        printf("\nAdjusting subtree with root at position %d (value = %d):\n", i, L->r[i]);
        HeapAdjust(L, i, n);
        printf("  After adjustment: ");
        PrintList(*L);
        printf("\n");
    }
    
    printf("\nInitial heap built: ");
    PrintList(*L);
    printf("\n");
}

// 堆排序
void HeapSort(SqList *L) {
    printf("\nPerforming Heap Sort:\n");
    printf("======================\n");
    
    // 把无序序列L.r[1..L.length]建成大根堆
    CreatHeap(L);
    
    // 堆排序过程
    for (int i = L->length; i > 1; --i) {
        int x = L->r[1];
        L->r[1] = L->r[i];
        L->r[i] = x;
        
        printf("\nStep %d: Swap r[1]=%d with r[%d]=%d\n", 
               L->length - i + 1, x, i, L->r[1]);
        printf("  After swap: ");
        PrintList(*L);
        
        // 将L.r[1..i-1]重新调整为大根堆
        HeapAdjust(L, 1, i - 1);
        
        printf("  After re-adjust: ");
        PrintList(*L);
        printf("\n");
    }
}

void Example8_6() {
    printf("\n===========================================\n");
    printf("Example 8.6: Building Initial Heap\n");
    printf("===========================================\n");
    
    int arr[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    SqList L;
    InitList(&L, arr, n);
    
    printf("Initial array: ");
    PrintList(L);
    printf("\n\n");
    
    printf("Start building heap from last non-leaf node (n/2 = %d):\n", n/2);
    
    for (int i = n / 2; i > 0; --i) {
        printf("\n--- Adjusting position %d (value = %d) ---\n", i, L.r[i]);
        
        int s = i;
        int rc = L.r[s];
        int m = n;
        int j;
        
        printf("  rc = %d\n", rc);
        
        for (j = 2 * s; j <= m; j *= 2) {
            if (j < m && L.r[j] < L.r[j + 1]) {
                ++j;
            }
            printf("  Compare rc(%d) with r[%d](%d): ", rc, j, L.r[j]);
            
            if (rc >= L.r[j]) {
                printf("rc >= r[%d], break\n", j);
                break;
            } else {
                printf("rc < r[%d], move r[%d] to r[%d]\n", j, j, s);
                L.r[s] = L.r[j];
                s = j;
            }
        }
        
        L.r[s] = rc;
        printf("  Insert rc at position %d\n", s);
        printf("  Current array: ");
        PrintList(L);
        printf("\n");
    }
    
    printf("\nFinal heap: ");
    PrintList(L);
    printf("\n");
    
    printf("\nHeap tree structure:\n");
    PrintHeapTree(L, 1, 0);
}

void Example8_7() {
    printf("\n===========================================\n");
    printf("Example 8.7: Complete Heap Sort Process\n");
    printf("===========================================\n");
    
    int arr[] = {49, 38, 65, 97, 76, 13, 27, 49};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    SqList L;
    InitList(&L, arr, n);
    
    printf("Initial array: ");
    PrintList(L);
    printf("\n\n");
    
    // 执行堆排序
    HeapSort(&L);
    
    printf("\nFinal sorted array: ");
    PrintList(L);
    printf("\n");
}

int main() {
    int choice;
    
    do {
        printf("\n===========================================\n");
        printf("Heap Sort Algorithm Implementation\n");
        printf("Based on Textbook Section 8.4.3\n");
        printf("===========================================\n");
        printf("1. Run Example 8.6 (Building Heap)\n");
        printf("2. Run Example 8.7 (Complete Heap Sort)\n");
        printf("0. Exit\n");
        printf("===========================================\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n');
            continue;
        }
        
        switch (choice) {
            case 1:
                Example8_6();
                break;
            case 2:
                Example8_7();
                break;
            case 0:
                printf("\nGoodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
        
        if (choice != 0) {
            printf("\nPress Enter to continue...");
            while (getchar() != '\n');
            getchar();
        }
        
    } while (choice != 0);
    
    getchar();
    return 0;
}