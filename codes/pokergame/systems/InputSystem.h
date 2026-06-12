#pragma once
#include <conio.h>
#include "../common/Commom.h"

class StateMachine;

class InputSystem {
private:
    int menuChoice = 0;
    
public:
    void processInput(StateMachine& fsm);
    int getMenuChoice() const { return menuChoice; }
    void resetMenuChoice() { menuChoice = 0; }
};