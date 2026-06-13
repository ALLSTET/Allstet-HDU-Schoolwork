#include "Snake.h"

Snake::Snake()
{
  reset();
}

void Snake::reset()
{
  body.clear();
  body.push_back({5, 5}); // 蛇头
  body.push_back({4, 5}); // 身体1
  body.push_back({3, 5}); // 身体2
  dir = Direction::RIGHT;
  growNextMove = false;
  dead = false;
}

void Snake::update()
{
  if (dead)
    return;

  Point head = body.front();
  switch (dir)
  {
  case Direction::UP:
    head.y--;
    break;
  case Direction::DOWN:
    head.y++;
    break;
  case Direction::LEFT:
    head.x--;
    break;
  case Direction::RIGHT:
    head.x++;
    break;
  default:
    break;
  }

  // 墙体碰撞检测 → 死亡
  if (head.x < 0 || head.x >= GAME_WIDTH ||
      head.y < 0 || head.y >= GAME_HEIGHT)
  {
    dead = true;
    return;
  }

  // 自身碰撞检测
  // ★ 先移尾再加头：蛇尾和蛇头同时移动，蛇头不能撞到正要移走的蛇尾
  if (!growNextMove)
  {
    body.pop_back(); // 蛇尾先离开
  }
  else
  {
    growNextMove = false;
  }

  body.push_front(head); // 蛇头进入新位置
  if (checkSelfCollision())
  {
    dead = true;
    return;
  }
}

void Snake::grow()
{
  growNextMove = true;
}

Point Snake::getHead() const
{
  return body.empty() ? Point{} : body.front();
}

bool Snake::isDead() const
{
  return dead;
}

bool Snake::checkSelfCollision() const
{
  if (body.size() < 2)
    return false;
  const Point &head = body.front();
  for (size_t i = 1; i < body.size(); ++i)
  {
    if (body[i] == head)
      return true;
  }
  return false;
}

Direction Snake::getDirection() const
{
  return dir;
}

void Snake::setDirection(Direction d)
{
  // 方向校验由调用方（GameLoop/InputSystem）负责
  // 此处仅简单赋值（已在上层过滤了180°反向）
  if (!isOpposite(d, dir))
  {
    dir = d;
  }
}

const std::deque<Point> &Snake::getBody() const
{
  return body;
}

void Snake::setBody(const std::deque<Point> &newBody)
{
  body = newBody;
  dead = false;
  growNextMove = false;
}
