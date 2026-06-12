#include <iostream>
#include <SDL2/SDL.h>
#include <set>
#include <iomanip>
#define WIDTH 640
#define HEIGHT 500
using namespace std;

void Bresenham_Circle(int xc, int yc, int r, SDL_Renderer *renderer);
void CirclePlot(int xc, int yc, int x, int y, SDL_Renderer *renderer);
void MidpointEllipse(int xc, int yc, int a, int b, SDL_Renderer *renderer);
void EllipsePlot(int xc, int yc, int x, int y, SDL_Renderer *renderer);

int main(int argc, char *argv[])
{
  static int xc, x1, yc, y1, r, r1;
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  window = SDL_CreateWindow(
      "圆&椭圆的中点算法实现",      // 标题
      SDL_WINDOWPOS_UNDEFINED, // 初始化x坐标
      SDL_WINDOWPOS_UNDEFINED, // 初始化y坐标
      WIDTH,                   // 初始化像素宽度
      HEIGHT,                  // 初始化像素高度
      SDL_WINDOW_SHOWN         // flag
  );

  if (window == NULL)
  {
    printf("Could not create window: %s\n", SDL_GetError());
    return 1;
  }
  renderer = SDL_CreateSoftwareRenderer(SDL_GetWindowSurface(window));

  bool quit = true;
  while (quit)
  {
    cout << "Pls input xc, yc, a, b:";
    cin >> x1 >> y1 >> r >> r1;
    xc = x1+320;
    yc = y1+250;
    if ((xc > WIDTH * 3 / 4 || xc < WIDTH / 4) || (yc > HEIGHT * 4 / 5 || yc < HEIGHT / 5) || (r <= 0))
    {
      cout << "Invalid input!" << endl;
    }
    else
      quit = false;
  }
  SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
  MidpointEllipse(xc, yc, r*20, r1*20, renderer); // 前两个参数是椭圆中心坐标，a和b分别是长轴和短轴长度的一半
  SDL_RenderPresent(renderer);
  SDL_UpdateWindowSurface(window);

  SDL_Event event;
  while (!quit)
  {
    SDL_WaitEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT: // 退出事件
      SDL_Log("quit");
      quit = true;
      break;
    }
  }
  if (renderer)
  {
    SDL_DestroyRenderer(renderer);
  }
  SDL_DestroyWindow(window); // 关闭并清理window

  SDL_Quit();
  return 0;
}

void Bresenham_Circle(int xc, int yc, int r, SDL_Renderer *renderer)
{
  int x, y, d;
  x = 0;
  y = r;
  d = 3 - 2 * r;
  CirclePlot(xc, yc, x, y, renderer); // CirclePlot()函数是参照圆的八分对称性完成八个点的位置计算的辅助函数
  while (x < y)
  {
    if (d < 0)
    {
      d = d + 4 * x + 6;
    }
    else
    {
      d = d + 4 * (x - y) + 10;
      y--;
    }
    x++;
    CirclePlot(xc, yc, x, y, renderer);
  }
}

void CirclePlot(int xc, int yc, int x, int y, SDL_Renderer *renderer)
{
  static std::set<std::pair<int, int>> printed;
  static int count = 0; // 计数器
  int px[8] = {xc + x, xc - x, xc + x, xc - x, xc + y, xc - y, xc + y, xc - y};
  int py[8] = {yc + y, yc + y, yc - y, yc - y, yc + x, yc + x, yc - x, yc - x};
  for (int i = 0; i < 8; ++i)
  {
    SDL_RenderDrawPoint(renderer, px[i], py[i]);
    // 四舍五入到一位小数
    int rx = static_cast<int>(px[i] * 10 / 100.0 + 0.5);
    int ry = static_cast<int>(py[i] * 10 / 100.0 + 0.5);
    auto p = std::make_pair(rx, ry);
    if (printed.find(p) == printed.end())
    {
      cout << fixed << setprecision(1) << "(" << rx / 10.0 << ", " << ry / 10.0 << ") ";
      printed.insert(p);
      count++;
      if (count % 5 == 0)
        cout << endl;
    }
  }
}

void MidpointEllipse(int xc, int yc, int a, int b, SDL_Renderer *renderer)
{
  int x = 0, y = b;
  double a2 = a * a, b2 = b * b;
  double d1 = b2 - a2 * b + 0.25 * a2;
  //EllipsePlot(xc, yc, x, y, renderer);

  // 区域1
  while (b2 * (x + 1) < a2 * (y - 0.5))
  {
    if (d1 < 0)
    {
      d1 += b2 * (2 * x + 3);
    }
    else
    {
      d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
      y--;
    }
    x++;
    //EllipsePlot(xc, yc, x, y, renderer);
  }

  // 打印区域1最后一个像素点
  double local_x = x / 20.0;
  double local_y = y / 20.0;
  cout << fixed << setprecision(1) 
  << "Area_1 last pixel dot: (" << local_x << ", " << local_y << ")" << endl;
  // 区域2
  double d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
  
  // 打印区域2第一个像素点
  cout << fixed << setprecision(1) 
  << "Area_2 first pixel dot: (" << local_x << ", " << local_y << ")" << endl;
  while (y > 0)
  {
    if (d2 < 0)
    {
      d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
      x++;
    }
    else
    {
      d2 += a2 * (-2 * y + 3);
    }
    y--;
    //EllipsePlot(xc, yc, x, y, renderer);
  }
}

void EllipsePlot(int xc, int yc, int x, int y, SDL_Renderer *renderer)
{
  static std::set<std::pair<int, int>> printed;
  static int count = 0;
  int px[4] = {xc + x, xc - x, xc + x, xc - x};
  int py[4] = {yc + y, yc + y, yc - y, yc - y};
  for (int i = 0; i < 4; ++i)
  {
    SDL_RenderDrawPoint(renderer, px[i], py[i]);
    int rx = static_cast<int>(px[i] * 10 / 100.0 + 0.5);
    int ry = static_cast<int>(py[i] * 10 / 100.0 + 0.5);
    auto p = std::make_pair(rx, ry);
    if (printed.find(p) == printed.end())
    {
      cout << fixed << setprecision(1) << "(" << rx / 10.0 << ", " << ry / 10.0 << ") ";
      printed.insert(p);
      count++;
      if (count % 5 == 0)
        cout << endl;
    }
  }
}
