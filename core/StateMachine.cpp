#include "StateMachine.h"

StateMachine::StateMachine()
    : currentState(GameState::MENU)
{
}

GameState StateMachine::getState() const
{
  return currentState;
}

void StateMachine::transitionTo(GameState newState)
{
  currentState = newState;
}

bool StateMachine::isRunning() const
{
  return currentState == GameState::RUNNING;
}

bool StateMachine::isPaused() const
{
  return currentState == GameState::PAUSED;
}

bool StateMachine::isGameOver() const
{
  return currentState == GameState::GAME_OVER;
}

bool StateMachine::isMenu() const
{
  return currentState == GameState::MENU;
}

bool StateMachine::isExiting() const
{
  return currentState == GameState::EXIT;
}

bool StateMachine::canProcessDirection() const
{
  // 只在 RUNNING 状态下允许方向输入
  return currentState == GameState::RUNNING;
}
