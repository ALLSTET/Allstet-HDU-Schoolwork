#include <iostream>
#include <string>
typedef int OperandType;   // 操作数类型
typedef char OperatorType; // 运算符类型
#define MAXSIZE 100

template <typename T>
struct Stack
{
  T *base = nullptr;
  T *top = nullptr;
  int capacity = 0;

  void Init(int reserveSize = MAXSIZE)
  {
    if (base)
      delete[] base;
    capacity = std::max(1, reserveSize);
    base = new T[capacity];
    top = base;
  }

  ~Stack()
  {
    if (base)
    {
      delete[] base;
      base = nullptr;
      top = nullptr;
      capacity = 0;
    }
  }

  void Push(const T &v)
  {
    if (!base)
      Init();
    int used = int(top - base);
    if (used >= capacity)
    {
      int newCap = capacity * 2;
      T *newBuf = new T[newCap];
      std::copy(base, base + capacity, newBuf);
      delete[] base;
      base = newBuf;
      top = base + used;
      capacity = newCap;
    }
    *top++ = v;
  }

  void Pop()
  {
    if (base == top)
      throw std::underflow_error("Stack empty!");
    --top;
  }

  T Top() const
  {
    if (base == top)
      throw std::underflow_error("Stack empty!");
    return *(top - 1);
  }

  bool Empty() const { return base == top; }
  int Size() const { return int(top - base); }
};

Stack<OperandType> OPND;  // 操作数栈
Stack<OperatorType> OPTR; // 运算符栈

bool Check(int ch)
{
  return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '#' || ch == '(' || ch == ')';
}

OperatorType Precede(const OperatorType &op1, const OperatorType &op2)
{
  // 先处理结束符号 '#'
  if (op1 == '#' && op2 == '#')
    return '=';
  if (op2 == '#')
    return '>'; // 遇到输入结束，需把栈中运算符弹出并计算（除非栈顶也是 '#')
  if (op1 == '(' && op2 == ')')
    return '=';
  if (op1 == '(')
    return '<';
  if (op2 == '(')
    return '<';
  if (op2 == ')')
    return '>';
  if ((op1 == '+' || op1 == '-') && (op2 == '+' || op2 == '-'))
    return '>';
  if ((op1 == '+' || op1 == '-') && (op2 == '*' || op2 == '/'))
    return '<';
  if ((op1 == '*' || op1 == '/') && (op2 == '+' || op2 == '-'))
    return '>';
  if ((op1 == '*' || op1 == '/') && (op2 == '*' || op2 == '/'))
    return '>';
  return '<';
}

OperandType Operate(const OperandType &a, const OperatorType &op, const OperandType &b)
{
  switch (op)
  {
  case '+':
    return a + b;
  case '-':
    return a - b;
  case '*':
    return a * b;
  case '/':
    if (b == 0)
      throw std::runtime_error("Division by 0!");
    return a / b;
  default:
    throw std::logic_error("Invalid operator!");
  }
}

int main()
{
  try
  {
    OPND.Init();
    OPTR.Init();
    OPTR.Push('#');

    std::string line;
    if (!std::getline(std::cin, line))
    {
      std::cerr << "No input\n";
      return 1;
    }

    line.push_back('#');

    size_t pos = 0;
    while (pos < line.size())
    {
      char ch = line[pos];

      if (!Check(ch))
      {
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
        {
          ++pos;
          continue;
        }
        if (ch >= '0' && ch <= '9')
        {
          long value = 0;
          while (pos < line.size() && line[pos] >= '0' && line[pos] <= '9')
          {
            value = value * 10 + (line[pos] - '0');
            ++pos;
          }
          OPND.Push((OperandType)value);
          continue;
        }
        else
        {
          throw std::logic_error("Invalid character in expression!");
        }
      }
      else
      {
        char topOp = OPTR.Top();
        char cmp = Precede(topOp, ch);
        switch (cmp)
        {
        case '<':
          OPTR.Push(ch);
          ++pos;
          break;
        case '>':
        {
          char theta = OPTR.Top();
          OPTR.Pop();
          OperandType b = OPND.Top();
          OPND.Pop();
          OperandType a = OPND.Top();
          OPND.Pop();
          OPND.Push(Operate(a, theta, b));
          break;
        }
        case '=':
          OPTR.Pop();
          ++pos;
          break;
        default:
          throw std::logic_error("Invalid precedence!");
        }
      }
    }

    OperandType result = OPND.Top();
    std::cout << "Answer:" << result << std::endl;
    getchar();
    getchar();
    return 0;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
