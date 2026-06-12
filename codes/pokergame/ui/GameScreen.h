#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "../core/GameLogic.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>

class GameScreen {
public:
    GameScreen(GameLogic& gameLogic, sf::RenderWindow& window);
    
    void draw();
    void handleMouseClick(sf::Vector2f mousePos);
    void handleBiddingClick(sf::Vector2f mousePos);
    void update();
    
    bool isPlayerTurn() const;
    int getSelectedCardIndex() const;
    void resetSelectedCard();

private:
    GameLogic& gameLogic;
    sf::RenderWindow& window;
    sf::Font font;
    int selectedCardIndex;
    std::unordered_map<std::string, sf::Texture> cardTextures;
    sf::FloatRect callBankerButtonRect;
    sf::FloatRect robBankerButtonRect;
    sf::FloatRect passBankerButtonRect;
    std::string textureLoadStatus;
    bool fontLoaded;
    
    void drawPlayers();
    void drawTableCards();
    void drawBiddingButtons();
    void drawGameInfo();
    void drawInstructions();
    int calculateHandValue(const Player& player) const;
    
    void loadFont();
    void loadCardTextures();
    std::string getCardTextureKey(const Card& card) const;
    void drawCard(const Card& card, float x, float y, bool selected = false);
    void drawPlayerHand(const Player& player, float x, float y, int playerIndex, bool horizontal = true);
    void drawButton(const std::string&, float, float, float, float);
};

#endif // GAME_SCREEN_H
