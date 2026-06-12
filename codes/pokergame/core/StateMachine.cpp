#include "StateMachine.h"
#include <algorithm>

StateMachine::StateMachine() : state(GameState::MENU) {
}

void StateMachine::startNewGame() {
    snake = Snake();
    food.respawn(snake.getBody());
    speedLevel = 1;
    intervalMs = 200;
    state = GameState::RUNNING;
}

void StateMachine::update() {
    if (state == GameState::RUNNING) {
        snake.update();
        if (food.isEaten(snake.getHead())) {
            snake.grow();
            food.respawn(snake.getBody());
            speedLevel++;
            intervalMs = std::max(50, 200 - (speedLevel - 1) * 15);
        }
    }
}

void StateMachine::changeState(GameState newState) {
    state = newState;
}

GameState StateMachine::getState() const {
    return state;
}

bool StateMachine::isRunning() const {
    return state != GameState::EXIT;
}

Snake& StateMachine::getSnake() {
    return snake;
}

const Snake& StateMachine::getSnake() const {
    return snake;
}

Food& StateMachine::getFood() {
    return food;
}

const Food& StateMachine::getFood() const {
    return food;
}

int StateMachine::getSpeedLevel() const {
    return speedLevel;
}

int StateMachine::getIntervalMs() const {
    return intervalMs;
}

void StateMachine::setSpeedLevel(int level) {
    speedLevel = level;
}

void StateMachine::setIntervalMs(int interval) {
    intervalMs = interval;
}
