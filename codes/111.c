#include <stdio.h>

int main()
{
  char ch, a, b;
  while ((ch = getchar()) != '\n')
  {
    scanf("%c %c", &a, &b);
    printf("%c %c",a, b);
  }
  return 0;
}