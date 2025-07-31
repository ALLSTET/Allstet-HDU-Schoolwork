#include <stdio.h>

int main()
{
  int a;
  scanf("%d", &a);
  if ((a%4==0 && a%100!=0) || (a%400==0))
  {
    printf("输入年份: %d 是闰年\n",a);
  }
  return 0;
}#