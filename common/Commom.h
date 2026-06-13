#pragma once

struct Point
{
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point &other) const
    {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point &other) const
    {
        return !(*this == other);
    }
};

constexpr int GAME_WIDTH = 20;
constexpr int GAME_HEIGHT = 10;

// 速度等级：tick间隔（毫秒），越小越快
constexpr int SPEED_SLOW = 200;
constexpr int SPEED_MEDIUM = 130;
constexpr int SPEED_FAST = 70;
constexpr int SPEED_MIN = 40; // 最快上限

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    STOP
};

// 判断两个方向是否相反（180°反向）
inline bool isOpposite(Direction a, Direction b)
{
    return (a == Direction::UP && b == Direction::DOWN) ||
           (a == Direction::DOWN && b == Direction::UP) ||
           (a == Direction::LEFT && b == Direction::RIGHT) ||
           (a == Direction::RIGHT && b == Direction::LEFT);
}

inline Direction keyToDirection(char key)
{
    switch (key)
    {
    case 'w':
    case 'W':
        return Direction::UP;
    case 's':
    case 'S':
        return Direction::DOWN;
    case 'a':
    case 'A':
        return Direction::LEFT;
    case 'd':
    case 'D':
        return Direction::RIGHT;
    default:
        return Direction::STOP;
    }
}

// 处理方向键（扩展键码）
inline Direction arrowKeyToDirection(int extendedKey)
{
    switch (extendedKey)
    {
    case 72:
        return Direction::UP; // ↑
    case 80:
        return Direction::DOWN; // ↓
    case 75:
        return Direction::LEFT; // ←
    case 77:
        return Direction::RIGHT; // →
    default:
        return Direction::STOP;
    }
}
