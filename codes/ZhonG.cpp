#include <iostream>
#include <vector>
#include <cmath>
#include <array>

int main()
{
  const int image_width = 200;
  const int image_height = 100;

  // 创建像素数组，初始化为黑色
  std::vector<std::array<int, 3>> value(image_width, std::array<int,3>{0, 0, 0});
  std::vector<std::vector<std::array<int, 3>>> image(image_height, value);

  // 设定直线起点和终点（可自行修改）
  int x0 = 20, y0 = 10;
  int x1 = 180, y1 = 80;

  // 算法绘制直线（白色）
  int dx = x1 - x0;
  int dy = y1 - y0;
  int steps = std::max(std::abs(dx), std::abs(dy));
  float x_inc = dx / float(steps);
  float y_inc = dy / float(steps);
  float x = x0;
  float y = y0;
  for (int i = 0; i <= steps; ++i)
  {
    int xi = std::round(x);
    int yi = std::round(y);
    // 注意y坐标要在[0, image_height-1]范围内
    if (xi >= 0 && xi < image_width && yi >= 0 && yi < image_height)
    {
      image[image_height - 1 - yi][xi] = {255, 255, 255}; // 白色
    }
    x += x_inc;
    y += y_inc;
  }

  // 输出PPM
  std::cout << "P3\n"
            << image_width << ' ' << image_height << "\n255\n";
  for (int y = 0; y < image_height; ++y)
  {
    for (int x = 0; x < image_width; ++x)
    {
      auto &pixel = image[y][x];
      std::cout << pixel[0] << ' ' << pixel[1] << ' ' << pixel[2] << '\n';
    }
  }
}
