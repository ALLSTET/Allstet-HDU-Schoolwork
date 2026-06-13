#include "TimeSystem.h"

TimeSystem::TimeSystem()
    : lastTick(Clock::now()), tickIntervalMs(SPEED_SLOW)
{
}

void TimeSystem::setSpeed(Speed speed)
{
  switch (speed)
  {
  case Speed::SLOW:
    tickIntervalMs = SPEED_SLOW;
    break;
  case Speed::MEDIUM:
    tickIntervalMs = SPEED_MEDIUM;
    break;
  case Speed::FAST:
    tickIntervalMs = SPEED_FAST;
    break;
  }
}

void TimeSystem::setSpeedMs(int ms)
{
  tickIntervalMs = ms;
  if (tickIntervalMs < SPEED_MIN)
    tickIntervalMs = SPEED_MIN;
}

void TimeSystem::increaseSpeed(int stepMs)
{
  tickIntervalMs -= stepMs;
  if (tickIntervalMs < SPEED_MIN)
    tickIntervalMs = SPEED_MIN;
}

bool TimeSystem::shouldUpdate()
{
  auto now = Clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();
  if (elapsed >= tickIntervalMs)
  {
    lastTick = now;
    return true;
  }
  return false;
}

void TimeSystem::markUpdated()
{
  lastTick = Clock::now();
}

int TimeSystem::getFps() const
{
  if (tickIntervalMs <= 0)
    return 0;
  return 1000 / tickIntervalMs;
}

void TimeSystem::reset()
{
  lastTick = Clock::now();
}
