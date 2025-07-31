#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  int id;
  char name[21];
  int score;
} Student;

typedef struct Node
{
  Student student;
  struct Node *next;
} Node;

void display_menu();
void add_student(Node *head);
void print_students(Node *head);
void count_students(Node *head);
void find_student(Node *head);
void modify_student(Node *head);
void delete_student(Node *head);
void sort_students(Node *head);
void free_memory(Node *head);

int main()
{
  Node *head = (Node *)malloc(sizeof(Node));
  if (head == NULL)
  {
    fprintf(stderr, "memory allocation failed\n");
    return 1;
  }
  head->next = NULL;

  while (1)
  {
    display_menu();
    int choice;
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
      add_student(head);
      break;
    case 2:
      print_students(head);
      break;
    case 3:
      count_students(head);
      break;
    case 4:
      find_student(head);
      break;
    case 5:
      modify_student(head);
      break;
    case 6:
      delete_student(head);
      break;
    case 7:
      sort_students(head);
      break;
    case 8:
      free_memory(head);
      printf("Exiting system...\n");
      return 0;
    default:
      printf("Invalid choice. Try again.\n");
      break;
    }
  }
  return 0;
}

void display_menu()
{
  printf("\n\n---------------------------------\n");
  printf("*        Student Management       *\n");
  printf("---------------------------------\n");
  printf("* 1. Add Student                  *\n");
  printf("* 2. Print Students               *\n");
  printf("* 3. Count Students               *\n");
  printf("* 4. Find Student                 *\n");
  printf("* 5. Modify Student               *\n");
  printf("* 6. Delete Student               *\n");
  printf("* 7. Sort Students by Score       *\n");
  printf("* 8. Exit System                  *\n");
  printf("---------------------------------\n");
  printf("Enter your choice: ");
}

void add_student(Node *head)
{
  Node *new_node = (Node *)malloc(sizeof(Node));
  if (new_node == NULL)
  {
    fprintf(stderr, "Memory allocation failed\n");
    return;
  }

  printf("Enter student ID: ");
  scanf("%d", &new_node->student.id);
  printf("Enter student name: ");
  scanf("%s", new_node->student.name);
  printf("Enter student score: ");
  scanf("%d", &new_node->student.score);

  new_node->next = NULL;

  Node *current = head;
  while (current->next != NULL)
  {
    current = current->next;
  }
  current->next = new_node;

  printf("Student added successfully!\n");
}

void print_students(Node *head)
{
  Node *current = head->next;
  while (current != NULL)
  {
    printf("ID: %d\tName: %s\tScore: %d\n", current->student.id, current->student.name, current->student.score);
    current = current->next;
  }
}

void count_students(Node *head)
{
  Node *current = head->next;
  int count = 0;
  while (current != NULL)
  {
    count++;
    current = current->next;
  }
  printf("Total number of students: %d\n", count);
}

void find_student(Node *head)
{
  int id;
  printf("Enter student ID to find: ");
  scanf("%d", &id);

  Node *current = head->next;
  while (current != NULL)
  {
    if (current->student.id == id)
    {
      printf("ID: %d\tName: %s\tScore: %d\n", current->student.id, current->student.name, current->student.score);
      return;
    }
    current = current->next;
  }
  printf("Student not found.\n");
}

void modify_student(Node *head)
{
  int id;
  printf("Enter student ID to modify: ");
  scanf("%d", &id);

  Node *current = head->next;
  while (current != NULL)
  {
    if (current->student.id == id)
    {
      printf("Enter new student name (20 letters at most): ");
      scanf("%31s", current->student.name);
      printf("Enter new student score: ");
      scanf("%d", &current->student.score);
      printf("Student information updated.\n");
      return;
    }
    current = current->next;
  }
  printf("Student not found.\n");
}

void delete_student(Node *head)
{
  int id;
  printf("Enter student ID to delete: ");
  scanf("%d", &id);

  Node *current = head;
  Node *previous = NULL;

  while (current != NULL && current->student.id != id)
  {
    previous = current;
    current = current->next;
  }

  if (current == NULL)
  {
    printf("Student not found.\n");
    return;
  }

  if (previous == NULL)
  {
    head->next = current->next;
  }
  else
  {
    previous->next = current->next;
  }

  free(current);
  printf("Student deleted.\n");
}

void sort_students(Node *head)
{
  if (head->next == NULL || head->next->next == NULL)
  {
    return;
  }

  for (Node *i = head->next; i->next != NULL; i = i->next)
  {
    for (Node *j = i->next; j != NULL; j = j->next)
    {
      if (i->student.score < j->student.score)
      {
        Student temp = i->student;
        i->student = j->student;
        j->student = temp;
      }
    }
  }
  printf("Students sorted by score.\n");
}

void free_memory(Node *head)
{
  Node *current = head;
  while (current != NULL)
  {
    Node *temp = current;
    current = current->next;
    free(temp);
  }
}
