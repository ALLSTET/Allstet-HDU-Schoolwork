#include <iostream>
#include <string>
#define MAXSIZE 100
typedef struct
{
  char data[MAXSIZE];
  int top;
} Stack;
void InitStack(Stack &S)
{
  S.top = -1;
}
bool StackEmpty(const Stack &S)
{
  return S.top == -1;
}
bool Push(Stack &S, char x)
{
  if (S.top == MAXSIZE - 1)
    return false;
  S.data[++S.top] = x;
  return true;
}
bool Pop(Stack &S, char &x)
{
  if (StackEmpty(S))
    return false;
  x = S.data[S.top--];
  return true;
}
int main()
{
  Stack S;
  InitStack(S);

  std::cout << "Enter a string with '&' as separator (e.g. 1+2&2+1): ";
  std::string line;
  if (!std::getline(std::cin, line))
    return 0;

  // find separator
  std::size_t sep = line.find('&');
  if (sep == std::string::npos)
  {
    std::cout << "No separator '&' found.\n";
    return 0;
  }

  // push left part to stack
  for (std::size_t i = 0; i < sep; ++i)
  {
    if (!Push(S, line[i]))
    {
      std::cerr << "Stack overflow!\n";
      return 1;
    }
  }

  // compare right part with popped values
  bool isPal = true;
  std::size_t j = sep + 1;
  char ch;
  while (j < line.size() && !StackEmpty(S))
  {
    if (!Pop(S, ch))
    {
      isPal = false;
      break;
    }
    if (ch != line[j])
    {
      isPal = false;
      break;
    }
    ++j;
  }
  // if stack not empty or right side has extra chars -> not palindrome
  if (!StackEmpty(S) || j != line.size())
    isPal = false;

  if (isPal)
    std::cout << "Palindrome\n";
  else
    std::cout << "Not palindrome\n";

  getchar();
  getchar();
  return 0;
}