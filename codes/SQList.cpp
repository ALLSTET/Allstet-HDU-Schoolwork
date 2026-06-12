#include "iostream"

typedef int ElemType;
typedef struct {
    ElemType *elem; // 存储空间基址
    int length;     // 当前长度
} SqList;

void MergeList_Sq(SqList LA, SqList LB, SqList &LC) {
    // 指针初始化
    ElemType *pa = LA.elem;
    ElemType *pb = LB.elem;
    
    // 设置新表长度并分配空间
    LC.length = LA.length + LB.length;
    LC.elem = new ElemType[LC.length];
    ElemType *pc = LC.elem;

    
    // 设置指向表尾的指针
    ElemType *pa_last = LA.elem + LA.length - 1;
    ElemType *pb_last = LB.elem + LB.length - 1;
    ElemType *pc_last = LC.elem + LC.length - 1;
    // 合并两个有序表
    while (pa > pa_last || pb > pb_last) { // 两个表都非空
        if (*pa < *pb) 
            *pc_last-- = *pa++;
        else if (*pa > *pb)
            *pc_last-- = *pb++;
        else{
            *pc_last-- = *pa++;
            if (pb <= pb_last)
                pb++;
        }
    }
    
    // 处理LA中剩余的元素
    while (pa <= pa_last) 
        *pc_last-- = *pa++;
    
    // 处理LB中剩余的元素  
    while (pb <= pb_last)
        *pc_last-- = *pb++;
        
    pc = LC.elem;  // 重置pc到数组开头
    std::cout<<"LC:";
    for(int i = 0; i < LC.length; i++) {
        std::cout << *pc++ << " ";
    }
    std::cout << std::endl;
}


int main(){
  ElemType arr1[] = {1,2,3,5,7};
  ElemType arr2[] = {2,4,6,7,8};
  SqList LA,LB,LC;
  LA.elem = arr1;
  LB.elem = arr2;
  LA.length = 5;
  LB.length = 5;
  MergeList_Sq(LA,LB,LC);

  getchar();
  getchar();
  return 0;
}