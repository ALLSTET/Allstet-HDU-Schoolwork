#pragma once
#include <iostream>
#include <sstream>
#include <string>

class StateMachine;
class InputSystem;

class RenderSystem {
private:
    std::stringstream buffer;
    bool firstFrame = true;
    static const int GAME_WIDTH = 20;
    static const int GAME_HEIGHT = 10;

public:
    void render(const StateMachine& fsm, const InputSystem& input);
    void flush();
};