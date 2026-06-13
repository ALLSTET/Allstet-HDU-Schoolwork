#pragma once
#include <deque>
#include <iostream>
#include <sstream>
#include "../common/Commom.h"

/**
 * Snake - 蛇实体
 *
 * 数据结构：std::deque<Point>
 * - front() = 蛇头
 * - back()  = 蛇尾
 *
 * 移动逻辑：
 *   头部按方向前进一格 → push_front(newHead)
 *   若未吃到食物 → pop_back()（尾部移除）
 *   若吃到食物 → 不pop_back（自然增长）
 *
 * 碰撞检测：
 *   1. 墙体碰撞（边界越界）
 *   2. 自身碰撞（头部与身体任一部分重合）
 */

class Snake
{
public:
  Snake();
  void update();
  void grow();
  Point getHead() const;
  Direction getDirection() const;
  void setDirection(Direction d);
  const std::deque<Point> &getBody() const;
  void setBody(const std::deque<Point> &newBody);
  bool isDead() const;
  void reset();
  bool checkSelfCollision() const;

private:
  std::deque<Point> body;
  Direction dir = Direction::RIGHT;
  bool growNextMove = false;
  bool dead = false;
};
