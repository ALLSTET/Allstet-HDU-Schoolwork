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

    // 自身碰撞检测（先移动再检测新头部）
    body.push_front(head);
    if (checkSelfCollision())
    {
        dead = true;
        return;
    }

    if (growNextMove)
    {
        growNextMove = false;
    }
    else
    {
        body.pop_back();
    }
}

void Snake::grow()
{
    growNextMove = true;
}

Point Snake::getHead() const
{
    return body.front();
}

bool Snake::isDead() const
{
    return dead;
}

bool Snake::checkSelfCollision() const
{
    const Point &head = body.front();
    // 从第二个元素开始检查（跳过头部）
    for (size_t i = 1; i < body.size(); ++i)
    {
        if (body[i] == head)
            return true;
    }
    return false;
}

void Snake::draw() const
{
    for (const auto &p : body)
    {
        std::cout << "(" << p.x << "," << p.y << ") ";
    }
    std::cout << std::endl;
}

void Snake::drawToBuffer(std::stringstream &buf) const
{
    for (const auto &p : body)
    {
        buf << "(" << p.x << "," << p.y << ") ";
    }
    buf << std::endl;
}

Direction Snake::getDirection() const
{
    return dir;
}

void Snake::setDirection(Direction d)
{
    // 禁止反向
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
    dead = false;         // ★ 读档后必须清除死亡状态
    growNextMove = false; // ★ 清除"下一回合增长"标记，避免状态残留
}
