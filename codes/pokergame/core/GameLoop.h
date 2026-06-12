#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "../core/GameLogic.h"
#include "../ui/GameScreen.h"
#include "../ui/StartScreen.h"
#include <SFML/Graphics.hpp>

constexpr float AI_BID_INTERVAL = 1.2f;  // AI出价间隔（秒）

enum GameState {
    START_SCREEN,
    GAME_PLAYING,
    GAME_OVER
};

class GameLoop {
public:
    GameLoop();
    
    void run();
    
private:
    sf::RenderWindow window;
    GameLogic gameLogic;
    GameState currentState;
    
    std::unique_ptr<StartScreen> startScreen;
    std::unique_ptr<GameScreen> gameScreen;
    
    sf::Clock aiBidClock;
    float timeSinceLastAIBid = 0.f;
    
    void handleEvents();
    void update();
    void render();
    
    void startNewGame();
    void endGame();
};

#endif // GAME_LOOP_H
