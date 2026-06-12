#include "GameLoop.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

GameLoop::GameLoop() : window(sf::VideoMode(sf::Vector2u(1200, 800)), "Poker Game - 扑克牌游戏"), currentState(START_SCREEN) {
    srand(static_cast<unsigned>(time(0)));
    window.setFramerateLimit(60);
    
    startScreen = std::make_unique<StartScreen>(window);
    gameScreen = std::make_unique<GameScreen>(gameLogic, window);
}

void GameLoop::run() {
    std::cout << "Game Started! Click Start Game button to begin..." << std::endl;
    
    while (window.isOpen()) {
        handleEvents();
        update();
        render();
    }
    
    std::cout << "Game Closed." << std::endl;
}

void GameLoop::handleEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        } else if (event->is<sf::Event::MouseButtonPressed>()) {
            auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>();
            if (mouseEvent && mouseEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                
                if (currentState == START_SCREEN) {
                    startScreen->handleMouseClick(mousePos);
                    if (startScreen->isStartButtonClicked(mousePos)) {
                        std::cout << "Start Game!" << std::endl;
                        startNewGame();
                    } else if (startScreen->isQuitButtonClicked(mousePos)) {
                        window.close();
                    }
                } else if (currentState == GAME_PLAYING) {
                    if (gameLogic.isBiddingActive() && gameLogic.getCurrentBidderIndex() == 0) {
                        gameScreen->handleBiddingClick(mousePos);
                    } else {
                        gameScreen->handleMouseClick(mousePos);
                    }
                }
            }
        } else if (event->is<sf::Event::KeyPressed>()) {
            auto keyEvent = event->getIf<sf::Event::KeyPressed>();
            if (keyEvent) {
                if (keyEvent->code == sf::Keyboard::Key::Space && currentState == GAME_PLAYING) {
                    if (gameLogic.isBiddingActive()) {
                        // 玩家叫庄/抢庄/过牌（AI由update()自动驱动）
                        if (gameLogic.getCurrentBidderIndex() == 0) {
                            if (gameLogic.canPlayerCallBanker(0)) {
                                gameLogic.playerCallBanker(0);
                                std::cout << "Player calls banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
                            } else if (gameLogic.canPlayerRobBanker(0)) {
                                gameLogic.playerRobBanker(0);
                                std::cout << "Player robs banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
                            } else {
                                gameLogic.playerPassBanker(0);
                            }
                        }
                    } else if (gameLogic.isSecondDealDone()) {
                        // 结算完成后，空格开始新一局
                        gameLogic.startNewRound();
                        aiBidClock.restart();
                        timeSinceLastAIBid = 0.f;
                        std::cout << "New round started!" << std::endl;
                    } else {
                        // Space key to play card (legacy)
                        int selectedCard = gameScreen->getSelectedCardIndex();
                        if (selectedCard >= 0) {
                            auto& players = gameLogic.getPlayers();
                            if (!players.empty() && !players[0].isAIPlayer()) {
                                const Card& card = players[0].getCard(selectedCard);
                                gameLogic.addTableCard(card);
                                std::cout << "Player plays: " << card.toString() << std::endl;
                                for (size_t i = 1; i < players.size(); i++) {
                                    if (players[i].getCardCount() > 0) {
                                        int aiIndex = rand() % players[i].getCardCount();
                                        const Card& aiCard = players[i].getCard(aiIndex);
                                        gameLogic.addTableCard(aiCard);
                                        std::cout << players[i].getName() << " plays: " << aiCard.toString() << std::endl;
                                    }
                                }
                                gameScreen->resetSelectedCard();
                                gameLogic.clearTableCards();
                            }
                        }
                    }
                } else if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    currentState = START_SCREEN;
                    std::cout << "Return to menu" << std::endl;
                }
            }
        }
    }
}

void GameLoop::update() {
    float dt = aiBidClock.restart().asSeconds();
    timeSinceLastAIBid += dt;
    
    // 叫庄阶段：AI逐帧自动出价
    if (gameLogic.isBiddingActive() && gameLogic.getCurrentBidderIndex() != 0) {
        if (timeSinceLastAIBid >= AI_BID_INTERVAL) {
            gameLogic.processAIBid();
            timeSinceLastAIBid = 0.f;
        }
    }
    
    // 叫庄结束后，发第5张牌（仅游戏进行中）
    if (currentState == GAME_PLAYING && !gameLogic.isBiddingActive() && !gameLogic.isSecondDealDone()) {
        gameLogic.dealCardsPhase2();
        std::cout << "Bidding over. 5th card dealt." << std::endl;
        int bi = gameLogic.getBankerIndex();
        const auto& pls = gameLogic.getPlayers();
        if (bi >= 0 && bi < static_cast<int>(pls.size())) {
            std::cout << "Banker: " << pls[bi].getName()
                      << "  Odds: x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
        }
        // 自动进入结算
        gameLogic.settleRound();
        timeSinceLastAIBid = 0.f;
    }
    
    gameScreen->update();
    
    if (currentState == GAME_PLAYING && gameLogic.isGameOver()) {
        currentState = GAME_OVER;
        std::cout << "Game Over! Press ESC to return to menu." << std::endl;
    }
}

void GameLoop::render() {
    // If a round winner exists, highlight background in blue
    if (gameLogic.getLastRoundWinner() != -1) {
        window.clear(sf::Color::Blue);
    } else {
        window.clear(sf::Color::Green);
    }
    
    if (currentState == START_SCREEN) {
        startScreen->draw();
    } else if (currentState == GAME_PLAYING || currentState == GAME_OVER) {
        gameScreen->draw();
        
        if (currentState == GAME_OVER) {
            // sf::Text gameOverText(font, "游戏结束！按ESC返回菜单", 30);
            // gameOverText.setPosition(window.getSize().x / 2.f - 200, window.getSize().y / 2.f);
            // gameOverText.setFillColor(sf::Color::White);
            // window.draw(gameOverText);
        }
    }
    
    window.display();
}

void GameLoop::startNewGame() {
    currentState = GAME_PLAYING;
    gameLogic.initializeGame();
    gameLogic.startNewRound();  // 扣底注 + 发4张 + 叫庄
    aiBidClock.restart();
    timeSinceLastAIBid = 0.f;
    
    std::cout << "\n========== GAME START ==========" << std::endl;
    std::cout << "Tips: Click buttons or SPACE to bid | SPACE for next round | ESC to menu" << std::endl;
}

void GameLoop::endGame() {
    currentState = START_SCREEN;
}
