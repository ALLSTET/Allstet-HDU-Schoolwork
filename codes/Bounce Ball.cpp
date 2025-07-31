#include <iostream>
#include <ctime>
#include <windows.h>
#include <conio.h>
using namespace std;

typedef struct
{
  int x, y, x_speed, y_speed;
  char shape;
} Ball;

void SpawnPointSet(Ball *a);
void Question(int *flag);
void Printer(const Ball *a);
void InputCheck(Ball *a, int *flag);
void Calculator(Ball *a);

int main()
{
  int flag = 0;
  Ball *hero = new Ball;
  hero->shape = 'o';
  srand(static_cast<unsigned int>(time(0)));
  Question(&flag);
  while (flag == 1)
  {
    SpawnPointSet(hero);
    Printer(hero);
    do
    {
      InputCheck(hero, &flag);
      Calculator(hero);
      Sleep(200);
      Printer(hero);
    } while (flag == 1);
    Question(&flag);
  }

  delete hero;
  return 0;
}

void SpawnPointSet(Ball *a)
{
  a->x = rand() % 11;
  a->y = rand() % 21;
}

void Question(int *flag)
{
  system("cls");
  char temp;
  if (*flag == 0)
  {
    cout << "Ready to start?Enter Y(yes)/N(no):";
  }
  else if (*flag == 1)
  {
    cout << "Do u wanna play again? Y(yes)/N(no):";
  }
  cin >> temp;
  switch (temp)
  {
  case 'Y':
  case 'y':
    *flag = 1;
    break;
  case 'N':
  case 'n':
    *flag = 0;
    break;
  default:
    cout << '\n'
         << "You'd just entered a wrong answer, pls try again:";
    Question(flag);
  }
}

void Printer(const Ball *a)
{
  system("cls");
  cout << "Ball position: (" << a->x << ", " << a->y << ")" << endl;
  for (int j = 0; j < 21; j++)
  {
    for (int i = 0; i < 11; i++)
    {
      if (a->x == i && a->y == j)
      {
        if (a->x == 10)
        {
          cout << 'd';
        }
        else if (a->x == 0)
        {
          cout << 'b';
        }
        else
          cout << a->shape;
      }
      else if (i % 10 == 0)
      {
        cout << '|';
        if (i == 10)
        {
          cout << endl;
        }
        else
          continue;
      }
      else
      {
        cout << "";
      }
    }
  }
  cout << "-----------" << endl;
  cout << '\n'
       << "You can press 'P' to end the game any time." << endl;
}

void InputCheck(Ball *a, int *flag)
{
  if (_kbhit())
  {
    char key = _getch();
    if (key == 224 || key == 0)
    {
      key = _getch();
      switch (key)
      {
      case 72:
        if (a->y > 0)
        {
          a->y_speed = -1;
        }
        else
          a->y_speed = 1;
        break;
      case 80:
        if (a->y < 21)
        {
          a->y_speed = 1;
        }
        else
          a->y_speed = -1;
        break;
      case 75:
        if (a->x > 0)
        {
          a->x_speed = -1;
        }
        else
          a->x_speed = 1;
        break;
      case 77:
        if (a->x < 10)
        {
          a->x_speed = 1;
        }
        else
          a->x_speed = -1;
      }
    }
    else if (key == 'p' || key == 'P')
    {
      *flag = 0;
    }
  }
}

void Calculator(Ball *a)
{
}
