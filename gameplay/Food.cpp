#include "Food.h"
#include <algorithm>

Food::Food()
{
  static bool seeded = false;
  if (!seeded)
  {
    seeded = true;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
  }
  respawn({});
}

Point Food::getPosition() const
{
  return position;
}

void Food::setPosition(const Point &p)
{
  position = p;
}

void Food::respawn(const std::deque<Point> &occupied)
{
  do
  {
    position.x = std::rand() % GAME_WIDTH;
    position.y = std::rand() % GAME_HEIGHT;
  } while (std::find(occupied.begin(), occupied.end(), position) != occupied.end());
}

bool Food::isEaten(const Point &p) const
{
  return position == p;
}
