#pragma once
#include <deque>
#include <iostream>
#include <sstream>
#include "common/Commom.h"

class Snake
{
private:
    std::deque<Point> body;
    Direction dir = Direction::RIGHT;
    bool growNextMove = false;
    bool dead = false; // 是否死亡

public:
    Snake();
    void update(); // 返回是否存活
    void grow();
    Point getHead() const;
    void draw() const;
    void drawToBuffer(std::stringstream &buf) const;
    Direction getDirection() const;
    void setDirection(Direction d);
    const std::deque<Point> &getBody() const;
    void setBody(const std::deque<Point> &newBody);
    bool isDead() const;             // 获取死亡状态
    void reset();                    // 重置蛇（重新开始）
    bool checkSelfCollision() const; // 检测自身碰撞
};
