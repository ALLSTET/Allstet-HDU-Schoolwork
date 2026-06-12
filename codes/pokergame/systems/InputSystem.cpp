#include "InputSystem.h"
#include "../core/StateMachine.h"
#include "../persistence/SaveSystem.h"
#include <cctype>

void InputSystem::processInput(StateMachine& fsm) {
    if (!_kbhit()) return;
    int keyCode = _getch();
    char key = tolower(static_cast<char>(keyCode));
    Direction newDir = Direction::STOP;
    bool isF5 = false;
    if (keyCode == 224) {
        int extKey = _getch();
        if (extKey == 63) isF5 = true;
        else newDir = arrowKeyToDirection(extKey);
    } else newDir = keyToDirection(key);
    GameState state = fsm.getState();
    if (state == GameState::MENU) {
        if (key == '1') menuChoice = 1;
        else if (key == '2') menuChoice = 2;
        else if (key == 'q') fsm.changeState(GameState::EXIT);
        else if (key == '\r' || key == '\n') {
            if (menuChoice == 1) fsm.startNewGame();
            else if (menuChoice == 2) { SaveSystem::loadGame("save.dat", fsm); fsm.changeState(GameState::RUNNING); }
            menuChoice = 0;
        }
    }
    else if (state == GameState::RUNNING) {
        if (key == 'p') fsm.changeState(GameState::PAUSED);
        else if (key == 'q') fsm.changeState(GameState::EXIT);
        else if (isF5) SaveSystem::saveGame("save.dat", fsm);
        else if (newDir != Direction::STOP) {
            Direction currentDir = fsm.getSnake().getDirection();
            if (!isOpposite(currentDir, newDir)) fsm.getSnake().setDirection(newDir);
        }
    }
    else if (state == GameState::PAUSED) {
        if (key == 'p') fsm.changeState(GameState::RUNNING);
        else if (key == 'q') fsm.changeState(GameState::EXIT);
    }
    else if (state == GameState::GAME_OVER) {
        if (key == 'q') fsm.changeState(GameState::MENU);
    }
}


