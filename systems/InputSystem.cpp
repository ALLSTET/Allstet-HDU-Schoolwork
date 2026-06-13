#include "InputSystem.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

InputSystem::InputSystem()
    : currentDirection(Direction::RIGHT)
{
}

void InputSystem::setCurrentDirection(Direction d)
{
  currentDirection = d;
}

// ==================== Windows 实现 ====================
#ifdef _WIN32

bool InputSystem::kbhit()
{
  return _kbhit() != 0;
}

int InputSystem::getch()
{
  return _getch();
}

bool InputSystem::isArrowKey(int ch)
{
  return ch == 224 || ch == 0;
}

int InputSystem::readArrowDirection(int firstCh)
{
  (void)firstCh;
  int ch = _getch();
  // 方向键扩展码 → 编码为负值以便区分（UP=1,DOWN=2,LEFT=3,RIGHT=4）
  switch (ch)
  {
  case 72:
    return -1; // UP
  case 80:
    return -2; // DOWN
  case 75:
    return -3; // LEFT
  case 77:
    return -4; // RIGHT
  default:
    return 0;
  }
}

// ★ 核心：最新合法输入覆盖策略
InputSystem::FrameInput InputSystem::poll()
{
  FrameInput result;
  // 默认不改方向
  result.nextDirection = Direction::STOP;

  Direction bestDirection = currentDirection; // 初始化为当前方向

  // 清空输入缓冲区，逐键处理
  while (kbhit())
  {
    int ch = getch();

    // 处理方向键
    if (isArrowKey(ch))
    {
      int dirCode = readArrowDirection(ch);
      Direction inputDir = Direction::STOP;
      switch (dirCode)
      {
      case -1:
        inputDir = Direction::UP;
        break;
      case -2:
        inputDir = Direction::DOWN;
        break;
      case -3:
        inputDir = Direction::LEFT;
        break;
      case -4:
        inputDir = Direction::RIGHT;
        break;
      default:
        continue; // 无效方向键，跳过
      }

      // ★ 过滤180°反向
      if (!isOpposite(inputDir, currentDirection))
      {
        bestDirection = inputDir; // 覆盖！保留最后一个合法方向
      }
      continue;
    }

    // 处理普通按键
    switch (ch)
    {
    case 'w':
    case 'W':
      if (!isOpposite(Direction::UP, currentDirection))
        bestDirection = Direction::UP;
      break;
    case 's':
    case 'S':
      if (!isOpposite(Direction::DOWN, currentDirection))
        bestDirection = Direction::DOWN;
      break;
    case 'a':
    case 'A':
      if (!isOpposite(Direction::LEFT, currentDirection))
        bestDirection = Direction::LEFT;
      break;
    case 'd':
    case 'D':
      if (!isOpposite(Direction::RIGHT, currentDirection))
        bestDirection = Direction::RIGHT;
      break;
    case 'p':
    case 'P':
      result.pausePressed = true;
      break;
    case 'o':
    case 'O':
      result.savePressed = true;
      break;
    case 'q':
    case 'Q':
      result.quitPressed = true;
      break;
    case '\r':
    case '\n':
      result.confirmPressed = true;
      break;
    default:
      // 无效按键，忽略（鲁棒性设计）
      break;
    }
  }

  // 如果有方向变化，返回最终选择的方向
  if (bestDirection != currentDirection)
  {
    result.nextDirection = bestDirection;
  }

  return result;
}

int InputSystem::waitForKey()
{
  while (!kbhit())
  {
    Sleep(30); // 阻塞等待，降低CPU占用
  }
  int ch = getch();
  if (isArrowKey(ch))
  {
    int dirCode = readArrowDirection(ch);
    return dirCode; // 返回负值
  }
  return ch;
}

// ==================== Linux/macOS 实现 ====================
#else

bool InputSystem::kbhit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return true;
  }
  return false;
}

int InputSystem::getch()
{
  return getchar();
}

bool InputSystem::isArrowKey(int ch)
{
  return ch == 27; // ESC 序列
}

int InputSystem::readArrowDirection(int firstCh)
{
  (void)firstCh;
  int ch = getchar();
  if (ch == EOF)
    return 0;
  if (ch == 91) // '['
  {
    ch = getchar();
    if (ch == EOF)
      return 0;
    switch (ch)
    {
    case 'A':
      return -1; // UP
    case 'B':
      return -2; // DOWN
    case 'C':
      return -4; // RIGHT
    case 'D':
      return -3; // LEFT
    default:
      return 0;
    }
  }
  return 0;
}

InputSystem::FrameInput InputSystem::poll()
{
  FrameInput result;
  result.nextDirection = Direction::STOP;
  Direction bestDirection = currentDirection;

  while (kbhit())
  {
    int ch = getch();

    if (isArrowKey(ch))
    {
      int dirCode = readArrowDirection(ch);
      Direction inputDir = Direction::STOP;
      switch (dirCode)
      {
      case -1:
        inputDir = Direction::UP;
        break;
      case -2:
        inputDir = Direction::DOWN;
        break;
      case -3:
        inputDir = Direction::LEFT;
        break;
      case -4:
        inputDir = Direction::RIGHT;
        break;
      default:
        continue;
      }

      if (!isOpposite(inputDir, currentDirection))
      {
        bestDirection = inputDir;
      }
      continue;
    }

    switch (ch)
    {
    case 'w':
    case 'W':
      if (!isOpposite(Direction::UP, currentDirection))
        bestDirection = Direction::UP;
      break;
    case 's':
    case 'S':
      if (!isOpposite(Direction::DOWN, currentDirection))
        bestDirection = Direction::DOWN;
      break;
    case 'a':
    case 'A':
      if (!isOpposite(Direction::LEFT, currentDirection))
        bestDirection = Direction::LEFT;
      break;
    case 'd':
    case 'D':
      if (!isOpposite(Direction::RIGHT, currentDirection))
        bestDirection = Direction::RIGHT;
      break;
    case 'p':
    case 'P':
      result.pausePressed = true;
      break;
    case 'o':
    case 'O':
      result.savePressed = true;
      break;
    case 'q':
    case 'Q':
      result.quitPressed = true;
      break;
    case '\r':
    case '\n':
      result.confirmPressed = true;
      break;
    default:
      break;
    }
  }

  if (bestDirection != currentDirection)
  {
    result.nextDirection = bestDirection;
  }

  return result;
}

int InputSystem::waitForKey()
{
  while (!kbhit())
  {
    usleep(30000); // 30ms
  }
  int ch = getch();
  if (isArrowKey(ch))
  {
    int dirCode = readArrowDirection(ch);
    return dirCode;
  }
  return ch;
}

#endif
