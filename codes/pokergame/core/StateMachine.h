#pragma once
#include "../gameplay/Snake.h"
#include "../gameplay/Food.h"

enum class GameState { MENU, RUNNING, PAUSED, GAME_OVER, EXIT };

class StateMachine {
private:
    GameState state = GameState::MENU;
    Snake snake;
    Food food;
    int speedLevel = 1;
    int intervalMs = 200;

public:
    StateMachine();
    void update();
    void startNewGame();
    void changeState(GameState newState);
    GameState getState() const;
    bool isRunning() const;
    Snake& getSnake();
    const Snake& getSnake() const;
    Food& getFood();
    const Food& getFood() const;
    int getSpeedLevel() const;
    int getIntervalMs() const;
    void setSpeedLevel(int level);
    void setIntervalMs(int interval);
};
