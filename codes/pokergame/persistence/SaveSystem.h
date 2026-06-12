#pragma once
#include <string>
#include "../gameplay/Snake.h"
#include "../core/StateMachine.h"

class SaveSystem {
public:
    static bool saveGame(const std::string& filename, const StateMachine& fsm);
    static bool loadGame(const std::string& filename, StateMachine& fsm);
    static bool saveExists(const std::string& filename);
};
