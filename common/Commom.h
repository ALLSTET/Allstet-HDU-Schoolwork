#pragma once

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

constexpr int GAME_WIDTH = 20;
constexpr int GAME_HEIGHT = 10;

enum class Direction { UP, DOWN, LEFT, RIGHT, STOP };

inline bool isOpposite(Direction a, Direction b) {
    return (a == Direction::UP && b == Direction::DOWN) ||
           (a == Direction::DOWN && b == Direction::UP) ||
           (a == Direction::LEFT && b == Direction::RIGHT) ||
           (a == Direction::RIGHT && b == Direction::LEFT);
}

inline Direction keyToDirection(char key) {
    switch (key) {
        case 'w': case 'W': return Direction::UP;
        case 's': case 'S': return Direction::DOWN;
        case 'a': case 'A': return Direction::LEFT;
        case 'd': case 'D': return Direction::RIGHT;
        default: return Direction::STOP;
    }
}

// 处理方向键（扩展键码）
inline Direction arrowKeyToDirection(int extendedKey) {
    switch (extendedKey) {
        case 72: return Direction::UP;      // ↑
        case 80: return Direction::DOWN;    // ↓
        case 75: return Direction::LEFT;    // ←
        case 77: return Direction::RIGHT;   // →
        default: return Direction::STOP;
    }
}
