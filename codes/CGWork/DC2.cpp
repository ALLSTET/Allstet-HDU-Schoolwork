#include <iostream>

struct SqList
{
  int data;
  SqList *next;
  SqList(int v = 0) : data(v), next(nullptr) {}
};

SqList *reverseList(SqList *head)
{
  SqList *prev = nullptr;
  SqList *cur = head;
  while (cur)
  {
    SqList *next = cur->next;
    cur->next = prev;
    prev = cur;
    cur = next;
  }
  return prev;
}

void printList(SqList *head)
{
  SqList *p = head;
  while (p)
  {
    std::cout << p->data;
    if (p->next)
      std::cout << " -> ";
    p = p->next;
  }
  std::cout << std::endl;
}

void freeList(SqList *head)
{
  while (head)
  {
    SqList *t = head->next;
    delete head;
    head = t;
  }
}

int main()
{
  int a[5];
  std::cout << "Enter 5 integers: ";
  for (int i = 0; i < 5; ++i)
  {
    if (!(std::cin >> a[i]))
      return 0;
  }

  SqList *head = nullptr;
  SqList *tail = nullptr;
  for (int i = 0; i < 5; ++i)
  {
    SqList *node = new SqList(a[i]);
    if (!head)
    {
      head = tail = node;
    }
    else
    {
      tail->next = node;
      tail = node;
    }
  }

  std::cout << "Original list: ";
  printList(head);

  SqList *newHead = reverseList(head);
  std::cout << "Reversed list: ";
  printList(newHead);

  freeList(newHead);

  getchar();
  getchar();
  return 0;
}
