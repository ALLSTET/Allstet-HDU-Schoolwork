#include <iostream>
#define cap_min 5
template <typename T>
struct Data {
  T value;
};

struct SqList {
  int length;
  Data<int>* data;
  int capacity;  
};

void initializeList(SqList *list, int cap) {
  if (cap < cap_min * 2) {
    std::cerr <<"Capacity must be at least "<< cap_min * 2 <<"!"<<std::endl;
    return;
  }
  list->capacity = cap;
  list->data = new Data<int>[cap];
  for (list->length = 0; list->length != cap_min; ++list->length) {
  list->data[list->length].value = list->length * 2;
  }
}

void destroyList(SqList *list) {
  delete[] list->data;
  list->data = nullptr;
  list->length = 0;
  list->capacity = 0;
}

void insertElement(SqList *list, const Data<int> &element) {
  int position;
  for (int i = 0; i != list->capacity; ++i) {
    if (element.value <= list->data[i].value) {
      position = i;
      break;
    }
    if (i == list->length - 1) {
      position = list->length;
    }
  }
  for (int i = list->length; i > position; --i) {
    list->data[i] = list->data[i - 1];
  }
  list->data[position] = element;
  list->length++;
}

int main() {
  SqList myList;
  initializeList(&myList, 10);

  for (int i = 0; i < myList.length; ++i) {
    std::cout << "Element " << i << ": " << myList.data[i].value << std::endl;
  }
  int x;
  std::cout << "Enter a value to insert: ";
  std::cin>>x;
  insertElement(&myList, Data<int>{x});

  for (int i = 0; i < myList.length; ++i) {
    std::cout << "Element " << i << ": " << myList.data[i].value << std::endl;
  }


  getchar();
  getchar();
  destroyList(&myList);
  return 0;
}