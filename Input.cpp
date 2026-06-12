#include "Input.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef _WIN32

InputAction Input::poll(bool isMenu, bool isGameOver)
{
  if (!_kbhit())
    return InputAction::NONE;

  int ch = _getch();

  // 方向键：返回两个字符 (224 + 方向码)
  if (ch == 224)
  {
    ch = _getch();
    switch (ch)
    {
    case 72:
      return InputAction::MOVE_UP; // ↑
    case 80:
      return InputAction::MOVE_DOWN; // ↓
    case 75:
      return InputAction::MOVE_LEFT; // ←
    case 77:
      return InputAction::MOVE_RIGHT; // →
    default:
      return InputAction::NONE;
    }
  }

  // 菜单模式按键
  if (isMenu)
  {
    switch (ch)
    {
    case '1':
      return InputAction::NEW_GAME;
    case '2':
      return InputAction::LOAD_GAME;
    case '3':
    case 'q':
    case 'Q':
      return InputAction::QUIT;
    default:
      return InputAction::NONE;
    }
  }

  // 游戏结束状态按键
  if (isGameOver)
  {
    switch (ch)
    {
    case 'r':
    case 'R':
      return InputAction::RESTART;
    case 'q':
    case 'Q':
      return InputAction::QUIT;
    default:
      return InputAction::NONE;
    }
  }

  // 正常游戏状态按键
  switch (ch)
  {
  case 'w':
  case 'W':
    return InputAction::MOVE_UP;
  case 's':
  case 'S':
    return InputAction::MOVE_DOWN;
  case 'a':
  case 'A':
    return InputAction::MOVE_LEFT;
  case 'd':
  case 'D':
    return InputAction::MOVE_RIGHT;
  case 'p':
  case 'P':
    return InputAction::PAUSE;
  case 'o':
  case 'O':
    return InputAction::SAVE; // O = Save
  case 'l':
  case 'L':
    return InputAction::LOAD; // L = Load
  case 'q':
  case 'Q':
    return InputAction::QUIT;
  default:
    return InputAction::NONE;
  }
}

InputAction Input::pollSlot()
{
  if (!_kbhit())
    return InputAction::NONE;
  int ch = _getch();

  // ESC
  if (ch == 27)
    return InputAction::CANCEL;

  switch (ch)
  {
  case '0':
    return InputAction::SLOT_0;
  case '1':
    return InputAction::SLOT_1;
  case '2':
    return InputAction::SLOT_2;
  case '3':
    return InputAction::SLOT_3;
  case 'q':
  case 'Q':
    return InputAction::CANCEL;
  default:
    return InputAction::NONE;
  }
}

#else
// Linux/macOS 非阻塞输入实现
InputAction Input::poll(bool isMenu, bool isGameOver)
{
  termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  int ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch == EOF)
    return InputAction::NONE;

  if (ch == 27)
  { // ESC 序列（方向键）
    ch = getchar();
    if (ch == EOF)
      return InputAction::QUIT;
    if (ch == 91)
    { // '['
      ch = getchar();
      if (ch == EOF)
        return InputAction::NONE;
      switch (ch)
      {
      case 'A':
        return InputAction::MOVE_UP;
      case 'B':
        return InputAction::MOVE_DOWN;
      case 'C':
        return InputAction::MOVE_RIGHT;
      case 'D':
        return InputAction::MOVE_LEFT;
      default:
        return InputAction::NONE;
      }
    }
    return InputAction::NONE;
  }

  if (isMenu)
  {
    switch (ch)
    {
    case '1':
      return InputAction::NEW_GAME;
    case '2':
      return InputAction::LOAD_GAME;
    case '3':
    case 'q':
    case 'Q':
      return InputAction::QUIT;
    default:
      return InputAction::NONE;
    }
  }

  if (isGameOver)
  {
    switch (ch)
    {
    case 'r':
    case 'R':
      return InputAction::RESTART;
    case 'q':
    case 'Q':
      return InputAction::QUIT;
    default:
      return InputAction::NONE;
    }
  }

  switch (ch)
  {
  case 'w':
  case 'W':
    return InputAction::MOVE_UP;
  case 's':
  case 'S':
    return InputAction::MOVE_DOWN;
  case 'a':
  case 'A':
    return InputAction::MOVE_LEFT;
  case 'd':
  case 'D':
    return InputAction::MOVE_RIGHT;
  case 'p':
  case 'P':
    return InputAction::PAUSE;
  case 'o':
  case 'O':
    return InputAction::SAVE;
  case 'l':
  case 'L':
    return InputAction::LOAD;
  case 'q':
  case 'Q':
    return InputAction::QUIT;
  default:
    return InputAction::NONE;
  }
}

InputAction Input::pollSlot()
{
  termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  int ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch == EOF)
    return InputAction::NONE;
  if (ch == 27)
    return InputAction::CANCEL;

  switch (ch)
  {
  case '0':
    return InputAction::SLOT_0;
  case '1':
    return InputAction::SLOT_1;
  case '2':
    return InputAction::SLOT_2;
  case '3':
    return InputAction::SLOT_3;
  case 'q':
  case 'Q':
    return InputAction::CANCEL;
  default:
    return InputAction::NONE;
  }
}
#endif
