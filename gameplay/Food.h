#pragma once
#include <deque>
#include <cstdlib>
#include <ctime>
#include "../common/Commom.h"

/**
 * Food - 食物
 *
 * 随机生成逻辑：
 *   在游戏区域内随机选点，确保不与蛇身（occupied 列表）重叠。
 */

class Food
{
public:
  Food();
  Point getPosition() const;
  void setPosition(const Point &p);
  void respawn(const std::deque<Point> &occupied);
  bool isEaten(const Point &p) const;

private:
  Point position;
};
