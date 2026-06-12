#ifndef START_SCREEN_H
#define START_SCREEN_H

#include <SFML/Graphics.hpp>
#include <string>

class StartScreen {
public:
    enum Difficulty { EASY, NORMAL, HARD };

    StartScreen(sf::RenderWindow& window);
    
    void draw();
    void handleMouseClick(sf::Vector2f mousePos);
    bool isStartButtonClicked(sf::Vector2f mousePos) const;
    bool isQuitButtonClicked(sf::Vector2f mousePos) const;
    Difficulty getSelectedDifficulty() const;
    
private:
    sf::RenderWindow& window;
    sf::Font font;
    sf::FloatRect startButtonRect;
    sf::FloatRect quitButtonRect;
    sf::FloatRect easyButtonRect;
    sf::FloatRect normalButtonRect;
    sf::FloatRect hardButtonRect;
    Difficulty selectedDifficulty;
    
    void loadFont();
    void drawButton(const std::string& text, float x, float y, float width, float height, 
                   bool isHovered, sf::FloatRect& outRect, bool selected = false);
    void drawDifficultyButtons();
};

#endif // START_SCREEN_H
