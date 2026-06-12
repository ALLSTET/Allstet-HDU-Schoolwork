#pragma once
#include <deque>
#include <cstdlib>
#include <ctime>
#include "common/Commom.h"

class Food
{
private:
    Point position;

public:
    Food();
    Point getPosition() const;
    void setPosition(const Point &p); // ”√”⁄¥Êµµª÷∏¥
    void respawn(const std::deque<Point> &occupied);
    bool isEaten(const Point &p) const;
    void draw() const;
};
